cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(xua_unit_tests)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(APP_HW_TARGET XK-EVK-XU316)
set(APP_DEPENDENT_MODULES "lib_xua"
                          "lib_unity")

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS -fcomment-asm
                       -Wall
                       -O2
                       -report
                       -g
                       -fxscope
                       -DUSB_TILE=tile[0]
                       -DUNITY_SUPPORT_64
                       -DUNITY_INCLUDE_DOUBLE
                       -DXUD_CORE_CLOCK=600
                       -DXUD_SERIES_SUPPORT=4
                       -DXASSERT_ENABLE_ASSERTIONS=0)

file(GLOB_RECURSE tests RELATIVE ${CMAKE_CURRENT_LIST_DIR} src/test_*.c)

foreach(test_file ${tests})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(SOURCE_FILES_${test_name} ${test_file})

    # For HID tests only enable HID
    if(${test_file} MATCHES ".+hid.*")
        set(APP_COMPILER_FLAGS_${test_name} ${APP_COMPILER_FLAGS} -DHID_CONTROLS=1)
    endif()
endforeach()

# HID report descriptor header is shared between tests, so select the correct part
list(APPEND APP_COMPILER_FLAGS_test_hid -DUNIT_TEST_SIMPLE_REPORT)
list(APPEND APP_COMPILER_FLAGS_test_hid_multi_report -DUNIT_TEST_MULTI_REPORT)

# Enable auto gen of test runners
set(LIB_UNITY_AUTO_TEST_RUNNER ON)

# Disable unity features that we dont use
set(LIB_UNITY_USE_FIXTURE OFF)
set(LIB_UNITY_USE_MEMORY OFF)

XMOS_REGISTER_APP()
