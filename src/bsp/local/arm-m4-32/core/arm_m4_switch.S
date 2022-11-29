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
.global arm_m4_mpu_enable
.global arm_m4_mpu_disable

.section .text.arm_m4_task_switch

/*
 * This context switch stuff uses PSP for the task switching, and isn't
 * touching MSP.  MSP is used for top level interrupt/exception handling
 * if it happens whilst another interrupt/exception is happening.
 *
 * So, we leave PSP here, and we need to ensure we leave enough space
 * for the MSP.
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

	mrs r1, control

	/* Save the core registers. */
	stmdb r0!, {r1, r4-r11, r14}
	/* Save the new top of stack into the first member of the TCB. */
	str r0, [r2]

	stmdb sp!, {r0, r3}

	/* Mask other interrupts above 0 whilst we figure out which task */
	mov r0, #0xffffffff
	msr basepri, r0

	/* MPU disable */
	bl arm_m4_mpu_disable

	dsb
	isb
	bl kern_task_select

	/* XXX TODO: MPU? Or leave it up to kern_task_select? */
	bl arm_m4_mpu_enable
	dsb
	isb

	/* Zero r0, reset basepri to re-enable interrupts */
	mov r0, #0
	msr basepri, r0

	/* Pop stack registers */
	ldmia sp!, {r0, r3}


	/* The first item in pxCurrentTCB is the task top of stack. */
	ldr r1, [r3]
	ldr r0, [r1]

	/* Pop the core registers. */
	ldmia r0!, {r3-r11, r14}
	msr control, r3

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
