#ifndef	__ARM_M4_VTOR_H__
#define	__ARM_M4_VTOR_H__

#include <hw/types.h>

#define	ARM_M4_NUM_EXCEPTIONS		256

struct arm_m4_vtor {
	paddr_t ex[ARM_M4_NUM_EXCEPTIONS];
};

#endif	/* __ARM_M4_VTOR_H__ */
