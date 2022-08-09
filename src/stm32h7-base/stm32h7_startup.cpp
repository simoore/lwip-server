#include <algorithm>
#include <cstdint>

/// Function pointer type of constructors of statically allocated objects.
using functionType = void (*)();

// Start of the init_array section. Defined as weak in case there is no init_array section. The function pointers
// in this section point to code that initializers the statically allocated objects in the application.
extern functionType __init_array_start __attribute__((weak));

// End of the init_array section. Defined as weak in case there is no init_array section.
extern functionType __init_array_end __attribute__((weak));

// The location in flash of the initialization values of the .data section. Defined in linker script.
extern char _sidata;

// Start address for the .data section. Defined in linker script.
extern char _sdata;

// End address for the .data section. Defined in linker script.
extern char _edata;

// Start address for the .bss section. Defined in linker script.
extern char _sbss;

// End address for the .bss section. Defined in linker script.
extern char _ebss;

// Initializes registers of the STM32H7 chip. Defined in stm32h7_system.cpp.
void System_Init();

/// The application entry point. It is defined as the entry point in the linker script and the address of this function
/// is always the second address in the vector table at the start of the flash memory.
/// https://allthingsembedded.com/post/2019-01-03-arm-cortex-m-startup-code-for-c-and-c/
extern "C" void Reset_Handler() {
    // Initialize data section. The data section contains initialized global and static variables. It is placed in RAM,
    // since these variables are not constant. The initial values of this section is stored in flash, and must be 
    // copied into RAM.
    const size_t size = static_cast<size_t>(&_edata - &_sdata);
    std::copy(&_sidata, &_sidata + size, &_sdata);

    // Initialize bss section. The bss section contains are uninitalized static and global variables in RAM. We set
    // this section to 0.
    std::fill(&_sbss, &_ebss, char(0));

    // Configures critical registers to a known state.
    System_Init();          

    // Call the constructors of statically allocated objects.
    std::for_each(&__init_array_start, &__init_array_end, [](const functionType pf){ pf(); });

    // Once the system is ready, we can start our application. Compiler doesn't support direct call of main() so we 
    // use assembler.
    asm ("bl main");        

    // Stop application here if main returns.               
    while (true);           
}
