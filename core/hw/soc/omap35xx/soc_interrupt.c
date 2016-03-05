
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

extern uint32_t *_interrupt_vector_table;
cpu_callback irq_function_table[128] __attribute__ ((aligned(32)));

/*Specifies allowed interrupts on bit level*/
uint32_t interrupt_ctrl[3];

/*Pending interrupts*/
uint32_t pending_interrupt[3];

static cpu_callback interrupt_handler = 0;
static volatile intc_registers *intc = 0;

void mask_interrupt(uint32_t irq, uint32_t pending)
{
	uint32_t hwirq = irq;
	int offset = irq >> 5;

	irq &= 31;		/*Position of interrupt */
	if (interrupt_ctrl[offset]) {	/*INT CTRL BITMASK IRQ guests are allowed to use */
		if (hwirq != 0x5f && hwirq != 0x25) {	/*Do not mask timer interrupt */
			intc->sections[offset].intc_mir_set = (1 << irq);
		}
		if (pending)
			pending_interrupt[offset] |= (1 << irq);
	} else
		hyper_panic("Mask Guest not allowed to use this IRQ", irq);
}

void unmask_interrupt(uint32_t irq, uint32_t pending)
{
	int offset = irq >> 5;

	irq &= 31;		/*Position of interrupt */

	if (interrupt_ctrl[offset]) {	/*INT CTRL BITMASK IRQ guests are allowed to use */
		intc->sections[offset].intc_mir_clear = (1 << irq);
	} else
		hyper_panic("Unmask Guest not allowed to use this IRQ", irq);
}

void ack_interrupt()
{
/*
  ldr r0, =INTC_VIRT_BASE
  ldr r1, =INTC_CONTROL_NEWIRQAGR
  str r1, [r0, #4 * INTC_CONTROL] */

}

/*Returns the first true bit*/
uint32_t check_pending_interrupts()
{
	int pending, i = 0;
	for (i = 2; i >= 0; i--) {	/*Prioritize timer interrupt by decrementing loop (IRQ 95) */
		if (pending_interrupt[i]) {
			pending = __builtin_clz(pending_interrupt[i]);	/*Built in GCC function to count leading zeros */

			/*Clear the pending IRQ */
			pending_interrupt[i] &= ~(1 << (31 - pending));

			pending = (31 - pending) + (31 * i) + (i * 1);	/*Get the actual IRQ number */
			return pending;
		}
	}
	return (uint32_t) - 1;	/*No pending interrupts */
}

static return_value interrupt_handler_stub(uint32_t irq, uint32_t r1,
					   uint32_t r2)
{
	if (interrupt_handler)
		interrupt_handler(irq, r1, r2);

	return RV_OK;
}

void interrupt_set(uint32_t irq, cpu_callback irq_handler)
{

	interrupt_handler = irq_handler;
	cpu_irq_set_handler(irq, interrupt_handler_stub);

}

/* static */ return_value default_handler(uint32_t r0, uint32_t r1,
					  uint32_t r2)
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
	if (n < 0 || n >= INTC_SOURCE_COUNT)
		return;

	section = (n >> 5);
	pos = n & 0x1f;

	/* set or clear interrupt mask for this irq */
	if (enable) {
		intc->sections[section].intc_mir &= ~(1 << pos);
		intc->sections[section].intc_mir_clear = 1 << pos;
		interrupt_ctrl[section] |= (1 << pos);
	} else {
		intc->sections[section].intc_mir |= (1 << pos);
		intc->sections[section].intc_mir_set = 1 << pos;
		interrupt_ctrl[section] &= ~(1 << pos);	/*INT CTRL BITMASK IRQ guests are allowed to use */
	}
}

void cpu_irq_set_handler(int n, cpu_callback handler)
{

	/* invalid irq number ?? */
	if (n < 0 || n >= INTC_SOURCE_COUNT)
		return;

	if (!handler)
		handler = interrupt_handler_stub;

	//set handler
	irq_function_table[n] = handler;
}

cpu_callback irq_handler();

void soc_interrupt_init()
{
	printf("soc_interrupt_init omap35 \n");
	int i;
	interrupt_handler = (cpu_callback) irq_handler;
	intc = (intc_registers *) IO_VA_ADDRESS(INTC_BASE);

	/* reset the controller */
	intc->intc_sysconfig = INTC_SYSCONFIG_RESET;
	while (!(intc->intc_sysstatus & INTC_SYSSTATUS_RESET_DONE)) ;

	/* turn of all interrupts for now */
	for (i = 0; i < INTC_SOURCE_COUNT; i++) {
		cpu_irq_set_enable(i, FALSE);
		cpu_irq_set_handler(i, default_handler);
	}
	cpu_irq_set_enable(0x4a, TRUE);

	intc->intc_control = INTC_CONTROL_NEWIRQAGR;
}
