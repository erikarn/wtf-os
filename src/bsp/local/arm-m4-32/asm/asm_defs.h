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
__attribute__( ( always_inline ) ) static inline void enter_wfi(void)
{
	asm volatile ("wfi");
}

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

__attribute__( ( always_inline ) ) static inline uint32_t get_primask(void)
{
	uint32_t result;

	asm volatile ("MRS %0, primask" : "=r" (result) );
	return(result);
}

__attribute__( ( always_inline ) ) static inline void
set_primask(uint32_t priMask)
{
	asm volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

__attribute__( ( always_inline ) ) static inline uint32_t get_basepri(void)
{
	uint32_t result;

	asm volatile ("MRS %0, basepri" : "=r" (result) );
	return(result);
}

__attribute__( ( always_inline ) ) static inline void
set_basepri(uint32_t basepri)
{
	asm volatile ("MSR basepri, %0" : : "r" (basepri) : "memory");
}


#endif /* __ASM_DEFS_H__ */
