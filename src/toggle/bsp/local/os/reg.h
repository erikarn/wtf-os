#ifndef	__REG_H__
#define	__REG_H__

#include <asm/asm_defs.h>

/*
 * XXX TODO: convert to platform specific PA/VA macros based
 * on whether in userland or kernel and whether running in
 * a VA setup or a microcontroller PA setup.
 *
 * For now this assumes a microcontroller PA setup, with no VA.
 */

static inline uint32_t
os_reg_read32(uintptr_t base, uint32_t reg)
{
  uint32_t val;

  arm_dsb();
  val = *(uint32_t *)(uintptr_t)(base + reg);
  arm_dsb();
  return val;
}

static inline void
os_reg_write32(uintptr_t base, uint32_t reg, uint32_t val)
{

  *(uint32_t *)(uintptr_t)(base + reg) = val;
  arm_dsb();
}

#endif	/* __REG_H__ */
