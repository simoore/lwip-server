#
# STM32H7 LwIP Server Application Build
# https://dev.to/younup/cmake-on-stm32-the-beginning-3766
# Toolchain cmake settings in: cmake/arm-none-eabi.cmake
#

cmake_minimum_required(VERSION 3.2)
project(stm32h7-lwip-server LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)

# Library directories
string(REPLACE "\\" "/" DEVLIBDIR $ENV{DEVLIBDIR})
set(HAL_DIR ${DEVLIBDIR}/STM32H7xx_HAL_Driver)
set(CMSIS_DIR ${DEVLIBDIR}/CMSIS)
set(LWIP_DIR ${DEVLIBDIR}/lwip)
# include(${LWIP_DIR}/src/Filelists.cmake)

# Build options
# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
set(MCU_LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/stm32h7-base/stm32h7.ld)
set(MCU_OPTIONS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(OPT_OPTIONS -fdata-sections -ffunction-sections -fno-exceptions -fno-unwind-tables -nostdlib)
set(DBG_OPTIONS $<$<CONFIG:Debug>:-Og -g -gdwarf-2>)
set(CPP_OPTIONS $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -fno-rtti>)
set(WRN_OPTIONS -Wall -Werror)

set(STM32H7_SOURCES
    src/Main.cpp
    src/stm32h7-base/stm32h7_startup.cpp
    src/stm32h7-base/stm32h7_system.cpp
    src/stm32h7-base/stm32h7_vectors.cpp
    src/stm32h7-base/stm32h7_interrupts.cpp
    src/stm32h7-base/stm32h7_debug_uart.cpp
    src/stm32h7-base/Stm32h7Base.cpp
    src/utils/DmaRxBuffer.cpp
    src/utils/DmaTxBuffer.cpp
    ${HAL_DIR}/Src/stm32h7xx_hal.c
    ${HAL_DIR}/Src/stm32h7xx_hal_cortex.c
    ${HAL_DIR}/Src/stm32h7xx_hal_dma.c
    ${HAL_DIR}/Src/stm32h7xx_hal_dma_ex.c
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
    ${HAL_DIR}/Src/stm32h7xx_hal_uart_ex.c
)

add_executable(${CMAKE_PROJECT_NAME} ${STM32H7_SOURCES})
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".elf")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32H743xx)
target_link_libraries(${CMAKE_PROJECT_NAME} c m nosys stdc++)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    src/
    src/base/
    src/stm32h7-base/
    src/utils/
    ${HAL_DIR}/Inc
    ${HAL_DIR}/Inc/Legacy
    ${CMSIS_DIR}/Device/ST/STM32H7xx/Include
    ${CMSIS_DIR}/Include
)

target_compile_options(${PROJECT_NAME} PRIVATE ${MCU_OPTIONS} ${OPT_OPTIONS} ${DBG_OPTIONS} ${CPP_OPTIONS} 
    ${WRN_OPTIONS} -fdiagnostics-color=always
)

target_link_options(${PROJECT_NAME} PRIVATE -T${MCU_LINKER_SCRIPT} ${MCU_OPTIONS}
    -specs=nosys.specs
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--print-memory-usage
)

# Print executable size
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND arm-none-eabi-size ${PROJECT_NAME}.elf
)

# Create hex file
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND arm-none-eabi-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)

# Generate disassembly
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND arm-none-eabi-objdump -D -S ${PROJECT_NAME}.elf > ${PROJECT_NAME}.list
)