#ifndef	__CORE_LOCK_H__
#define	__CORE_LOCK_H__

#include <core/platform.h>
#include <hw/types.h>

/*
 * Define the lock primitives used on this platform.
 */
typedef struct platform_spinlock {
	irq_save_t	irq;
} platform_spinlock_t;

typedef struct platform_critical_lock {
	irq_save_t	irq;
} platform_critical_lock_t;

static inline void
platform_spinlock_init(platform_spinlock_t *s)
{
	s->irq = 0;
}

/**
 * Acquire a spinlock.
 *
 * Since we're running on a single core right now,
 * we are just disabling interrupts so nothing preempts
 * us.  However later on (if I start running on multi-core
 * cortex-M4 systems?) I'll update this to be a proper
 * spinlock.
 *
 * Spinlocks are in theory acquirable without stopping
 * preemption or interrupts.  I'm implementing them here
 * purely as an experiment but I'm hoping to not use them.
 */
static inline void
platform_spinlock_lock(platform_spinlock_t *s)
{

	s->irq = platform_cpu_irq_disable_save();
}

static inline void
platform_spinlock_unlock(platform_spinlock_t *s)
{

	platform_cpu_irq_enable_restore(s->irq);
}

/**
 * Enter a critical section.
 *
 * A critical section is defined as the current task being
 * locked to the current CPU, and no other interrupts and
 * task switches may occur.
 *
 * Critical sections must not be nested with either themselves
 * or any other locks.
 */
static inline void
platform_critical_enter(platform_critical_lock_t *s)
{
	s->irq = platform_cpu_irq_disable_save();
}

/**
 * Exit a critical section.
 *
 * A critical section is defined as the current task being
 * locked to the current CPU, and no other interrupts and
 * task switches may occur.
 *
 * Critical sections must not be nested with either themselves
 * or any other locks.
 */
static inline void
platform_critical_exit(platform_critical_lock_t *s)
{

	platform_cpu_irq_enable_restore(s->irq);
}

#endif	/* __CORE_LOCK_H__ */
