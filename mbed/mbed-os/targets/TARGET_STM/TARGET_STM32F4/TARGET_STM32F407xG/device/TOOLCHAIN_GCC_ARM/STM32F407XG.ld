/* Linker script to configure memory regions. */

/*
 * Source: %mbedmicro%/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_STM32F407VG/TOOLCHAIN_GCC_ARM
 */

MEMORY
{ 
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
  CCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
  RAM (rwx) : ORIGIN = 0x20000188, LENGTH = 96k - 0x188
  SRAM1 (rwx) : ORIGIN = 0x20018000, LENGTH = 16K
  SRAM2 (rwx) : ORIGIN = 0x2001C000, LENGTH = 16K
}

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 * 
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   _estack
 */
ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    __etext = .;
    _sidata = .;

    .data : AT (__etext)
    {
        __data_start__ = .;
        _sdata = .;
        *(vtable)
        *(.data*)

        . = ALIGN(8);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(8);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(8);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(8);
        /* All data end */
        __data_end__ = .;
        _edata = .;

    } > RAM

    .bss :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
        _ebss = .;
    } > RAM

    .heap (COPY):
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > RAM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    _estack = __StackTop;
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    __FillStart = ALIGN(__end__, 8);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

 
    /* Code can explicitly ask for data to be
       placed in these higher RAM banks where
       they will be left uninitialized.
    */
    .AHBSRAM0 (NOLOAD):
    {
        Image$$RW_IRAM2$$Base = . ;
        PROVIDE(__AHB0_block_start = .);
        *(AHBSRAM0)
        Image$$RW_IRAM2$$ZI$$Limit = .;
        PROVIDE(__AHB0_dyn_start = .);
        PROVIDE(__AHB0_end = ORIGIN(SRAM1) + LENGTH(SRAM1));
    } > SRAM1

    .AHBSRAM1 (NOLOAD):
    {
        Image$$RW_IRAM3$$Base = . ;
        PROVIDE(__AHB1_block_start = .);
        *(AHBSRAM1)
        Image$$RW_IRAM3$$ZI$$Limit = .;
        PROVIDE(__AHB1_dyn_start = .);
        PROVIDE(__AHB1_end = ORIGIN(SRAM2) + LENGTH(SRAM2));
    } > SRAM2
}
