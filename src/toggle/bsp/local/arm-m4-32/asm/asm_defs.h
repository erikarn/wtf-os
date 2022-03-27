#ifndef	__ASM_DEFS_H__
#define	__ASM_DEFS_H__

/* Instruction accessors */

__attribute__( ( always_inline ) ) static inline void arm_dsb(void)
{
  asm volatile ("dsb");
}

__attribute__( ( always_inline ) ) static inline void arm_dmb(void)
{
  asm volatile ("dmb");
}

__attribute__( ( always_inline ) ) static inline void arm_isb(void)
{
  asm volatile ("isb");
}

__attribute__( ( always_inline ) ) static inline void enable_irq(void)
{
	asm volatile ("cpsie i" : : : "memory");
}

__attribute__( ( always_inline ) ) static inline void disable_irq(void)
{
	asm volatile ("cpsid i" : : : "memory");
}

// WFI - wait for interrupt
// WFE - wait for event
// SEV - send event

/* Register acessors */

__attribute__( ( always_inline ) ) static inline uint32_t get_ipsr(void)
{
	uint32_t result;

	asm volatile ("MRS %0, ipsr" : "=r" (result) );
	return (result);
}

__attribute__( ( always_inline ) ) static inline uint32_t get_apsr(void)
{
	uint32_t result;

	asm volatile ("MRS %0, apsr" : "=r" (result) );
	return (result);
}


#endif /* __ASM_DEFS_H__ */
