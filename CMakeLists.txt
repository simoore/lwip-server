cmake_minimum_required(VERSION 3.20)
project(stm32h7-lwip-server LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE on)
set(BUILD_SHARED_LIBS off)

# Library directories
set(DEVLIBDIR $ENV{DEVLIBDIR})
set(HAL_DIR ${DEVLIBDIR}/STM32H7xx_HAL_Driver)
set(CMSIS_DIR ${DEVLIBDIR}/CMSIS)
set(LWIP_DIR ${DEVLIBDIR}/lwip)
set(ETL_DIR ${DEVLIBDIR}/etl-master)
set(FREERTOS_DIR ${DEVLIBDIR}/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel)

include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip INTERFACE)
target_sources(lwip INTERFACE 
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipapi_SRCS}
    ${LWIP_DIR}/src/netif/ethernet.c
    ${lwipmqtt_SRCS})
target_include_directories(lwip INTERFACE ${LWIP_DIR}/src/include)
target_compile_options(lwip INTERFACE ${ALL_OPTIONS})
#target_compile_options(lwip INTERFACE LWIP_DEBUG=1)

if (CMAKE_SYSTEM_NAME MATCHES "Generic")

    #
    # STM32H7 LwIP Server Application Build
    # https://dev.to/younup/cmake-on-stm32-the-beginning-3766
    # Toolchain cmake settings in: cmake/arm-none-eabi.cmake
    #

    # Build options
    # https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
    set(MCU_LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/stm32h7-base/stm32h7.ld)
    set(MCU_OPTIONS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
    set(OPT_OPTIONS -fdata-sections -ffunction-sections -fno-exceptions -fno-unwind-tables -nostdlib)
    set(DBG_OPTIONS $<$<CONFIG:Debug>:-Og -g -gdwarf-2>)
    set(CPP_OPTIONS $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -fno-rtti>)
    set(WRN_OPTIONS -Wall -Werror)
    set(ALL_OPTIONS ${MCU_OPTIONS} ${OPT_OPTIONS} ${DBG_OPTIONS} ${CPP_OPTIONS} 
        ${WRN_OPTIONS} -fdiagnostics-color=always)

    add_library(stm32h7hal INTERFACE)
    target_sources(stm32h7hal INTERFACE
        ${HAL_DIR}/Src/stm32h7xx_hal.c
        ${HAL_DIR}/Src/stm32h7xx_hal_cortex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_dma.c
        ${HAL_DIR}/Src/stm32h7xx_hal_dma_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_eth.c
        ${HAL_DIR}/Src/stm32h7xx_hal_eth_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_flash.c
        ${HAL_DIR}/Src/stm32h7xx_hal_flash_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_gpio.c
        ${HAL_DIR}/Src/stm32h7xx_hal_pwr.c
        ${HAL_DIR}/Src/stm32h7xx_hal_pwr_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_rcc.c
        ${HAL_DIR}/Src/stm32h7xx_hal_rcc_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_tim.c
        ${HAL_DIR}/Src/stm32h7xx_hal_tim_ex.c
        ${HAL_DIR}/Src/stm32h7xx_hal_uart.c
        ${HAL_DIR}/Src/stm32h7xx_hal_uart_ex.c)
    target_include_directories(stm32h7hal INTERFACE 
        src/stm32h7-base 
        ${HAL_DIR}/Inc 
        ${HAL_DIR}/Inc/Legacy
        ${CMSIS_DIR}/Device/ST/STM32H7xx/Include
        ${CMSIS_DIR}/Include
        ${ETL_DIR}/include)
    target_compile_definitions(stm32h7hal INTERFACE USE_HAL_DRIVER STM32H743xx)
    target_compile_options(stm32h7hal INTERFACE ${ALL_OPTIONS})

    add_library(freertos INTERFACE)
    target_sources(freertos INTERFACE 
        ${FREERTOS_DIR}/croutine.c
        ${FREERTOS_DIR}/event_groups.c
        ${FREERTOS_DIR}/list.c
        ${FREERTOS_DIR}/queue.c
        ${FREERTOS_DIR}/stream_buffer.c
        ${FREERTOS_DIR}/tasks.c
        ${FREERTOS_DIR}/timers.c
        ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c
        ${FREERTOS_DIR}/portable/MemMang/heap_4.c)
    target_include_directories(freertos INTERFACE 
        ${FREERTOS_DIR}/include
        ${FREERTOS_DIR}/portable/GCC/ARM_CM4F)

    add_library(lwip_baremetal INTERFACE)
    target_include_directories(lwip_baremetal INTERFACE src/app-bare-metal)
    target_link_libraries(lwip_baremetal INTERFACE lwip)

    add_library(lwip_freertos INTERFACE)
    target_sources(lwip_freertos INTERFACE ${LWIP_DIR}/contrib/ports/freertos/sys_arch.c)
    target_include_directories(lwip_freertos INTERFACE src/app-freertos ${LWIP_DIR}/contrib/ports/freertos/include)
    target_link_libraries(lwip_freertos INTERFACE lwip freertos)

    add_library(common INTERFACE)
    target_sources(common INTERFACE 
        src/common/MqttClient.cpp 
        src/common/TcpServer.cpp 
        src/common/utils/DmaRxBuffer.cpp 
        src/common/utils/DmaTxBuffer.cpp)
    target_include_directories(common INTERFACE src/common)
    target_link_libraries(common INTERFACE lwip)

    add_library(stm32h7base INTERFACE)
    target_sources(stm32h7base INTERFACE
        src/stm32h7-base/lan8742.c
        src/stm32h7-base/stm32h7_startup.cpp
        src/stm32h7-base/stm32h7_system.cpp
        src/stm32h7-base/stm32h7_vectors.cpp
        src/stm32h7-base/stm32h7_interrupts.cpp
        src/stm32h7-base/stm32h7_debug_uart.cpp
        src/stm32h7-base/Stm32h7Base.cpp
        src/stm32h7-base/arch/sio.cpp)
    target_include_directories(stm32h7base INTERFACE src/stm32h7base src/base)
    target_link_libraries(stm32h7base INTERFACE lwip)

    set(COMMON_LIBS stm32h7base common stm32h7hal c m nosys stdc++)
    set(COMMON_LINK_OPTS -T${MCU_LINKER_SCRIPT} ${MCU_OPTIONS}
        -specs=nosys.specs
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--print-memory-usage)

    add_executable(lwip-server-baremetal
        src/app-bare-metal/Interrupts.cpp
        src/app-bare-metal/Main.cpp
        src/app-bare-metal/Network.cpp
        src/app-bare-metal/ethernetif.c)
    target_link_libraries(lwip-server-baremetal lwip_baremetal ${COMMON_LIBS})
    set_target_properties(lwip-server-baremetal PROPERTIES OUTPUT_NAME lwip-server-baremetal SUFFIX ".elf")
    target_link_options(lwip-server-baremetal PRIVATE ${COMMON_LINK_OPTS})

    add_executable(lwip-server-rtos
        src/app-freertos/Interrupts.cpp
        src/app-freertos/MainRTOS.cpp
        src/app-freertos/NetworkRTOS.cpp
        src/app-freertos/eth_freertos.cpp
        src/app-freertos/timebase.cpp)
    target_link_libraries(lwip-server-rtos PRIVATE lwip_freertos ${COMMON_LIBS})
    set_target_properties(lwip-server-rtos PROPERTIES OUTPUT_NAME lwip-server-rtos SUFFIX ".elf")
    target_link_options(lwip-server-rtos PRIVATE ${COMMON_LINK_OPTS})

    # Print executable size
    add_custom_command(TARGET lwip-server-baremetal 
        POST_BUILD
        COMMAND arm-none-eabi-size lwip-server-baremetal.elf)
    add_custom_command(TARGET lwip-server-rtos
        POST_BUILD
        COMMAND arm-none-eabi-size lwip-server-rtos.elf)

    # Generate disassembly - this generally takes some time so we don't want to execute it all the time.
    # add_custom_command(TARGET ${PROJECT_NAME}
    #     POST_BUILD
    #     COMMAND arm-none-eabi-objdump -D -S ${PROJECT_NAME}.elf > ${PROJECT_NAME}.list
    # )

else()

    #
    # Unit Tests
    #

    set(UNIT_TESTS_COMPILE_OPTIONS -g3 -fno-omit-frame-pointer)
    set(UNIT_TESTS_CPP_OPTIONS $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -fno-rtti>)

    set(LWIP_INCLUDE_DIRS ${LWIP_DIR}/src/include tests)
    set(LWIP_COMPILER_FLAGS ${UNIT_TESTS_COMPILE_OPTIONS} ${UNIT_TESTS_CPP_OPTIONS})
    set(unittestsSources 
        ${LWIP_DIR}/src/netif/ethernet.c
        src/common/drivers/Lan8742.cpp
        tests/sys_arch.c
        #tests/EthernetInterfaceTest.cpp
        tests/Lan8742Test.cpp
        tests/Main.cpp)
    add_executable(unittests ${unittestsSources})
    target_include_directories(unittests PRIVATE
        src/common
        tests/
        ${DEVLIBDIR}/etl-master/include
        ${DEVLIBDIR}/lwip/src/include
        ${DEVLIBDIR}/googletest-release-1.11.0/googlemock/include 
        ${DEVLIBDIR}/googletest-release-1.11.0/googletest/include)
    target_link_directories(unittests PRIVATE
        ${DEVLIBDIR}/googletest-release-1.11.0/build/lib)
    target_link_libraries(unittests pthread gmock gtest lwip)
    target_compile_options(unittests PRIVATE ${UNIT_TESTS_COMPILE_OPTIONS} ${UNIT_TESTS_CPP_OPTIONS})
    target_link_options(unittests PRIVATE -static-libasan -static-libgcc -static-libstdc++)

endif()

include(${LWIP_DIR}/src/Filelists.cmake)
