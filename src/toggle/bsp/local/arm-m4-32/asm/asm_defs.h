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

// WFI - wait for interrupt
// WFE - wait for event
// SEV - send event

/* Register acessors */

__attribute__( ( always_inline ) ) static inline uint32_t get_ipsr(void)
{
	register uint32_t ispr asm ("ipsr");
	return (ispr);
}

__attribute__( ( always_inline ) ) static inline uint32_t get_apsr(void)
{
	register uint32_t aspr asm ("apsr");
	return (aspr);
}


#endif /* __ASM_DEFS_H__ */
