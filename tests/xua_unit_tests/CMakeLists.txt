cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

## App name
set( APP_NAME xua_unit_tests )


# Auto-generate schedule and top level config files

if( NOT ${Python3_FOUND} )
  message(FATAL_ERROR "Python3 not found for running . ") 
endif()

#copy conftest.py in the build directory since pytest_collect_file only collects tests from the directory tree where conftest.py is present
configure_file( conftest.py conftest.py COPYONLY ) 

## executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Set unity runner generate script
set( GEN_RUNNER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/generate_unity_runner.py )

# Create directory for runner files
set( RUNNERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/src.runners )
file( MAKE_DIRECTORY ${RUNNERS_DIR} )

file( GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test_*/*.c )

message(STATUS "HERE: ${TEST_SOURCES}")


# For every source file in xua_unit_tests/
foreach(testfile ${TEST_SOURCES})
    message(STATUS "Processing: ${testfile}")

    # Get test name
    #cmake_path( GET testfile STEM TESTNAME )
    get_filename_component(TESTNAME ${testfile} NAME_WLE)

    message(STATUS "Unit test name: ${TESTNAME}")
    project(${TESTNAME})


    
    # Create runner file directory
    file(MAKE_DIRECTORY ${RUNNERS_DIR}/${TESTNAME} )

    #########
    ## Create runner file
    set( RUNNER_FILE ${RUNNERS_DIR}/${TESTNAME}/${TESTNAME}_Runner.c )
    set( GEN_RUNNER_SCRIPT_BYPRODUCTS ${RUNNER_FILE} )

    unset(GEN_RUNNER_SCRIPT_ARGS) 
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --project-root ${UNITY_PATH}/.. )
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --source-file ${testfile} )
    list(APPEND GEN_RUNNER_SCRIPT_ARGS --runner-file ${RUNNER_FILE})

    ## Add command to generate runner file
    add_custom_command(
        OUTPUT ${RUNNER_FILE}
        COMMAND ${Python3_EXECUTABLE} ${GEN_RUNNER_SCRIPT} ${GEN_RUNNER_SCRIPT_ARGS}
        COMMENT "Generating XUA Unit Test Runner" )

    ##########################
    ## Do xcommon cmake build
    ##########################

    set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

    set(APP_HW_TARGET XK-EVK-XU316)
    set(APP_DEPENDENT_MODULES "lib_xua")
    set(APP_PCA_ENABLE ON)
    set(APP_COMPILER_FLAGS ${EXTRA_BUILD_FLAGS} -fcomment-asm
                                                -Wall
                                                -O3
                                                -report
                                                -g
                                                -fxscope
                                                -DUSB_TILE=tile[0]
                                                )

    set(APP_INCLUDES    ${CMAKE_CURRENT_LIST_DIR}/src
                        ${CMAKE_CURRENT_LIST_DIR}/../../../Unity/src)

    XMOS_REGISTER_APP()

endforeach()
