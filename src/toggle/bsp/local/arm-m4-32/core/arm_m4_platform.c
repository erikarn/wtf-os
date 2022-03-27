#include <stddef.h>
#include <stdint.h>

#include <core/platform.h>

#include <kern/console/console.h>

/*
 * These variables are set in the linker and startup .s file.
 * They provide the initial platform memory layout that we will
 * need in order to bootstrap our own setup.
 */

extern	uint32_t _estack;
extern	uint32_t _sdata, _edata;
extern	uint32_t _sbss, _ebss;
extern	uint32_t _sidata;

void
platform_cpu_init(void)
{

	console_printf("%s: called\n", __func__);

	console_printf("%s: _estack=0x%x\n", __func__, &_estack);
	console_printf("%s: _data=0x%x -> 0x%x\n", __func__, &_sdata, &_edata);
	console_printf("%s: _bss=0x%x -> 0x%x\n", __func__, &_sbss, &_ebss);
}
