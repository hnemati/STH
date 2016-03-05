/*
 * ARM context top file.
 * this file should be included on top of your exception handler
 * It contains the low level context handling definitions and macros.
 */

#define INDEX_R0 0    
#define INDEX_SP 13
#define INDEX_LR 14
#define INDEX_PC 15
#define INDEX_PSR 16   


    /*
     * saves context, modifies R12 and LR
     */
     .macro store_context
     
     push { r0-r3 }    
     ldr r0, =context_stack_curr
     ldr r1, =context_stack
     ldr r2, [r0]
     add r2, r2, #1     
     ldr r1, [r1, r2, lsl #2]
     str r2, [r0]

     str lr, [r1, #4 * INDEX_PC]
     mov lr, r1
     pop { r0-r3 }
     
     stmia lr, {r0-lr}^
     mrs r12, spsr
     
     /*Virtual Interrupts, save guest virtual IRQ and FIQ into psr (real irq and fiq always on in guest) 
*/
     ldr r11, [lr, #4 * INDEX_PSR]
     and r11, #0xDF
     orr r12, r11
     
     str r12, [lr, #4 * INDEX_PSR]    
     .endm
     
     


    /*
     * saves context, modifies R12 but retains LR
     */
     .macro store_context_keep_lr
     
     push { r0-r3 }    
     ldr r0, =context_stack_curr
     ldr r1, =context_stack
     ldr r2, [r0]
     add r2, r2, #1
     ldr r1, [r1, r2, lsl #2]
     str r2, [r0]
     
    
     str lr, [r1, #4 * INDEX_PC]
     mov lr, r1
     pop { r0-r3 }
    
     stmia lr, {r0-lr}^
     mrs r12, spsr

/*Virtual Interrupts, save guest virtual IRQ and FIQ into psr (real irq and fiq always on in guest) 
*/
     ldr r11, [lr, #4 * INDEX_PSR]
     and r11, #0xDF
     orr r12, r11
     
     str r12, [lr, #4 * INDEX_PSR]    
     ldr lr, [lr, #4 * INDEX_PC]
     .endm
     
     
.macro restore_context

     ldr r0, =context_stack_curr
     ldr r1, =context_stack
     ldr r2, [r0]
     ldr lr, [r1, r2, lsl #2]
     sub r2, r2, #1     
     str r2, [r0]
     
     ldr r0, [lr, #4 * INDEX_PSR]
     #and r0,  #0xFFFFFFF0 @Ensure user mode
     and r0,   #0xFFFFFF30 @Ensure user mode and IRQ and FIQ always enabled 
     msr spsr, r0
     ldmia lr, {r0-lr} ^
     ldr lr, [lr, #4 * INDEX_PC]    
.endm

