#ifndef	__ASM_DEFS_H__
#define	__ASM_DEFS_H__

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

#endif /* __ASM_DEFS_H__ */
