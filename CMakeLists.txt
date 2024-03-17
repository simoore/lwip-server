cmake_minimum_required(VERSION 3.26)
project(lwip-server LANGUAGES C CXX)

set(BUILD_SHARED_LIBS off)

if (CMAKE_SYSTEM_NAME MATCHES "Generic")

    ###########################################################################
    # STM32H7 LwIP Server Application Build
    # https://dev.to/younup/cmake-on-stm32-the-beginning-3766
    # Toolchain cmake settings in: cmake/arm-none-eabi.cmake
    ###########################################################################

    ###########################################################################
    # Build options
    # https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
    ###########################################################################

    set(MCU_LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/stm32h7/stm32h7.ld)
    set(MCU_OPTIONS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
    set(OPT_OPTIONS -fdata-sections -ffunction-sections -fno-exceptions -fno-unwind-tables -nostdlib)
    set(DBG_OPTIONS -Og -g -gdwarf-2)
    set(CPP_OPTIONS -std=c++23 -fno-rtti)
    set(WRN_OPTIONS -Wall -Werror)
    set(ALL_OPTIONS ${MCU_OPTIONS} ${OPT_OPTIONS} ${WRN_OPTIONS} -fdiagnostics-color=always)

    add_library(STM32H7Options INTERFACE)
    target_compile_options(STM32H7Options INTERFACE
        ${ALL_OPTIONS}
        $<$<CONFIG:Debug>:${DBG_OPTIONS}>
        $<$<COMPILE_LANGUAGE:CXX>:${CPP_OPTIONS}>)
    target_link_libraries(STM32H7Options INTERFACE nosys c m stdc++)
    target_link_options(STM32H7Options INTERFACE -T${MCU_LINKER_SCRIPT} ${MCU_OPTIONS} -specs=nosys.specs)
    target_include_directories(STM32H7Options INTERFACE include)

    ###########################################################################
    # FreeRTOS Configuration
    # We define this target for the freertos cmake build system to find FreeRTOSConfig.h 
    ###########################################################################

    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config INTERFACE include)
    target_link_libraries(freertos_config INTERFACE STM32H7Options)
    set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
    set(FREERTOS_PORT "GCC_ARM_CM7" CACHE STRING "" FORCE)

    ###########################################################################
    # Project Application and Libraries
    ###########################################################################

    add_library(common INTERFACE)
    target_sources(common INTERFACE 
        src/network/MqttClient.cpp 
        src/network/TcpServer.cpp 
        src/utils/DmaRxBuffer.cpp)
    target_link_libraries(common INTERFACE LwIPFreeRTOS Stm32h7hal etl)

    add_library(stm32h7base INTERFACE)
    target_sources(stm32h7base INTERFACE
        src/stm32h7/Base.cpp
        src/stm32h7/DebugUart.cpp
        src/stm32h7/Ethernetif.cpp
        src/stm32h7/Interrupts.cpp
        src/stm32h7/Startup.cpp
        src/stm32h7/Syscalls.cpp
        src/stm32h7/System.cpp
        src/stm32h7/Timebase.cpp
        src/stm32h7/Vectors.cpp)
    target_link_libraries(stm32h7base INTERFACE LwIPFreeRTOS Stm32h7hal)

    set(COMMON_LIBS stm32h7base common)
    set(COMMON_LINK_OPTS -Wl,-Map=${PROJECT_NAME}.map,--cref -Wl,--print-memory-usage -Wl,--no-warn-rwx-segment)

    add_executable(lwip-server-rtos
        src/app-freertos/MainRTOS.cpp
        )
    target_link_libraries(lwip-server-rtos PRIVATE ${COMMON_LIBS})
    set_target_properties(lwip-server-rtos PROPERTIES OUTPUT_NAME lwip-server-rtos SUFFIX ".elf")
    target_link_options(lwip-server-rtos PRIVATE ${COMMON_LINK_OPTS})

    # Print executable size
    add_custom_command(TARGET lwip-server-rtos
        POST_BUILD
        COMMAND arm-none-eabi-size lwip-server-rtos.elf)

    # Generate disassembly - this generally takes some time so we don't want to execute it all the time.
    # add_custom_command(TARGET lwip-server-rtos
    #     POST_BUILD
    #     COMMAND arm-none-eabi-objdump -D -S lwip-server-rtos.elf > lwip-server-rtos.list
    # )

else()

    ###########################################################################
    # Unit Tests
    ###########################################################################

    enable_testing()

    add_executable(unittests
        tests/Lan8742Test.cpp
        tests/Main.cpp)
    target_include_directories(unittests PRIVATE include)
    target_link_libraries(unittests gmock gtest etl)
    target_compile_options(unittests PRIVATE 
        -g3 -fno-omit-frame-pointer
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++23 -fno-rtti>)

    add_test(NAME UnitTests COMMAND unittests)

endif()

include(cmake/third-party.cmake)
