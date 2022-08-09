cmake_minimum_required(VERSION 3.2)
project(unittests LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)
string(REPLACE "\\" "/" DEVLIBDIR $ENV{DEVLIBDIR})
set(LWIP_DIR ${DEVLIBDIR}/lwip)

#
# STM32H7 LwIP Server Application Build
# https://dev.to/younup/cmake-on-stm32-the-beginning-3766
#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -fdiagnostics-color=always")
include(${LWIP_DIR}/src/Filelists.cmake)

set(unittestsSources 
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipcore6_SRCS}
    ${lwipapi_SRCS}
    ${LWIP_DIR}/src/netif/ethernet.c
    src/Lan8742.cpp
    tests/sys_arch.c
    tests/EthernetInterfaceTest.cpp
    tests/Lan8742Test.cpp
    tests/Main.cpp
)
add_executable(unittests ${unittestsSources})
target_include_directories(unittests PRIVATE
    src/
    tests/
    ${DEVLIBDIR}/etl-master/include
    ${DEVLIBDIR}/lwip/src/include
    ${DEVLIBDIR}/googletest-release-1.11.0/googlemock/include 
    ${DEVLIBDIR}/googletest-release-1.11.0/googletest/include 
)
target_link_directories(unittests PRIVATE
    ${DEVLIBDIR}/googletest-release-1.11.0/build/lib
)
target_link_libraries(unittests pthread gmock gtest)
set_target_properties(unittests PROPERTIES COMPILE_FLAGS "-g3 -fsanitize=address -fno-omit-frame-pointer")
set_target_properties(unittests PROPERTIES LINK_FLAGS "-fsanitize=address -static-libasan")


