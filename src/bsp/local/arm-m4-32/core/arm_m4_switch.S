/*
 * This is from the FreeRTOS cortex-m4 switch code.
 */

.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.global kern_task_select
.global current_task
.global arm_m4_task_switch

.section .text.arm_m4_task_switch

/*
 * This context switch stuff uses PSP for the task switching, and isn't
 * at all touching MSP.  This is a bit problematic right now for a few
 * reasons.
 *
 * + the MSP may be being used when we get higher priority exceptions;
 * + I was /kind/ of hoping I could use the MSP for a kernel task stack,
 *   and the PSP for unprivileged process stack.  Then I could keep them
 *   in different memory regions and protect them.
 *
 * So, I think (!) I need to extend a couple of things here.
 *
 * First up, I think I need to add saving both the MSP and PSP and restoring
 * them every context switch.
 *
 * Then for kernel tasks, can I have the MSP be the only stack pointer?
 * Then interrupts would come in on the current stack pointer? Or is that
 * a bit too problematic? (since we can't just have a kernel task stack
 * be sized for /that/ task, it has to handle interrupts too.)
 *
 * For user tasks, there'd be a kernel stack and a userland stack?
 *
 * Is this something I want to do with MSP = priv, PSP = unpriv, OR
 * do I want a global MSP, and PSP that gets shuffled around based on
 * whether I'm in priv or unpriv mode?
 */

arm_m4_task_switch:

	mrs r0, psp
	isb

	/* Get the location of the current TCB. */
	ldr	r3, =current_task
	ldr	r2, [r3]

	/* Is the task using the FPU context?  If so, push high vfp registers. */
	tst r14, #0x10
	it eq
	vstmdbeq r0!, {s16-s31}

	/* Save the core registers. */
	stmdb r0!, {r4-r11, r14}
	/* Save the new top of stack into the first member of the TCB. */
	str r0, [r2]

	stmdb sp!, {r0, r3}

	/* Mask other interrupts above 0 whilst we figure out which task */
	mov r0, #0xffffffff
	msr basepri, r0
	dsb
	isb
	bl kern_task_select
	mov r0, #0
	msr basepri, r0
	ldmia sp!, {r0, r3}

	/* The first item in pxCurrentTCB is the task top of stack. */
	ldr r1, [r3]
	ldr r0, [r1]

	/* Pop the core registers. */
	ldmia r0!, {r4-r11, r14}

	/*
	 * Is the task using the FPU context?  If so, pop the high
	 * vfp registers too.
	 */
	tst r14, #0x10
	it eq
	vldmiaeq r0!, {s16-s31}

	msr psp, r0
	isb

	bx r14

.size arm_m4_task_switch, .-arm_m4_task_switch
