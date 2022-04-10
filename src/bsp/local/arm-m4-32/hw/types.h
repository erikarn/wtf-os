#ifndef	__ARM_M4_HW_TYPES_H__
#define	__ARM_M4_HW_TYPES_H__

#include <stdint.h>

/* Note: for M4, there's no virtual address types */

typedef uint32_t paddr_t;
typedef uint32_t stack_addr_t;
typedef uint32_t kern_code_exec_addr_t;
typedef uint32_t kern_code_stack_addr_t;
typedef uint32_t irq_save_t;
typedef uint32_t syscall_arg_t;
typedef uint32_t syscall_retval_t;

#endif	/* __ARM_M4_HW_TYPES_H__ */
