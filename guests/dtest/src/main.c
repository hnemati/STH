
#include <lib.h>
#include <types.h>
    
#include "hypercalls.h"
#include "print_err.h"
    uint32_t l1[4096] __attribute__ ((aligned(16 * 1024)));
uint32_t l2[1024] __attribute__ ((aligned(4 * 1024)));
 
// TEMP STUFF
enum dmmu_command 
    { CMD_MAP_L1_SECTION, CMD_UNMAP_L1_PT_ENTRY, CMD_CREATE_L2_PT,
CMD_MAP_L1_PT, CMD_MAP_L2_ENTRY, CMD_UNMAP_L2_ENTRY, CMD_FREE_L2,
CMD_CREATE_L1_PT, CMD_SWITCH_ACTIVE_L1, CMD_FREE_L1 
};
  extern uint32_t syscall_dmmu(uint32_t r0, uint32_t r1, uint32_t r2);

#define ISSUE_DMMU_HYPERCALL(type, p0, p1, p2) \
    syscall_dmmu(type | (p2 << 4), p0, p1);
 
#define ISSUE_DMMU_HYPERCALL_(type, p0, p1, p2, p3) \
    syscall_dmmu((type | (p2 & 0xFFFFFFF0)), p0, ((p1 << 20) | p3));
 void dmmu_map_L1_section_() 
{
	uint32_t va, pa, attrs, res;
	int t_id = 0;
	
	    // #0: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	    va = 0x0;
	pa = 0x0;
	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
	t_id++;
	 
	    // #1: mapping 0xc0200000 is ok, since it is the page containing the active page table
	    // This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	    va = 0xc0200000;
	pa = 0x0;
	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
	t_id++;
	 
	    // #2: mapping 0xc020000 is ok, since it is the page containing the active page table
	    // This test should fail, because this is already mapped
	    va = 0xc0000000;
	pa = 0x81000000;
	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
	t_id++;
	
	    // #3: mapping 0xc020000 is ok, since it is the page containing the active page table
	    // This test should fail, because the access permission is not supported
	    va = 0xc0200000;
	pa = 0x81000000;
	attrs = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
	t_id++;
	 
	    // #4: mapping 0xc020000 is ok, since it is the page containing the active page table
	    // This test should succeed
	    va = 0xc0200000;
	pa = 0x81000000;
	
	    //attrs = 0x12; // 0b1--10
	    //attrs |= MMU_AP_USER_RW << MMU_SECTION_AP_SHIFT;
	    //attrs = (attrs & (~0x10)) | 0xC | (HC_DOM_KERNEL << MMU_L1_DOMAIN_SHIFT);
	    attrs = 0xc2e;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
	t_id++;
	 
	    // #5: mapping 0xc030000 with read-only access permission is ok, since it is the page containing the active page table
	    // This test should succeed
	    va = 0xc0300000;
	pa = 0x81000000;
	attrs = 0xb2e;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	print_2_err(t_id, "MAP L1 SECTION", va, res);
}  void dmmu_unmap_L1_pageTable_entry_() 
{
	uint32_t va, res;
	int t_id = 0;
	
	    // #0: I can not unmap 0, since it is reserved by the hypervisor to access the guest page tables
	    va = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	print_2_err(t_id, "UNMAP L1 PT ENTRY", va, res);
	t_id++;
	 
	    // #1: I can not unmap 0xf0000000, since it is reserved by the hypervisor code
	    va = 0xf0000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	print_2_err(t_id, "UNMAP L1 PT ENTRY", va, res);
	t_id++;
	 
	    // #2: Unmapping 0xc0300000 has no effect, since this page is unmapped
	    va = 0xc0300000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	print_2_err(t_id, "UNMAP L1 PT ENTRY", va, res);
	t_id++;
	 
	    // #3: Unmapping 0xc0200000 is ok if this test is executed after the l1_map_section test, otherwise it has no effect
	    va = 0xc0200000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	print_2_err(t_id, "UNMAP L1 PT ENTRY", va, res);
	t_id++;
	 
	    // #4: Unmapping 0xc0000000 is ok, but this is the page where the guest code resides
	    //printf("test 5: THIS WILL BRAKE THE GUEST\n");
/*
	va = 0xc0000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	print_2_err(t_id,"UNMAP L1 PT ENTRY", va, res);
*/ 
}  void dmmu_create_L2_pt_() 
{
	uint32_t pa, va, sec_va, attrs, res;
	int j, t_id = 0;
	
/*
	  // #0 : Guest can not write its own l2 page table in an unmapped area
	  // This test will break the system (Dabort)
	  sec_va = 0x300000;
	  attrs = 0;
	  va = 0xc0300000;
	  pa = 0x81100000;
	  for(j = 0; j < 1024; j++)
		  l2[j] = ((uint32_t)0x32);
	  memcpy((void*)sec_va, l2, sizeof l2);
*/ 
	    t_id++;
	  
	    // #1 : Guest can not write its own l2 page table in a physical address outside the allowed range
	    // This test should fail because physical address 0x0 is not accessible by the guest
	    pa = 0x0;		// setting pa to an address which will case a failure
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	  
	    // #2: Guest can not use an address which is not 4KB aligned to create an L2
	    // this test should fail because we are only allowed to create l2 in a 4KB aligned address
	    va = 0x112000;
	pa = 0x81100100;	// this address is not 4KB aligned
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x32);
	memcpy((void *)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	  
	    // #3: Guest can not create a new L2 in a region which already contains an L2
	    attrs = 0xc2e;
	va = 0x1b0000;
	pa = 0x811b0000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x32);
	memcpy((void *)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);	// this should fail
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	 
	    // #4: Guest can not create a new L2 in a region which already contains a page table or a referenced data page
	    //*******************************************************//
	    // Creating an L2 to map
	    for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x0);
	memcpy((void *)0x1e0000, l2, sizeof l2);
	pa = 0x811e0000;	// L2 base address
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	
	    //*******************************************************//
	    uint32_t pga = 0x81110000;	// data page address
	uint32_t idx = 0xc2;
	attrs = 0x32;
	
	    // TODO: system call need to be modified to hold 4 parameters. For now I am writing pga and attrs arguments manually in r4, r3.
 asm("mov  r4, %[value] \n\t" : :[value] "r"(attrs) /*input */ 
 :				/* No clobbers */ );
 asm("mov  r3, %[value] \n\t" : :[value] "r"(pga) /*input */ 
 :				/* No clobbers */ );
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L2_ENTRY, pa, idx, 0);
	
	    // here pa is pointing to a referenced data page
	    pa = 0x81110000;
	va = 0x100000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x32);
	memcpy((void *)va, l2, sizeof l2);
	
	    // Commenting the next line will cause this test to fail because the reference counter of pointed data page is not zero
	    //ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	    res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);	//referenced data page
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	 
	    // #5: Guest can not create a new L2 with an unsupported descriptor type (0x3)
	    attrs = 0xc2e;
	va = 0x140000;
	pa = 0x81140000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x31);
	memcpy((void *)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CMD CREATE L2 PT", pa, res);
	t_id++;
	 
	    // #6: Guest can not create a new L2 with an entry which point to L2 page table itself with a writable  access permission
	    attrs = 0xc2e;
	va = 0x160000;
	pa = 0x81160000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x81160032);	//self reference with ap = 3, it successfully failed
	memcpy((void *)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	 
	    // #7: Guest can create a new L2 with an entry which point to another page table with a read-only  access permission
	    attrs = 0xc2e;
	va = 0x190000;
	pa = 0x81190000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x811e0022);	//self reference with ap = 2, it succeed
	memcpy((void *)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
	 
	    // #8: Guest can not create a new L2 with an entry which point to another page table with a writable access permission
	    attrs = 0xc2e;
	va = 0x1a0000;
	pa = 0x811a0000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x811e0032);	// pointing to guest initial l1 with ap = 3, it successfully failed
	memcpy((void *)va, l2, sizeof l2);
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L2 PT", pa, res);
	t_id++;
 }

 void dmmu_l1_pt_map_() 
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	 
	    // Creating an L1 to map
	    attrs = 0xc2e;
	va = 0x170000;
	pa = 0x81170000;
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x32);
	memcpy((void *)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	
	    // end of L2 page table creation
	    
	    // #1: I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	    attrs = 0x0;
	va = 0x0;
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	print_2_err(t_id, "MAP L1 PT", va, res);
	t_id++;
	 
	    // #2: mapping 0xc0200000 is ok, since it is the page containing the active page table
	    // This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	    attrs = 0x0;
	va = 0xc0200000;
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	print_2_err(t_id, "MAP L1 PT", va, res);
	t_id++;
	 
	    // #3: mapping 0xc0200000 is ok, since it is the page containing the active page table
	    // This test should fail, because 0x81110000 does not point to a valid L2
	    attrs = 0x0;
	va = 0xc0200000;
	pa = 0x81110000;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	print_2_err(t_id, "MAP L1 PT", va, res);
	t_id++;
	 
	    // #4: mapping 0xc0200000 is ok, since it is the page containing the active page table
	    // This test should fail, because PXN is enabled
	    attrs = 0xc25;
	va = 0xc0200000;
	pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	print_2_err(t_id, "MAP L1 PT", va, res);
	t_id++;
	 
	    // #5: mapping 0xc0300000 is ok, since it is the page containing the active page table
	    // This test should fail, because guest can not map an L2 in a given entry two times in row
	    va = 0xc0300000;
	pa = 0x81100000;
	attrs = 0xc21;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	res = ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);	// this call should fail because the entry has already been mapped
	print_2_err(t_id, "MAP L1 PT", va, res);
	t_id++;
}

 void dmmu_l2_map_entry_() 
{
	
	    // idx is the index of a entry we want to map pga into
	    uint32_t pa, va, idx, pga, attrs, res;
	int j, t_id = 0;
	
	    // Creating an L2 to map its entry
	    attrs = 0xc2e;
	va = 0xc0300000;
	pa = 0x81100000;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x0);
	memcpy((void *)va, l2, sizeof l2);
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	
	    // end of L2 page table creation
	    
	    // #0: L2 base can not be 0, since it is reserved by the hypervisor to access the guest page tables
	    pga = 0x81110000;
	idx = 0xc2;
	attrs = 0x32;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, 0x0, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #1: L2 base address is ok, but guest can not map a page outside the allowed range into its L2 page table entries
	    pga = 0x0;
	idx = 0xc2;
	attrs = 0x32;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	  
	    // #2: All the parameters are correct and this test should succeed
	    pga = 0x81110000;
	idx = 0xc2;
	attrs = 0x32;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #3: this test should fail, because the entry has already been mapped.
	    res =
	    ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #4: this test should fail, because guest can not map a page table to an entry of the given L2 with writable access permission
	    res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pa, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #5: this test should fail, because guest can not map any thing to an entry of a data page
	    res =
	    ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pga, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #6: this test should fail, because guest is passing an unsupported  access permission
	    attrs = 0x02;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #7: All the parameters are correct and this test should succeed
	    // in this test reference counter of the mapped page should not be increased
	    pga = 0x81110000;
	idx = 0xab;
	attrs = 0x22;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
	t_id++;
	 
	    // #8: All the parameters are correct and this test should succeed
	    // in this test guest is mapping the L2 base address into one of L2 entry with read-only access permission
	    idx = 0xac;
	attrs = 0x22;
	res = ISSUE_DMMU_HYPERCALL_(CMD_MAP_L2_ENTRY, pa, idx, pga, attrs);
	print_3_err(t_id, "MAP L2 ENTRY", pa, pga, res);
}

 void dmmu_l2_unmap_entry_() 
{
	
	    // this test is done in combination with l2_pt_map API
	    // idx is the index of a entry we want to unmap it
	    uint32_t pa, idx, res;
	int t_id;
	 
	    // #0: L2 base address can not be 0x0, since it is reserved by the hypervisor to access the guest page tables
	    idx = 0xc2;
	pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	print_3_err(t_id, "UNMAP L2 ENTRY", pa, idx, res);
	t_id++;
	 
	    // #1: The guest is trying to unmap entry of a data page
	    // This test should fail because the l2 base address is not pointing to a valid page table(L2)
	    idx = 0xc2;
	pa = 0x81110000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	print_3_err(t_id, "UNMAP L2 ENTRY", pa, idx, res);;
	t_id++;
	 
	    // #1: The entry guest is trying to unmap should be mapped
	    // This test should fail because the L2 page table entry which guest tries to unmap is not mapped beforehand
	    idx = 0xe2;
	pa = 0x81100000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	print_3_err(t_id, "UNMAP L2 ENTRY", pa, idx, res);
	t_id++;
	 
	    // #2: all the parameters are well defined
	    // idx is the index of an L2 entry which points to a page table
	    // This test should succeed but the reference counter should remain untouched
	    idx = 0xac;
	pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	print_3_err(t_id, "UNMAP L2 ENTRY", pa, idx, res);
	t_id++;
	 
	    // #3: this test is a successful attempt
	    idx = 0xc2;
	pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L2_ENTRY, pa, idx, 0);
	print_3_err(t_id, "UNMAP L2 ENTRY", pa, idx, res);
}  void dmmu_unmap_L2_pt_() 
{
	
	    // this test has been using l2_create and l2_pt_map
	    uint32_t pa, va, attrs, res;
	int t_id = 0;
	 
	    // #0: this test should fail because L2 base address can not be 0x0, since it is reserved by the hypervisor to access the guest page tables
	    pa = 0x0;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	print_2_err(t_id, "FREE L2", pa, res);
	t_id++;
	 
	    // #1: this test should fail because L2 base address does not point to a valid L2
	    pa = 0x81110000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	print_2_err(t_id, "FREE L2", pa, res);
	t_id++;
	 
	    // #2: if the base address is not 4KB aligned, in our model, for sure there is not valid L2 inside the page frame pointed by that address
	    t_id++;
	 
	    // #3: this test should fail because guest is trying to free a referenced L2
	    //dmmu_l2_map_entry_();
	    va = 0xc0300000;
	pa = 0x81100000;
	attrs = 0xc21;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_PT, va, pa, attrs);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	print_2_err(t_id, "FREE L2", pa, res);
	t_id++;
	 
	    // #4: this test should succeed
	    pa = 0x81100000;
	va = 0xc0300000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L2, pa, 0, 0);
	print_2_err(t_id, "FREE L2", pa, res);
}  void dmmu_create_L1_pt_() 
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	 
	    // #0: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	    pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #1: this test should fail because L1 base is not aligned
	    pa = 0x81101000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #2: this test should fail because guest is trying to create a new page table in a part of the memory where another L1 resides in
	    pa = 0x81200000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #3: this test should fail because guest is trying to create a page table using referenced data pages
	    //*******************************************************//
	    // Creating an L2 to map
	    for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x0);
	memcpy((void *)0x1e0000, l2, sizeof l2);
	pa = 0x811e0000;	// L2 base address
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	
	    //*******************************************************//
	    uint32_t pga = 0x81110000;	// data page address
	uint32_t idx = 0xc2;
	attrs = 0x32;
	
	    // TODO: system call need to be modified to hold 4 parameters. For now I am writing pga and attrs arguments manually in r4, r3.
 asm("mov  r4, %[value] \n\t" : :[value] "r"(attrs) /*input */ 
 :				/* No clobbers */ );
 asm("mov  r3, %[value] \n\t" : :[value] "r"(pga) /*input */ 
 :				/* No clobbers */ );
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L2_ENTRY, pa, idx, 0);
	
	    // here pa is pointing to a referenced data page
	    pa = 0x81110000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #4: this test should fail because guest is trying to map a non-L2 page table page into L1 as a L2 page table
	    // Writing content of the new L1 page table
	    l1[0] = ((uint32_t) 0x81110001);	// L2 descriptor
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #5: this test should fail because guest is trying to use super section descriptor instead of a section descriptor
	    // Writing content of the new L1 page table
	    l1[0] = ((uint32_t) 0x81240802);	// L2 descriptor
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #6: this test should fail because guest is trying to use an unsupported access permission in a section descriptor
	    // Writing content of the new L1 page table
	    l1[0] = ((uint32_t) 0x81200002);	// L2 descriptor (ap = 0 it is unsupported)
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #7: this test should fail because guest is trying to map part of the memory as a writable section where I initial page table of guest resides
	    // Writing content of the new L1 page table
	    l1[0] = ((uint32_t) 0x81200C02);	// L2 descriptor
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #8: this test should fail because guest is trying to map part of the memory as a writable section where it is trying to create the new L1
	    // Writing content of the new L1 page table
	    l1[0] = ((uint32_t) 0x81100C02);	// L2 descriptor
	for (j = 1; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	 memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #9 ....
	    // creating a writable section to map
	    // for this test minimal_config.c has been modified and now ".pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff"
	    attrs = 0xc2e;
	va = 0xc0500000;
	pa = 0x81300000;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	
	    // after this test I changed minimal_config.c file to its previous value ".pa_for_pt_access_end = HAL_PHYS_START + 0x012fffff"
	    l1[0] = ((uint32_t) 0x81300C02);	// section descriptor with write access,  mapping of this section succeed
	//l1[1] = ((uint32_t)0x81200802); // section descriptor with read-only access
	l1[1] = ((uint32_t) 0x81200C02);	// section descriptor with write access , this should make the L1 invalid and it was a successful attempt
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
	t_id++;
	 
	    // #10 ....
	    // Creating an L2 to map
	    for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x0);
	va = 0x150000;
	memcpy((void *)va, l2, sizeof l2);
	
	    // end of L2 page table creation
	    
	    // Writing content of the new L1 page table
	    for (j = 0; j < 4096; j++)
		l1[j] = ((uint32_t) 0x81150001);	// L2 page table descriptor
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	
	    // end of L1
	    va = 0xc0300000;
	ISSUE_DMMU_HYPERCALL(CMD_UNMAP_L1_PT_ENTRY, va, 0, 0);
	pa = 0x81150000;
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	  pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	print_2_err(t_id, "CREATE L1 PT", pa, res);
 }

 void dmmu_switch_mm_() 
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	 
	    // #0: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	    pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	print_2_err(t_id, "SWITCH ACTIVE L1", pa, res);
	t_id++;
	 
	    // #1: this test should fail because L1 base is not aligned
	    pa = 0x81101000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);
	print_2_err(t_id, "SWITCH ACTIVE L1", pa, res);
	t_id++;
	 
	    // #2: this test should fail because guest is trying to switch into a non-page table page
	    pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);	// just to see if it possible to switch the active L1 or not
	print_2_err(t_id, "SWITCH ACTIVE L1", pa, res);
	t_id++;
	 
	    // start: creating an L1
	    // for this test minimal_config.c has been modified and now ".pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff"
	    attrs = 0xc2e;
	va = 0xc0500000;
	pa = 0x81300000;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	
	    // after this test I changed minimal_config.c file to its previous value ".pa_for_pt_access_end = HAL_PHYS_START + 0x012fffff"
	    l1[0] = ((uint32_t) 0x81300C02);	// section descriptor with write access,  mapping of this section succeed
	for (j = 1; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	
	    // end: creating an L1
	    pa = 0x81100000;
	res = ISSUE_DMMU_HYPERCALL(CMD_SWITCH_ACTIVE_L1, pa, 0, 0);	// just to see if it possible to switch the active L1 or not
	print_2_err(t_id, "SWITCH ACTIVE L1", pa, res);
}

 void dmmu_unmap_L1_pt_() 
{
	uint32_t pa, va, attrs, res;
	int j, t_id = 0;
	
	    // #0: this test should fail because guest is trying to create a new page table in a part of the memory that is reserved for hypervisor use
	    pa = 0x80000000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	print_2_err(t_id, "FREE L1", pa, res);
	t_id++;
	 
	    // #1: this test should fail because base address is not aligned
	    pa = 0x81101000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	print_2_err(t_id, "FREE L1", pa, res);
	t_id++;
	 
	    // #2: this test should fail because base address is not pointing to a L1 page table
	    pa = 0x811C0000;
	res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	print_2_err(t_id, "FREE L1", pa, res);
	t_id++;
	 
	    // #3: this test should succeed, section
	    // start: creating an L1
	    // for this test minimal_config.c has been modified and now ".pa_for_pt_access_end = HAL_PHYS_START + 0x014fffff"
	    attrs = 0xc2e;
	va = 0xc0500000;
	pa = 0x81300000;
	ISSUE_DMMU_HYPERCALL(CMD_MAP_L1_SECTION, va, pa, attrs);
	l1[0] = ((uint32_t) 0x81300C02);	// section descriptor with write access,  mapping of this section succeeds
	l1[1] = ((uint32_t) 0x81200802);	// section descriptor with read-only access
	for (j = 2; j < 4096; j++)
		l1[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l1, sizeof l1);
	 pa = 0x81100000;	// L1 base address
	ISSUE_DMMU_HYPERCALL(CMD_CREATE_L1_PT, pa, 0, 0);
	
	    // end: creating an L1
	    res = ISSUE_DMMU_HYPERCALL(CMD_FREE_L1, pa, 0, 0);
	print_2_err(t_id, "FREE L1", pa, res);
	t_id++;
	 
	    // #4: this test should succeed, because after unmapping an L1 guest should be able to treat freed page as regular data pages
	    // Creating an L2 to map
	    for (j = 0; j < 1024; j++)
		l2[j] = ((uint32_t) 0x0);
	va = 0x100000;
	memcpy((void *)va, l2, sizeof l2);
	
	    // end of L2 page table creation
	    res = ISSUE_DMMU_HYPERCALL(CMD_CREATE_L2_PT, pa, 0, 0);
	print_2_err(t_id, "FREE L1", pa, res);
}

void _main() 
{
	int j;
	printf("starting\n");
	for (;;) {
		for (j = 0; j < 500000; j++)
			asm("nop");
		
		    //dmmu_map_L1_section_();
		    //dmmu_unmap_L1_pageTable_entry_();
		    //dmmu_create_L2_pt_();
		    //dmmu_l1_pt_map_();
		    dmmu_l2_map_entry_();
		
		    //dmmu_l2_unmap_entry_();
		    //dmmu_unmap_L2_pt_();
		    //dmmu_create_L1_pt_();
		    //dmmu_switch_mm_();
		    //dmmu_unmap_L1_pt_();
		    printf("running\n");
	}
}

 
/*Each guest must provide a handler rpc*/ 
void handler_rpc(unsigned callNum, void *params) 
{
 } 
