cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Enabling Debug mode" FORCE)
project(test_decouple_in_overflow)

set(APP_HW_TARGET XK-EVK-XU316)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)
include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(APP_INCLUDES src)

set(COMMON_FLAGS -O3
                 -g
                 -lflash
                 -DXUD_CORE_CLOCK=600
                 -fxscope
                 -DXUD_WEAK_API=1
                 -DDEBUG_PRINT_ENABLE=1
				 -DXASSERT_ENABLE_ASSERTIONS_MAIN=1
                 )

foreach(sample_rate IN ITEMS 48000 96000 192000)
    set(APP_COMPILER_FLAGS_${sample_rate} ${COMMON_FLAGS}
                                       -DDEFAULT_FREQ=${sample_rate}
                                       )
endforeach()

XMOS_REGISTER_APP()
