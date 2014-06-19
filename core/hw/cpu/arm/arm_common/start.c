#include "hyper.h"

extern virtual_machine *curr_vm;

void start(){
    asm volatile ("mov LR, %0      \n\t" :: "r"(curr_vm->config->guest_entry_point));
#ifdef LINUX
    /*Prepare r0 r1 and r2 for linux boot */
    asm volatile("mov r1, %0      \n\t" :: "r"(LINUX_ARCH_ID));
    asm volatile("mov r2, %0      \n\t" :: "r"(curr_vm->config->guest_entry_point - 0x10000 + 0x100));
    asm volatile("mov r0, #0 \n\t");
#endif
    asm volatile("MSR SPSR, #0xD0 \n\t");
    asm volatile("MOVS PC, LR \n\t");

}
