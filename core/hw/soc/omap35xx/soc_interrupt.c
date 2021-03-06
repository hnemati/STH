
#include <hw.h>
#include <soc_defs.h>



struct intc_section {
    uint32_t intc_itr;
    uint32_t intc_mir;
    uint32_t intc_mir_clear;
    uint32_t intc_mir_set;
    uint32_t intc_isr_set;
    uint32_t intc_isr_clear;
    uint32_t intc_pending_irq;
    uint32_t intc_pending_fiq;    
};

typedef struct {
    uint32_t intc_revision;
    uint32_t unused0[3];
    uint32_t intc_sysconfig;
    uint32_t intc_sysstatus;
    uint32_t unused1[10];
    uint32_t intc_sir_irq;
    uint32_t intc_sir_fiq;
    uint32_t intc_control;
    uint32_t intc_protection;
    uint32_t intc_idle;
    uint32_t unused2[3];
    uint32_t intc_irq_priority;
    uint32_t intc_fiq_priority;
    uint32_t intc_threshold;
    uint32_t unused3[5];
    struct intc_section sections[4];
    uint32_t intc_ilr[128];
} volatile intc_registers;


/* ----------------------------------------------------- */

extern uint32_t * _interrupt_vector_table;
cpu_callback irq_function_table[128] __attribute__ ((aligned (32)));


static cpu_callback interrupt_handler = 0;
static intc_registers *intc = 0;

void mask_interrupt(uint32_t irq){
	int offset = irq >> 5;

	irq &= 31; /*Position of interrupt*/

	intc->sections[offset].intc_mir_set = (1 << irq);
}

void unmask_interrupt(uint32_t irq){
	int offset = irq >> 5;

	irq &= 31; /*Position of interrupt*/

	intc->sections[offset].intc_mir_clear = (1 << irq);
}

static return_value interrupt_handler_stub(uint32_t irq, uint32_t r1, uint32_t r2 )
{
    if(interrupt_handler)
        interrupt_handler(irq, r1, r2);

    return RV_OK;
}

void interrupt_set(uint32_t irq, cpu_callback irq_handler){

	interrupt_handler = irq_handler;
	cpu_irq_set_handler(irq, interrupt_handler_stub);

}

/* static */ return_value default_handler(uint32_t r0, uint32_t r1, uint32_t r2)
{
    printf("DIH %x:%x:%x\n", r0, r1, r2);
    return RV_OK;
}

/* 
 */
int cpu_irq_get_count()
{
    return INTC_SOURCE_COUNT;
}
void cpu_irq_set_enable(int n, BOOL enable)
{
    uint32_t section, pos;
        
    /* invalid irq number ?? */
    if(n < 0 || n >= INTC_SOURCE_COUNT) return;
    
   section = (n >> 5);
    pos = n & 0x1f;    
    
    /* set or clear interrupt mask for this irq */
    if(enable) {
        intc->sections[section].intc_mir &= ~(1 << pos);
        intc->sections[section].intc_mir_clear = 1 << pos;
    }  else {
        intc->sections[section].intc_mir |= (1 << pos);        
        intc->sections[section].intc_mir_set = 1 << pos;
    }        
}


void cpu_irq_set_handler(int n, cpu_callback handler)
{
        
    /* invalid irq number ?? */
    if(n < 0 || n >= INTC_SOURCE_COUNT) return;
    
    if(!handler) handler = interrupt_handler_stub;
    
    //set handler
    irq_function_table[n] = handler;    
}

cpu_callback irq_handler();

void soc_interrupt_init()
{    
    int i;
    interrupt_handler = (cpu_callback)irq_handler;
    intc = (intc_registers *)IO_VA_ADDRESS(INTC_BASE);

    /* reset the controller */
    intc->intc_sysconfig = INTC_SYSCONFIG_RESET;
    while(!(intc->intc_sysstatus & INTC_SYSSTATUS_RESET_DONE))
        ;
          
    /* turn of all interrupts for now */
    for(i = 0; i < INTC_SOURCE_COUNT; i++) {
        cpu_irq_set_enable(i, FALSE);
        cpu_irq_set_handler(i, default_handler);
    }
    cpu_irq_set_enable(0x4a, TRUE);
    
    intc->intc_control = INTC_CONTROL_NEWIRQAGR;    
}
