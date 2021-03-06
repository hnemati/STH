#include "hw.h"
#include "hyper.h"
#include "hyp_cache.h"

extern virtual_machine *curr_vm;


extern uint32_t *flpt_va;
extern uint32_t *slpt_va;


/*Create a MB Section page
 *Guest can only map to its own domain and to its own physical addresses
 */
void hypercall_create_section(addr_t va, addr_t pa, uint32_t page_attr)
{
	uint32_t PHYS_OFFSET = curr_vm->guest_info.phys_offset;
	uint32_t guest_size = curr_vm->guest_info.guest_size;

	/*Set domain in Section page for user */
	page_attr &= ~MMU_L1_DOMAIN_MASK;
	page_attr |= (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);


	/*Check virtual address*/
	if( va >= HAL_VIRT_START && va < (uint32_t)(HAL_VIRT_START+MAX_TRUSTED_SIZE))
		hyper_panic("Guest trying to map sensitive section virtual address", 1);

	/*Check physical address*/
	if(!(pa >= (PHYS_OFFSET) && pa < (PHYS_OFFSET + guest_size ))){
		hyper_panic("Guest does not own section physical address", 1);
	}

	uint32_t offset = ((va >> MMU_L1_SECTION_SHIFT)*4);
	uint32_t *pmd = (uint32_t *)((uint32_t)flpt_va + offset);

	/*We have created own mapping of first MB in hypervisor with lvl 2
	 *Do nothing if this is the page to be mapped, this is to guarantee read only access to
	 *the master page table(even though its not used )*/
	if(va == 0xc0000000){
		return;
	}

	*pmd = (pa | page_attr);
	COP_WRITE(COP_SYSTEM,COP_DCACHE_INVALIDATE_MVA, pmd);

}

#define LINUX_VA(x) ((((x - curr_vm->guest_info.phys_offset) << 4) >> 24) + 0xc00)

/*Switch page table
 * TODO Add list of allowed page tables*/
void hypercall_switch_mm(addr_t table_base, uint32_t context_id)
{
	uint32_t *l2_pt, lvl2_idx;
	uint32_t pgd_va;
	uint32_t pgd_size = 0x4000;
	uint32_t PAGE_OFFSET = curr_vm->guest_info.page_offset;

	/*First translate the physical address to linux virtual*/
	pgd_va = LINUX_VA(table_base);

	/*Check if va is inside allowed regions*/
	uint32_t va = pgd_va << 20;

	/*Check page address*/
	if( va < PAGE_OFFSET || va > (uint32_t)(HAL_VIRT_START - pgd_size) )
		hyper_panic("Table base va does not reside in kernel address space\n", 1);

	l2_pt = (uint32_t *)GET_VIRT((MMU_L1_PT_ADDR(flpt_va[pgd_va])));
	lvl2_idx = ((table_base << 12) >> 24); /*This is the index+4 to make read only*/

	/*Look if the lvl2 entry is set to read only*/
	if(l2_pt[lvl2_idx] & (1 << 4))
		hyper_panic("Guest tried to set a table base address that is not read only", 1);

	/*Switch the TTB and set context ID*/
	COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, 0); //Set reserved context ID
	isb();
	COP_WRITE(COP_SYSTEM,COP_SYSTEM_TRANSLATION_TABLE0, table_base); // Set TTB0
	isb();
	COP_WRITE(COP_SYSTEM,COP_CONTEXT_ID_REGISTER, context_id); //Set context ID
	isb();

}

/* Free Page table, Make it RW again
 * TODO Security critical, need to add list of used page tables
 * and have a counter for each reference so that we dont free a page table that is still
 * used by another task (Linux kernel knows not to do this but can be used in an attack)*/
void hypercall_free_pgd(addr_t *pgd)
{
//	printf("\n\tLinux kernel Free PGD: %x\n", pgd);
	uint32_t pgd_size = 0x4000;
	uint32_t PAGE_OFFSET = curr_vm->guest_info.page_offset;

	/*Check page address*/
	if( (uint32_t)pgd < PAGE_OFFSET || (uint32_t)pgd > (uint32_t)(HAL_VIRT_START - pgd_size) )
		hyper_panic("Page address not reside in kernel address space\n", 1);


	/*Make the page tables RW*/
	uint32_t lvl2_idx,  i, clean_va;
	uint32_t *l2_pt;


	/*Get level 2 page table address*/

	l2_pt = (uint32_t *)GET_VIRT(((MMU_L1_PT_ADDR(flpt_va[(uint32_t)pgd >> MMU_L1_SECTION_SHIFT]))));


	lvl2_idx = (((uint32_t)pgd << 12) >> 24); /*This is the index+4 to make read only*/


	/*First get the physical address of the lvl 2 page by
	 * looking at the index of the pgd location. Then set
	 * 4 lvl 2 pages to read only*/

    for(i=lvl2_idx; i < lvl2_idx + 4; i++){
		 l2_pt[i] |= (1 << 4 | 1 << 5); /*RW */
		 clean_va = (MMU_L2_SMALL_ADDR(l2_pt[i])) + curr_vm->guest_info.page_offset
				 	 	 	 	 	 	 	 -curr_vm->guest_info.phys_offset;

		 COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, &l2_pt[i]);
		 dsb();

		 COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA,clean_va);
		 COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, clean_va); /*Update cache with new values*/
		 dsb();
		 isb();
    }
    hypercall_dcache_clean_area((uint32_t)pgd, 0x4000);
}

/*New pages for processes, copys kernel space from master pages table
 *and cleans the cache, set these pages read only for user
 *TODO Add list of used page tables and keep count */
void hypercall_new_pgd(addr_t *pgd)
{
	uint32_t slpt_pa, lvl2_idx, i;
	uint32_t *l2_pt, clean;
	uint32_t PAGE_OFFSET = curr_vm->guest_info.page_offset;
	uint32_t PHYS_OFFSET = curr_vm->guest_info.phys_offset;
	uint32_t pgd_size = 0x4000;

	/*Check page address*/
	if( (uint32_t)pgd < PAGE_OFFSET || (uint32_t)pgd > (uint32_t)(HAL_VIRT_START - pgd_size) )
		hyper_panic("New page global directory does not reside in kernel address space\n", 1);

//	printf("\n\tLinux kernel NEW PGD: %x\n", pgd);
	/*If the requested page is in a section page, we need to modify it to lvl 2 pages
	 *so we can modify the access control granularity */
	slpt_pa = (flpt_va[(uint32_t)pgd >> MMU_L1_SECTION_SHIFT]);
	uint32_t linux_va, pgd_va;

	if(slpt_pa & MMU_L1_TYPE_SECTION) { /*Section page, replace it with lvl 2 pages*/
		pgd_va = MMU_L1_SECTION_ADDR((uint32_t)pgd) ; //Mask out everything except address
		linux_va = MMU_L1_PT_ADDR(flpt_va[(uint32_t)pgd >> MMU_L1_SECTION_SHIFT]) - PHYS_OFFSET + PAGE_OFFSET;
    	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();
		flpt_va[(uint32_t)pgd >> MMU_L1_SECTION_SHIFT] = 0; /*Clear it and replace mapping with small pages*/
		pt_create_coarse (flpt_va,pgd_va, MMU_L1_SECTION_ADDR(slpt_pa), MMU_L1_SECTION_SIZE, MLT_USER_RAM);
		l2_pt = (uint32_t *)(GET_VIRT(MMU_L1_PT_ADDR(flpt_va[(uint32_t)pgd >> MMU_L1_SECTION_SHIFT])));
		clean = (((uint32_t)pgd >> MMU_L1_SECTION_SHIFT) * 4) + (uint32_t)flpt_va;
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,clean);
		dsb();
	}
	/*Here we already got a second lvl page*/
	else {
		l2_pt = (uint32_t *)GET_VIRT(MMU_L1_PT_ADDR(slpt_pa));
	}

	/*Get the index of the page entry to make read only*/
	lvl2_idx = (((uint32_t)pgd << 12) >> 24);

	/*First get the physical address of the lvl 2 page by
	 * looking at the index of the pgd location. Then set
	 * 4 lvl 2 pages to read only (16k page table)*/

    for(i=lvl2_idx; i < lvl2_idx + 4; i++){
    	linux_va = (MMU_L2_SMALL_ADDR(l2_pt[i])) - PHYS_OFFSET + PAGE_OFFSET;
    	COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();
    	l2_pt[i] &= ~(MMU_L2_SMALL_AP_MASK << MMU_L2_SMALL_AP_SHIFT);
		l2_pt[i] |= (MMU_AP_USER_RO << MMU_L2_SMALL_AP_SHIFT); //AP = 2 READ ONLY
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,(uint32_t)&l2_pt[i]);
		dsb();

    }

	/* Page table 0x0 - 0x4000
	 * Reset user space 0-0x2fc0
	 * 0x2fc0 = 0xBF000000 END OF USER SPACE
	 * Copy kernel, IO and hypervisor mappings
	 * 0x2fc0 - 0x4000
	 * */
	memset((void *)pgd, 0, 0x2fc0);
	memcpy((void *)((uint32_t)pgd + 0x2fC0), (uint32_t *)((uint32_t)(flpt_va) + 0x2fc0), 0x1040);

	/*Clean dcache on whole table*/
	hypercall_dcache_clean_area((uint32_t)pgd, 0x4000);
}

/*TODO Add some security check on the val
 * Sets a entry in the current L1 page table
 * If val is 0, it means clean the page so we need to check the current
 * mapped page and remove the write protection.
 * */
void hypercall_set_pmd(addr_t *pmd, uint32_t val)
{
//	printf("\n\tHYP: Set page pgd: %x val: %x \n", pgd, val );
	uint32_t offset, *l1_pt, slpt_pa, sect_idx;
	uint32_t PAGE_OFFSET = curr_vm->guest_info.page_offset;
	uint32_t PHYS_OFFSET = curr_vm->guest_info.phys_offset;
	uint32_t guest_size = curr_vm->guest_info.guest_size;

	/*Security Checks*/
	uint32_t pa = MMU_L1_SECTION_ADDR(val);

	/*Check virtual address*/
	if((uint32_t)pmd < PAGE_OFFSET || (uint32_t)pmd > (uint32_t)(HAL_VIRT_START - sizeof(uint32_t)))
		hyper_panic("Page middle directory reside outside of allowed address space !\n", 1);
	if(val != 0){
		/*Check physical address*/
		if(!(pa >= (PHYS_OFFSET) && pa < (PHYS_OFFSET + guest_size ))){
			printf("Address: %x\n", pa);
			hyper_panic("Guest does not own pte physical address", 1);
		}
	}

	/***********************************************/

	/*Swapper Page*/
	if((uint32_t)pmd >= PAGE_OFFSET + 0x4000 && (uint32_t)pmd < PAGE_OFFSET + 0x8000) {
		offset = (uint32_t)pmd - PAGE_OFFSET - 0x4000; //Pages located 0x4000 below kernel
		l1_pt = (uint32_t *)((uint32_t)flpt_va + offset);
	}
	else{ /* Here the pages are at some physical address, user process page*/
		l1_pt = pmd;
	}
	uint32_t *l1_pt_va, pte;

	/*Contains the page table entry, if the val is 0 we have to
	 *get it from the master page table */
	if(val != 0)
		pte = val;
	else
		pte = l1_pt[0];
	sect_idx = (((pte - PHYS_OFFSET) >> MMU_L1_SECTION_SHIFT)*4) + 0x3000;
	l1_pt_va = (uint32_t *)(sect_idx + (uint32_t)flpt_va);

	slpt_pa = *l1_pt_va;

	if(slpt_pa & MMU_L1_TYPE_SECTION) { /*Section page, replace it with lvl 2 pages*/
		uint32_t linux_va = MMU_L2_SMALL_ADDR(pte) - PHYS_OFFSET + PAGE_OFFSET;
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();
		*l1_pt_va = 0; /*Clear it and replace mapping with small pages*/
		pt_create_coarse (flpt_va, (sect_idx * 4) << 16 , MMU_L2_SMALL_ADDR(slpt_pa), MMU_L1_SECTION_SIZE, MLT_USER_RAM);
		uint32_t clean;
		if((uint32_t)pmd > 0xc0008000)
			clean = (((uint32_t)pmd >> MMU_L1_SECTION_SHIFT) * 4) + (uint32_t)flpt_va;
		else
			clean = (((uint32_t)pmd - 0xc0004000) + (uint32_t)flpt_va);
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,clean);
		dsb();
	}

	if(val != 0){
		uint32_t linux_va = val - PHYS_OFFSET + PAGE_OFFSET;
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();

#if 1
		/*Make virtual address of l2 page RO*/
		uint32_t l2_idx = (((uint32_t)val << 12) >> 24);
		uint32_t l2_pt_phys = *l1_pt_va;
		uint32_t *l2_pt_va = MMU_L1_PT_ADDR(l2_pt_phys - HAL_PHYS_START + HAL_VIRT_START);
		l2_pt_va[l2_idx] &= ~(MMU_L2_SMALL_AP_MASK << MMU_L2_SMALL_AP_SHIFT);
		l2_pt_va[l2_idx] |= (MMU_AP_USER_RO << MMU_L2_SMALL_AP_SHIFT); //AP = 2 READ ONLY
		/*TODO FLUSH THIS VIRT ADDRESS*/
#endif

		l1_pt[0] = val;
		l1_pt[1] = (val + 256 * 4); //(4 is the size of each entry)
		/*Flush entry*/
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)l1_pt);
		dsb();

	}
	else{
		uint32_t linux_va = l1_pt[0] - PHYS_OFFSET + PAGE_OFFSET;
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, linux_va);
		COP_WRITE(COP_SYSTEM, COP_BRANCH_PRED_INVAL_ALL, linux_va);
		dsb();
		isb();

#if 1
		uint32_t l2_idx = (((uint32_t)l1_pt[0] << 12) >> 24);
		uint32_t l2_pt_phys = *l1_pt_va;
		uint32_t *l2_pt_va = MMU_L1_PT_ADDR(l2_pt_phys - HAL_PHYS_START + HAL_VIRT_START);
		l2_pt_va[l2_idx] &= ~(MMU_L2_SMALL_AP_MASK << MMU_L2_SMALL_AP_SHIFT);
		l2_pt_va[l2_idx] |= (MMU_AP_USER_RW << MMU_L2_SMALL_AP_SHIFT); //AP = 3 READ WRITE
		/*TODO FLUSH THIS VIRT ADDRESS*/
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)&l2_pt_va[l2_idx]);
//		mem_mmu_tlb_invalidate_all(TRUE, TRUE);
		dsb();
//		mem_cache_invalidate(FALSE,TRUE,TRUE); //instr, data, writeback
#endif

		l1_pt[0] = 0;
		l1_pt[1] = 0; //(4 is the size of each entry)
		/*Flush entry*/
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)l1_pt);
		dsb();
	}
}

/*Sets an entry in lvl 2 page table*/
void hypercall_set_pte(addr_t *va, uint32_t linux_pte, uint32_t phys_pte)
{
	uint32_t *phys_va = (uint32_t *)((uint32_t)va - 0x800);
	uint32_t PAGE_OFFSET = curr_vm->guest_info.page_offset;
	uint32_t PHYS_OFFSET = curr_vm->guest_info.phys_offset;
	uint32_t guest_size = curr_vm->guest_info.guest_size;

	/*Security Checks*/
	uint32_t pa = MMU_L2_SMALL_ADDR(phys_pte);

	/*Check virtual address*/
	if((uint32_t)phys_va < PAGE_OFFSET || (uint32_t)va > (uint32_t)(HAL_VIRT_START - sizeof(uint32_t)))
		hyper_panic("Page table entry reside outside of allowed address space !\n", 1);

	if(phys_pte != 0) { /*If 0, then its a remove mapping*/
		/*Check physical address*/
		if(!(pa >= (PHYS_OFFSET) && pa < (PHYS_OFFSET + guest_size) )){
			printf("Address: %x\n", pa);
			hyper_panic("Guest trying does not own pte physical address", 1);
		}
	}

	/***********************************************/
//	printf("\n\tHYP: Set pte va: %x phys_pte: %x \n", va, phys_pte );
	*phys_va = phys_pte;
	*va = linux_pte;
	COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, (uint32_t)phys_va );
	dsb();
}
