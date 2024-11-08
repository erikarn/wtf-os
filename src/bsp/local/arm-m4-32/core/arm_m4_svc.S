.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.global arm_m4_svc_handler
.global arm_m4_syscall_handler
.global arm_m4_c_syscall_handler

.section .text.arm_m4_syscall_handler

/*
 * The THREAD part of the syscall handler.
 *
 * This sets up the privileged stack, jumps into the kernel C
 * code to process the syscall, then restores the unprivilege
 * mode and returns to the original caller.
 *
 * r8 contains the original PC to jump back into.
 *
 * XXX TODO: can I /use/ r8 like this? I thought r8 needed to
 * be preserved!  I may need to go and actually push r8
 * onto the kernel stack in my svc handler and pop it before
 * I return here.
 */

arm_m4_syscall_handler:

	/*
	 * Fetch the top of the kernel task stack in userland
	 * mode.
	 */
	ldr ip, =current_task
	ldr ip, [ip]
	ldr ip, [ip, #4]

	/*
	 * Store the current SP/LR at the beginning of this
	 * kernel stack.
	 */
	subs ip, #8
	str sp, [ip, #0]
	str lr, [ip, #4]

	/* Switch */
	msr PSP, ip

	/* Branch to our syscall handler in C */
	ldr ip, =arm_m4_c_syscall_handler
	orrs ip, ip, #1
	blx ip
	/* Note: r0 has a return value now! */

	/* restore LR */
	ldr lr, [sp, #4]
	/* restore SP to original unpriv stack location */
	ldr ip, [sp, #0]
	msr PSP, ip

	/*
	 * Fetch the original function location, force thumb mode.
	 * store it in r1 so we can use it across the unpriv boundary
	 * below.
	 */
	ldr ip, =current_task
	ldr ip, [ip]
	ldr ip, [ip, #8]
	orrs ip, ip, #1
	mov r1, ip

	/* Drop back to unpriv */
	/*
	 * At this point the MPU may be enabled, we can't touch
	 * non-task memory (eg the TCB will be unavailable!)
	 */
	mrs ip, CONTROL
	orrs ip, ip, #1
	msr CONTROL, ip

	/* barrier - don't run next code in old priv */
	isb

	/* Grab the return address back into IP for return */
	mov ip, r1

	/* Zero out caller-saved registers to not leak state */
	/* (leave r0, it has a 32 bit return value) */
	mov r1, #0
	mov r2, #0
	mov r3, #0

	/* Branch back */
	bx ip
.size arm_m4_syscall_handler, .-arm_m4_syscall_handler

.section .text.arm_m4_svc_handler

/*
 * Handle an SVC by (for now) treating it as a syscall and trampolining
 * through it in HANDLER mode back to THREAD mode, but in kernel mode
 * and running the syscall handler.
 *
 * Then when the syscall handler returns we'll trampoline back into
 * the userspace code with the return values.
 *
 * The stack frame saved in the exception is:
 *
 * r0, r1, r2, r3, r12/IP, r14/LR, r15/PC, XPSR
 *
 * Entry /into/ the SVC handler we're going to jump
 * into looks like (for debugging, so we don't reuse
 * the values accidentally!)
 *
 * r0: 0x0
 * r1: 0x0
 * r2: 0x0
 * r3: 0x0
 *
 * So, we can use r0, r1, r2, r3 as temporary
 * registers; they'll be restored before the SVC
 * routine is jumped into.
 */

arm_m4_svc_handler:

	/* Figure out where our calling stack is */
	tst lr, #4
	ite eq
	mrseq r0, MSP
	mrsne r0, PSP

	/* Figure out which SVC call was invoked - 2 byte instruction */
	ldr r1, [r0, #24]	/* PC from stack frame */
	ldrb r1, [r1, #-2]	/* Get SVC code from second byte */

	/* Save control to modify before we jump */
	mrs r2, CONTROL

	/*
	 * Get IP pointing to our current_task struct, we'll store our
	 * current PC there.
	 */
	ldr ip, =current_task
	ldr ip, [ip]

	/*
	 * When we jump into our syscall handler code, we have a few
	 * other registers to setup as noted above.
	 */
	ldr r1, [r0, #24]	/* PC from stack frame */
	str r1, [ip, #8]	/* Third entry in struct - store return PC */

	/* Overwrite the return PC */
	ldr r1, =arm_m4_syscall_handler
	str r1, [r0, #24]

	/* Set mode to privileged for the return */
	bic r2, #1
	msr CONTROL, r2

	/*
	 * Ensure we don't have anything running under the
	 * previous privilege level.
	 */
	isb

	/* Debug - clear these out to make sure I'm not leaking */
	mov r0, #0
	mov r1, #0
	mov r2, #0
	mov r3, #0

	/*
	 * Return from SVC exception (in handler mode) to our
	 * syscall handler (in thread mode.)
	 */
	bx lr

.size arm_m4_svc_handler, .-arm_m4_svc_handler
