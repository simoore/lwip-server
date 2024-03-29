/*
 * This sets the start of address of the ELF executable to the address of Reset_Handler. For bare-metal applications, 
 * I don't think this statement really does anything as I beleive the processor will execute address stored in the 
 * second entry of the vector table which is set to Reset_Handler.
 */ 
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x0;      /* required amount of heap  */

_Min_Stack_Size = 0x800; /* required amount of stack */


/*
 * Reference: 
 * Reference Manual STM32H742, STM32H743/753 and STM32H750 Value line advanced Arm®-based 32-bit MCUs
 * RM0433 Rev 6, April 2019
 * Section 2.3.2: Memory map and register boundary addresses
 *
 * RAM_D1 is known as AXI SRAM in the documentation.
 * RAM_D2 is the combination of SRAM1, SRAM2, and SRAM3 in domain 2.
 * RAM_D3 is the SRAM4 in domain 3.
 */
MEMORY
{
    ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
    DTCMRAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 128K
    RAM_D1 (xrw)    : ORIGIN = 0x24000000, LENGTH = 512K
    RAM_D2 (xrw)    : ORIGIN = 0x30000000, LENGTH = 288K
    RAM_D3 (xrw)    : ORIGIN = 0x38000000, LENGTH = 64K
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K
}


SECTIONS
{
    /* The startup code goes first into FLASH */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Startup code */
        . = ALIGN(4);
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

    /* Fast executing code goes in ITCMEM */
    itcmemFlashAddr = LOADADDR(.itcmem);
    .itcmem :
    {
        . = ALIGN(4);
        startItcmem = .; 
        *(.itcmem)         
        *(.itcmem*)         
        . = ALIGN(4);
        endItcmem = .;
    } >ITCMRAM AT> FLASH

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

    /* These sections are used to store the construction and initialization of static objects and arrays.
     */

    .preinit_array     :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    /* 
     * The init_array section is where the compiler places function pointers to functions that initialize statically
     * allocated objects. This includes the C++ constructors of global and static objects.
     *
     * PROVIDE  Indicates that the linker only define the symbol if it is not defined in any compiled object.
     * HIDDEN   For ELF binaries, the symbol won't be exported.
     */
    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    /*
     * This section includes destructors to statically allocated objects.
     */
    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* used by the startup to initialize data */
    _sidata = LOADADDR(.data);

    /* Initialized data sections goes into RAM, load LMA copy after code */
    .data : 
    {
        . = ALIGN(4);
        _sdata = .;        /* create a global symbol at data start */
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */

        . = ALIGN(4);
        _edata = .;        /* define a global symbol at data end */
    } >RAM_D1 AT> FLASH

    
        /* Uninitialized data section */
        . = ALIGN(4);
    .bss :
    {
        /* This is used by the startup in order to initialize the .bss section */
        _sbss = .;         /* define a global symbol at bss start */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = _ebss;
    } >RAM_D1

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM_D1 
    
    .lwip_sec (NOLOAD) : {
        . = ABSOLUTE(0x30000000);
        *(.RxDecripSection) 
        
        . = ABSOLUTE(0x30000400);
        *(.TxDecripSection)
        
        . = ABSOLUTE(0x30000800);
        *(.Rx_PoolSection) 
    } >RAM_D2 AT> FLASH

    /*
     * Section in SRAM1 that is used for buffers for DMA1. It is 32 bytes aligned because cache coherency operations
     * operate on memory blocks that are 32 byte aligned.
     */
    .dmamem1 :
    {
        . = ALIGN(32);
        _sdmamem1 = .;       /* Symbol at the start of this memory section. */
        *(.dmamem1)
        *(.dmamem1*)
        . = ALIGN(32);
        _edmamem1 = .;       /* Symbol at the end of this memory section. */
    } >RAM_D2

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}


