include(FetchContent)

###############################################################################
# CMSIS for STM32H7 Devices
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    FetchContent_Declare(
        cmsis_device_h7
        GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis_device_h7.git
        GIT_TAG v1.10.3)
    FetchContent_MakeAvailable(cmsis_device_h7)

    add_library(CmsisH7 INTERFACE)
    target_include_directories(CmsisH7 INTERFACE ${cmsis_device_h7_SOURCE_DIR}/Include)

endif()

###############################################################################
# ARM CMSIS Library
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    FetchContent_Declare(
        cmsis5
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
        GIT_TAG 5.9.0)
    FetchContent_MakeAvailable(cmsis5)

    add_library(CmsisCore INTERFACE)
    target_include_directories(CmsisCore INTERFACE ${cmsis5_SOURCE_DIR}/CMSIS/Core/Include)

endif()

###############################################################################
# STM32H7 HAL Library
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    FetchContent_Declare(
        stm32h7xx_hal_driver
        GIT_REPOSITORY https://github.com/STMicroelectronics/stm32h7xx_hal_driver.git
        GIT_TAG v1.11.1)
    FetchContent_MakeAvailable(stm32h7xx_hal_driver)

    add_library(Stm32h7hal STATIC
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_cortex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_dma.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_dma_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_eth.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_eth_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_flash.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_flash_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_gpio.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_pwr.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_pwr_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_rcc.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_rcc_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_tim.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_tim_ex.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_uart.c
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_hal_uart_ex.c)
    target_include_directories(Stm32h7hal PUBLIC 
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc 
        ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc/Legacy)
    target_link_libraries(Stm32h7hal PUBLIC CmsisH7 CmsisCore STM32H7Options)
    target_compile_definitions(Stm32h7hal PUBLIC USE_HAL_DRIVER STM32H743xx)

endif()

###############################################################################
# ETL
###############################################################################

FetchContent_Declare(
    etl_cpp
    GIT_REPOSITORY https://github.com/ETLCPP/etl.git
    GIT_TAG 20.38.10)
FetchContent_MakeAvailable(etl_cpp)

###############################################################################
# FreeRTOS
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    FetchContent_Declare(
        freertos
        GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
        GIT_TAG V11.0.1)
    FetchContent_MakeAvailable(freertos)

endif()

###############################################################################
# LwIP
###############################################################################

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    FetchContent_Declare(
        lwip
        GIT_REPOSITORY https://github.com/lwip-tcpip/lwip.git
        GIT_TAG STABLE-2_2_0_RELEASE)
    FetchContent_Populate(lwip)

    set(LWIP_DIR ${lwip_SOURCE_DIR})
    include(${lwip_SOURCE_DIR}/src/Filelists.cmake)

    add_library(LwIP INTERFACE)
    target_sources(LwIP INTERFACE 
        ${lwipcore_SRCS}
        ${lwipcore4_SRCS}
        ${lwipapi_SRCS}
        ${LWIP_DIR}/src/netif/ethernet.c
        ${lwipmqtt_SRCS})
    target_include_directories(LwIP INTERFACE ${LWIP_DIR}/src/include)
    #target_compile_options(lwip INTERFACE LWIP_DEBUG=1)

    add_library(LwIPFreeRTOS STATIC)
    target_include_directories(LwIPFreeRTOS PUBLIC)
    target_link_libraries(LwIPFreeRTOS PUBLIC LwIP freertos_kernel)

endif()

###############################################################################
# Google Test
###############################################################################

if(WIN32 OR LINUX)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

endif()