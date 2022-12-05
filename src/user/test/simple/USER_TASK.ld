
/* Entry Point */
ENTRY(_start)


/* Specify the memory areas */
MEMORY
{
FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 128K
}

/* Define output sections */
SECTIONS
{

  .header :
  {
    _header_start = .;

    /*
     * Header for the userland program loader to figure
     * out what's going on.
     */
    LONG(LOADADDR(.text));
    LONG(  SIZEOF(.text));

    LONG(LOADADDR(.got));
    LONG(  SIZEOF(.got));

    LONG(LOADADDR(.bss));
    LONG(  SIZEOF(.bss));

    LONG(LOADADDR(.data));
    LONG(  SIZEOF(.data));

    LONG(LOADADDR(.rodata));
    LONG(  SIZEOF(.rodata));

    /* heap size */
    LONG(1024);
    /* stack size */
    LONG(1024);

    . = ALIGN(4);
    _header_end = .;
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  /*
   * note: deleted .preinit, .init, .fini for now as
   * I don't have any shared library support yet, and
   * even if I did I don't know how these would map to
   * PIC / r9 relative stuff.  Will revisit those once
   * dynamic loaded/run userland is actually bootstrapped.
   */

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
  } >FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end__ = .;
  } >FLASH

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


