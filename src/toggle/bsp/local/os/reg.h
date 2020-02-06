#ifndef	__REG_H__
#define	__REG_H__

/*
 * XXX TODO: convert to platform specific PA/VA macros based
 * on whether in userland or kernel and whether running in
 * a VA setup or a microcontroller PA setup.
 *
 * For now this assumes a microcontroller PA setup, with no VA.
 */

static inline uint32_t
os_reg_read32(void *base, uint32_t reg)
{
  uint32_t val;

  __dsb();
  val = *(uint32_t *)(uintptr_t)(base + reg);
  return val;
}

static inline void
os_reg_write32(void *base, uint32_t reg, uint32_t val)
{
  *(uint32_t *)(uintptr_t)(base + reg) = val;
  __dsb();
}

#endif	/* __REG_H__ */
