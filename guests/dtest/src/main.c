
#include <lib.h>
#include <types.h>
    
#include "hypercalls.h"
    
// TEMP STUFF
enum dmmu_command { 
};



{
	
	
	
	    /* syscall_dmmu(CMD_CREATE_L1, &l1, 0); */ 
	    
	    /* syscall_dmmu(CMD_CREATE_L1, 0xF0000000, 0); */ 
	    /* syscall_dmmu(CMD_CREATE_L1, 0x00000000, 0); */ 
	    /* syscall_dmmu(CMD_CREATE_L1, 0x10000000, 0); */ 
	    // test hypercall from guest
	    // ISSUE_HYPERCALL_REG1(HYPERCALL_NEW_PGD, 0x01234567);
	    for (;;) {
		
			asm("nop");
		
	



/*Each guest must provide a handler rpc*/ 
void handler_rpc(unsigned callNum, void *params) 
{
