cmake_minimum_required(VERSION 3.13)

set(XMOS_TOOLS_PATH $ENV{XMOS_TOOL_PATH}/bin)

#**********************
# Setup XMOS toolchain
#**********************
if(NOT DEFINED ENV{XUA_PATH})
    message(FATAL_ERROR "XUA_PATH environment variable not defined")
    # some more commands
endif()
include("$ENV{XUA_PATH}/cmake_utils/xmos_toolchain.cmake")

#**********************
# Project
#**********************
# Disable in-source build.
#if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
#    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
#endif()


## Define project
project(xua_unit_tests VERSION 0.1.0)

## Enable languages for project
enable_language(CXX XC C ASM)

message(STATUS "CAME HERE")
add_custom_target("runners" ALL)
add_custom_command(
        TARGET runners
        COMMAND python generate_unity_runners.py
        COMMENT "generate unity runners"
)

message(STATUS "CAME HERE 1")
file( GLOB APP_SOURCES src/test*.xc )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
foreach( testsourcefile ${APP_SOURCES} )
    get_filename_component(ITEM_NAME ${testsourcefile} NAME_WE)
    message(STATUS "item_name " ${ITEM_NAME})
    add_executable(${ITEM_NAME})
    set(APP_COMPILER_FLAGS
            "-O2"
            "-g"
            "-Wall"
            "-report"
            "-fxscope"
            "-target=XCORE-AI-EXPLORER"
            "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
            "-DHID_CONTROLS=1"
            "-DUNITY_SUPPORT_64"
            "-DUNITY_INCLUDE_DOUBLE"
            )
     set_source_files_properties(
        "runners/${ITEM_NAME}/${ITEM_NAME}_Runner.c"
        PROPERTIES GENERATED TRUE
        )

    set(APP_SRCS
            ${testsourcefile}
            "runners/${ITEM_NAME}/${ITEM_NAME}_Runner.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../Unity/src/unity.c"
        )
    set(APP_INCLUDES
        "src"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Unity/src"
    )
    set(APP_DEPENDENT_MODULES
            "lib_xua(>=2.0.0)"
            "lib_logging(>=3.0.0)"
            "lib_xassert(>=4.0.0)"
            "lib_xud(>=2.0.0)"
            "lib_spdif(>=4.0.0)"
            "lib_mic_array(>=4.0.0)"
        )

    include("$ENV{XUA_PATH}/cmake_utils/xua.cmake")
    set_target_properties(${ITEM_NAME} PROPERTIES OUTPUT_NAME ${ITEM_NAME}.xe)
    target_compile_options(${ITEM_NAME} PRIVATE ${APP_COMPILER_FLAGS})

    target_include_directories(${ITEM_NAME}
        PRIVATE ${APP_INCLUDES}
        PRIVATE ${XUA_INCLUDES_ALL}
    )

    target_sources(${ITEM_NAME}
            PRIVATE ${APP_SRCS}
            PRIVATE ${XUA_SRCS_ALL}
            )
    add_dependencies(${ITEM_NAME} runners)
    target_link_options(${ITEM_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    ## Set any additional flags only for C++
    set(CMAKE_CXX_FLAGS  "-std=c++11")

endforeach( testsourcefile ${APP_SOURCES} )

message(STATUS ${APP_SOURCES})

message(STATUS "CAME HERE 2")
## Register the application
#XMOS_REGISTER_APP()
