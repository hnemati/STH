#include <hw.h>
#include "hyper.h"
#include "guest_blob.h"
#include "mmu.h"
#include "hw_core_mem.h"
#include "dmmu.h"
  
/*
 * Function prototypes
 */ 
void change_guest_mode();
void start();
void board_init();

/*Handlers */ 
void prefetch_abort_handler();
void data_abort_handler();
void swi_handler();
void irq_handler();
void undef_handler();
 
/*Init guest*/ 
void linux_init();

/****************************/ 
/*
 * Globals
 */ 
 extern int __hyper_pt_start__;
extern uint32_t l2_index_p;
 
/*Pointers to start of  first and second level Page tables
 *Defined in linker script  */ 
    uint32_t * flpt_va = (uint32_t *) (&__hyper_pt_start__);
uint32_t * slpt_va = (uint32_t *) ((uint32_t) & __hyper_pt_start__ + 0x4000);	//16k Page offset
extern memory_layout_entry *memory_padr_layout;
  
//Static VM - May change to dynamic in future
    virtual_machine vm_0;
virtual_machine * curr_vm;
 extern void start_();
extern uint32_t _interrupt_vector_table;
 
#ifdef LINUX			//TODO remove ifdefs for something nicer
extern hc_config linux_config;

#endif	/*  */
#ifdef MINIMAL
extern hc_config minimal_config;

#endif	/*  */
    
/*****************************/ 
    
/* DEBUG */ 
void dump_mmu(addr_t adr) 
{
	uint32_t * t = (uint32_t *) adr;
	int i;
	 printf("  (L1 is at %x)\n", adr);
	for (i = 0; i < 4096; i++) {
		uint32_t x = t[i];
		switch (x & 3) {
		case 2:
			printf("SEC %x -> %x : %x DOM=%d C=%d B=%d AP=%d\n",
				i << 20, x, (x & 0xFFF00000), (x >> 5) & 15,
				(x >> 3) & 1, (x >> 2) & 1, (x >> 10) & 3);
			break;
		case 1:
			printf("COR %x -> %x : %x DOM=%d C=%d B=%d\n",
				i << 20, x, (x & 0xFFFFF000), (x >> 5) & 15,
				(x >> 3) & 1, (x >> 2) & 1);
			break;
		case 3:
			printf("FIN %x -> %x\n", i << 20, x);
			break;
		}
	}
	printf("\n");
}

 
/*****************************/ 
void memory_commit() 
{
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE, TRUE, TRUE);	//instr, data, writeback
}  void memory_init() 
{
	
	    /*Setup heap pointer */ 
	    core_mem_init();
	 uint32_t j, va_offset;
	 cpu_type type;
	cpu_model model;
	 cpu_get_type(&type, &model);
	 
	    /* Start with simple access control
	     * Only seperation between hypervisor and user address space
	     *
	     * Here hypervisor already runs in virtual address since boot.S, now just setup guests
	     */ 
	    memory_layout_entry * list =
	    (memory_layout_entry *) (&memory_padr_layout);
	 for (;;) {
		if (!list)
			break;
		 switch (list->type) {
		case MLT_IO_RW_REG:
		case MLT_IO_RO_REG:
		case MLT_IO_HYP_REG:
			
			    /*All IO get coarse pages */ 
			    pt_create_coarse(flpt_va,
					     IO_VA_ADDRESS(PAGE_TO_ADDR
							   (list->
							    page_start)),
					     PAGE_TO_ADDR(list->page_start),
					     (list->page_count -
					      list->page_start) << PAGE_BITS,
					     list->type);
			break;
		 case MLT_USER_RAM:
			
			    /* do this later */ 
			    break;
		 case MLT_HYPER_RAM:
		case MLT_TRUSTED_RAM:
			
			    /* own memory */ 
			    j = (list->page_start) >> 8;	/*Get L1 Page index */
			for (; j < ((list->page_count) >> 8); j++) {
				pt_create_section(flpt_va,
						   (j << 20) - HAL_OFFSET,
						   j << 20, list->type);
			}
			break;
		 case MLT_NONE:
			break;
		 }
		if (list->flags & MLF_LAST)
			break;
		list++;
	}
	 
	    /*map 0xffff0000 to Vector table, interrupt have been relocated to this address */ 
	    pt_map(0xFFFF0000, (uint32_t) GET_PHYS(&_interrupt_vector_table),
		   0x1000, MLT_USER_ROM);
	memory_commit();
	mem_cache_set_enable(TRUE);
	mem_mmu_set_domain(0x55555555);	//Start with access to all domains
}

 void setup_handlers() 
{
	
	    /*Direct the exception to the hypervisor handlers */ 
	    cpu_set_abort_handler((cpu_callback) prefetch_abort_handler,
				  (cpu_callback) data_abort_handler);
	cpu_set_swi_handler((cpu_callback) swi_handler);
	cpu_set_undef_handler((cpu_callback) undef_handler);
	 
	    /* Start the timer and direct interrupts to hypervisor irq handler */ 
	    timer_tick_start((cpu_callback) irq_handler);
}  void guests_init() 
{
	uint32_t i, guest = 0;
	vm_0.id = 0;
	vm_0.next = &vm_0;	//Only one VM
	
	    /*Start with VM_0 as the current VM */ 
	    curr_vm = &vm_0;
	  printf("HV pagetable before guests initialization:\n");	// DEBUG
	dump_mmu(flpt_va);	// DEBUG
	 
	    /* show guest information */ 
	    printf("We have %d guests in physical memory area %x %x\n",
		   guests_db.count, guests_db.pstart, guests_db.pend);
	 for (i = 0; i < guests_db.count; i++) {
		printf("Guest_%d: PA=%x+%x VA=%x FWSIZE=%x\n", i,
			guests_db.guests[i].pstart,
			guests_db.guests[i].psize,
			guests_db.guests[i].vstart,
			guests_db.guests[i].fwsize);
	}
	  
#ifdef LINUX
	    vm_0.config = &linux_config;
	vm_0.config.firmware = get_guest(guest++);
	linux_init();
	
#else	/*  */
	    vm_0.config = &minimal_config;
	vm_0.config->firmware = get_guest(guest++);
	 
	    /* KTH CHANGES */ 
	    /* - The hypervisor must be always able to read/write the guest PTs */ 
	    /*   we constraint that for the minimal guests, the page tables */ 
	    /*   are between physical addresses 0x01000000 and 0x012FFFFF (that are the three megabytes of the guest) */ 
	    /*   of memory reserved to the guest */ 
	    /*   these address are mapped by the virtual address  0x00000000 and 0x002FFFFF */ 
	    /*   TODO: this must be accessible only to the hypervisor */ 
	    // this must be a loop
	    uint32_t va_offset;
	for (va_offset = 0;
	      va_offset <=
	      vm_0.config->reserved_va_for_pt_access_end -
	      vm_0.config->reserved_va_for_pt_access_start;
	      va_offset += SECTION_SIZE) {
		uint32_t offset, pmd;
		uint32_t va =
		    vm_0.config->reserved_va_for_pt_access_start + va_offset;
		uint32_t pa =
		    vm_0.config->pa_for_pt_access_start + va_offset;
		pt_create_section(flpt_va, va, pa, MLT_HYPER_RAM);
		 
		    /* Invalidate the new created entries */ 
		    offset = ((va >> MMU_L1_SECTION_SHIFT) * 4);
		pmd = (uint32_t *) ((uint32_t) flpt_va + offset);
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, pmd);
		printf("%x -> %x\n", va, pa);	// DEBUG
	}
	memory_commit();
	 printf("HV pagetable after guests initialization:\n");	// DEBUG    
	dump_mmu(flpt_va);	// DEBUG    
	
	    // now the master page table is ready
	    // it contains
	    // - the virtual mapping to the hypervisor code and data
	    // - a fixed virtual mapping to the guest PT
	    // - some reserved mapping that for now we ignore, e.g. IO‌REGS
	    // - a 1-1 mapping to the guest memory (as defined in the board_mem.c) writable and readable by the user
	    // - THIS‌ SETUP ‌MUST ‌BE ‌FIXED, SINCE ‌THE ‌GUEST ‌IS ‌NOT ‌ALLOWED ‌TO ‌WRITE ‌IN TO ‌ITS ‌WHOLE‌ MEMORY
	    
	    /* - Create a copy of the master page table for the guest in the physical address: pa_initial_l1 */ 
	    uint32_t * guest_pt_va;
	guest_pt_va =
	    mmu_guest_pa_to_va(vm_0.config->pa_initial_l1, &(vm_0.config));
	printf("COPY %x %x\n", guest_pt_va, flpt_va);
	memcpy(guest_pt_va, flpt_va, 1024 * 16);
	 
#if 0
	    /* reomved by Arash: what is this code?? */ 
	    // Hamed Changes , Creating a valid L1 according to the verified L1_create API
	    if ((value & 0 b1) == 1)
		bft[PA_TO_PH_BLOCK(value)].type = PAGE_INFO_TYPE_L2PT;
	if (((value & 0xFFFF0000) == 0x81200000))
		*(guest_pt_va + index) = (value & 0xFFFFFBFF);
	
	    // END Hamed Changes
#endif	/*  */
	     printf("vm_0 pagetable:\n");	// DEBUG    
	dump_mmu(guest_pt_va);	// DEBUG
	
	    /* activate the guest page table */ 
	    memory_commit();
	COP_WRITE(COP_SYSTEM, COP_SYSTEM_TRANSLATION_TABLE0, vm_0.config->pa_initial_l1);	// Set TTB0
	isb();
	memory_commit();
	  
	    // Initialize the datastructures with the tyoe for the initial L1
	    // This shoud be done by MMU_CREATE_L1
	    dmmu_entry_t * bft = (dmmu_entry_t *) DMMU_BFT_BASE_VA;
	  
#if 0    
	    // create the attribute that allow the guest to read/write/execute
	    uint32_t attrs;
	attrs = 0x12;		// 0b1--10
	attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	attrs =
	    (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	dmmu_map_L1_section(0xc0000000, HAL_PHYS_START + 0x01000000, attrs);
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE, TRUE, TRUE);	//instr, data, writeback
	mem_cache_set_enable(TRUE);
	
#endif	/*  */
	    
	    /* END KTH CHANGES */ 
	    
#endif	/*  */
#ifdef TRUSTED
	    get_guest(guest++);
	curr_vm->mode_states[HC_GM_TRUSTED].ctx.sp =
	    curr_vm->config->rpc_handlers->sp;
	curr_vm->mode_states[HC_GM_TRUSTED].ctx.psr =
	    ARM_INTERRUPT_MASK | ARM_MODE_USER;
	
#endif	/*  */
	    guest = 0;
	 
	do {
		
		    /*Init default values */ 
		    for (i = 0; i < HC_NGUESTMODES; i++) {
			curr_vm->mode_states[i].mode_config =
			    (curr_vm->config->guest_modes[i]);
			curr_vm->mode_states[i].rpc_for = MODE_NONE;
			curr_vm->mode_states[i].rpc_to = MODE_NONE;
		}
		curr_vm->current_guest_mode = MODE_NONE;
		curr_vm->interrupted_mode = MODE_NONE;
		curr_vm->current_mode_state = 0;
		curr_vm->mode_states[HC_GM_INTERRUPT].ctx.psr =
		    ARM_MODE_USER;
		curr_vm = curr_vm->next;
		 
		    // let guest know where it is located
		    curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[3] =
		    curr_vm->config->firmware->pstart;
		curr_vm->mode_states[HC_GM_KERNEL].ctx.reg[4] =
		    curr_vm->config->firmware->vstart;
	} while (curr_vm != &vm_0);
	 memory_commit();
	cpu_context_initial_set(&curr_vm->mode_states[HC_GM_KERNEL].ctx);
}

   void start_guest() 
{
	 
	    /*Change guest mode to KERNEL before going into guest */ 
	    change_guest_mode(HC_GM_KERNEL);
	 
	    /*Starting Guest */ 
	    start();
 }  void start_() 
{
	cpu_init();
	 
	    /*Setting up pagetable rules */ 
	    memory_init();
	 
	    /* Initialize hardware */ 
	    soc_init();
	board_init();
	 
	    /* Setting up exception handlers and starting timer */ 
	    setup_handlers();
	 
	    /* dmmu init */ 
	    dmmu_init();
	 
	    /* Initialize hypervisor guest modes and data structures
	     * according to config file in guest*/ 
	    guests_init();
	
	    /*Test crypto */ 
	    printf("Hypervisor initialized\n Entering Guest\n");
	start_guest();
} 
