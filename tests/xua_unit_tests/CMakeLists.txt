cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

# Auto-generate schedule and top level config files
if( NOT ${Python3_FOUND} )
  message(FATAL_ERROR "Python3 not found for running . ") 
endif()

#copy conftest.py in the build directory since pytest_collect_file only collects tests from the directory tree where conftest.py is present
configure_file( conftest.py conftest.py COPYONLY ) 

## executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Set unity runner generate script
set(GEN_RUNNER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/generate_unity_runner.py)

# Create directory for runner files
set(RUNNERS_DIR ${CMAKE_CURRENT_LIST_DIR}/src.runners )
file(MAKE_DIRECTORY ${RUNNERS_DIR} )

# Find unit test files
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test_*/*.c)

# For every source file in xua_unit_tests/
foreach(TESTFILE ${TEST_SOURCES})
    set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

    # Get test name from C file stem
    cmake_path(GET TESTFILE STEM TESTNAME)
    project(${TESTNAME})
    message(STATUS "Processing unit test: ${TESTNAME}")

    # Create runner file directory
    file(MAKE_DIRECTORY ${RUNNERS_DIR}/${TESTNAME})

    #####################
    ## Create runner file
    #####################    
    set( RUNNER_FILE ${RUNNERS_DIR}/${TESTNAME}/${TESTNAME}_Runner.c )
    set( GEN_RUNNER_SCRIPT_BYPRODUCTS ${RUNNER_FILE})

    unset(GEN_RUNNER_SCRIPT_ARGS) 
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --project-root ${XMOS_SANDBOX_DIR})
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --source-file ${TESTFILE})
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --runner-file ${RUNNER_FILE})

    ## Add command to generate runner file
    add_custom_command(
        OUTPUT ${RUNNER_FILE}
        COMMAND python ${GEN_RUNNER_SCRIPT} ${GEN_RUNNER_SCRIPT_ARGS}
        COMMENT "Generate XUA Unit Test Runner" )

    ##########################
    ## Do xcommon cmake build
    ##########################
    set(APP_HW_TARGET XK-EVK-XU316)
    set(APP_DEPENDENT_MODULES "lib_xua")
    # set(APP_PCA_ENABLE ON)
    set(APP_COMPILER_FLAGS ${EXTRA_BUILD_FLAGS} -fcomment-asm
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
                                                )

    # For HID tests only enable HID
    if(${TESTFILE} MATCHES ".+hid.*")
        list(APPEND APP_COMPILER_FLAGS "-DHID_CONTROLS=1")
    endif()


    # Workaround for xcommon cmake pre-pending CMAKE_CURRENT_LIST_DIR
    string(REPLACE ${CMAKE_CURRENT_LIST_DIR} "" UNIT_TEST_SOURCE_RELATIVE ${TESTFILE})
    string(REPLACE ${CMAKE_CURRENT_LIST_DIR} "" RUNNER_FILE_RELATIVE ${RUNNER_FILE})

    set(APP_C_SRCS      ${RUNNER_FILE_RELATIVE}
                        ${UNIT_TEST_SOURCE_RELATIVE}
                        ../../../Unity/src/unity.c
                        )


    get_filename_component(TEST_FILE_DIR ${TESTFILE} DIRECTORY)
    set(APP_INCLUDES    ${CMAKE_CURRENT_LIST_DIR}/src
                        ${TEST_FILE_DIR}
                        ${XMOS_SANDBOX_DIR}/Unity/src
                        ${XMOS_SANDBOX_DIR}/lib_xud/lib_xud/src/user/class)

    XMOS_REGISTER_APP()

endforeach()
