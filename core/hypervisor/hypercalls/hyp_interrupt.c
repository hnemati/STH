#include "hw.h"
#include "hyper.h"
extern virtual_machine *curr_vm;

/*Interrupt operations*/

/*interrupt specifies the irq&fiq mask and op specifies the operation (disable, enable, restore)
 *restore operation will restore the flags to the interrupt mask */
void hypercall_interrupt_set(uint32_t interrupt, uint32_t op)
{
	uint32_t pending_irq;
	interrupt &= (ARM_IRQ_MASK | ARM_FIQ_MASK);
	if (op == 1) {		/*Enable */
		curr_vm->current_mode_state->ctx.psr &= ~(interrupt);

		pending_irq = check_pending_interrupts();
		if (pending_irq != 0xFFFFFFFF)
			irq_handler(pending_irq, 0, 0);

	} else if (op == 2) {	/*Restore ,restores the flag according to param0 */
		curr_vm->current_mode_state->ctx.psr &=
		    ~(ARM_IRQ_MASK | ARM_FIQ_MASK);
		curr_vm->current_mode_state->ctx.psr |= interrupt;

		if (!(curr_vm->current_mode_state->ctx.psr & IRQ_MASK)) {
			pending_irq = check_pending_interrupts();
			if (pending_irq != 0xFFFFFFFF)
				irq_handler(pending_irq, 0, 0);
		}
	} else if (op == 0) {	/*Disable */
		curr_vm->current_mode_state->ctx.psr |= interrupt;
	} else if (op == 3) {	/*Save IRQ and disable if interrupt !0 */
		curr_vm->current_mode_state->ctx.reg[0] =
		    curr_vm->current_mode_state->ctx.psr;
		curr_vm->current_mode_state->ctx.psr |= interrupt;
	} else
		hyper_panic("Unknown interrupt operation", 1);
}

void hypercall_interrupt_ctrl(uint32_t irq, uint32_t control)
{
#if 0				/*Guest cant control this anymore */
	switch (control) {
	case 0:
		unmask_interrupt(irq, 0);
		break;
	case 1:
		mask_interrupt(irq, 0);
		break;
	case 2:
		ack_interrupt(irq);
		break;
	default:
		printf("Invalid control: %d\n", control);

	}
#endif
}

#if 0
void hypercall_irq_save(uint32_t * param)
{
	uint32_t cpsr;

	/*Read CPSR from guest context */
	cpsr = curr_vm->current_mode_state->ctx.psr;

	/*Return value in reg0 */
	curr_vm->current_mode_state->ctx.reg[0] = cpsr;
	/*Disable IRQ */
	cpsr |= ARM_IRQ_MASK;
	curr_vm->current_mode_state->ctx.psr = cpsr;
}

void hypercall_irq_restore(uint32_t flag)
{
	/*Only let guest restore IRQ, FIQ flags not mode */
	flag &= (ARM_IRQ_MASK | ARM_FIQ_MASK);

	curr_vm->current_mode_state->ctx.psr &=
	    ~(ARM_IRQ_MASK | ARM_FIQ_MASK);
	curr_vm->current_mode_state->ctx.psr |= flag;
}
#endif

void hypercall_end_interrupt(uint32_t irq_regs)
{
	if (curr_vm->current_guest_mode != HC_GM_KERNEL) {
		hyper_panic
		    ("Guest tried to end interrupt but not in kernel mode.",
		     1);
	}
	if (curr_vm->interrupted_mode >= HC_NGUESTMODES) {
		hyper_panic("Invalid interrupted mode value.", 2);
	}
#if LINUX
	if (irq_regs < 0xC0000000
	    || (irq_regs >= 0xf0000000 && irq_regs <= 0xf0100000)) {
		printf("irq_reg: %x", irq_regs);
		hyper_panic("Irq reg not from kernel space", 1);
	}
	uint32_t *sp_pop = (uint32_t *) irq_regs;
	/*Check whether the interrupted context is in kernel or user mode */
	if (sp_pop[15] < 0xc0000000 && !(sp_pop[16] & 3))
		curr_vm->interrupted_mode = HC_GM_TASK;
	else {
		if (sp_pop[15] >= 0xc0000000 && (sp_pop[16] & 3))
			curr_vm->interrupted_mode = HC_GM_KERNEL;
		else {
			hyper_panic
			    ("Trying to restore context with wrong privileged mode",
			     0);
		}
	}

	uint32_t *context =
	    curr_vm->mode_states[curr_vm->interrupted_mode].ctx.reg;

	uint32_t i;
	for (i = 0; i < 17; i++) {
		*context++ = *sp_pop++;
	}
	*sp_pop = 0xFFFFFFFF;	//ORIG_R0
#endif
	change_guest_mode(curr_vm->interrupted_mode);
}
