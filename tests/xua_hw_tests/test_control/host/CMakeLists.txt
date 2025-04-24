cmake_minimum_required(VERSION 3.21)

if(USE_CLANG)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "C++ compiler" FORCE)
    set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "C compiler" FORCE)
endif()

enable_language(CXX C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

project(host_vendor_specifc_control)

set(APP_NAME host_control_test)

add_executable(${APP_NAME})

target_sources(${APP_NAME}
                PRIVATE
                src/control_test.cpp
                src/device_access_usb.c)

# Add options for different compilers
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_compile_options( ${APP_NAME}
    PRIVATE
        -WX
)
else()
target_compile_options( ${APP_NAME}
    PRIVATE
        -Werror
        -g
)
endif()

# Discern OS for libusb library location
set(xua_libusb_path "${CMAKE_CURRENT_LIST_DIR}/../../../../lib_xua/host/xmosdfu/libusb")
message(STATUS "xua_libusb_path = ${xua_libusb_path}")
if ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64"))
    target_link_directories(${APP_NAME} PRIVATE "${xua_libusb_path}/OSX64")
    set(libusb-1.0_INCLUDE_DIRS "${xua_libusb_path}/OSX64")
    set(LINK_LIBS usb-1.0.0)
elseif ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64"))
    target_link_directories(${APP_NAME} PRIVATE "${xua_libusb_path}/OSXARM")
    set(libusb-1.0_INCLUDE_DIRS "${xua_libusb_path}/OSXARM")
    set(LINK_LIBS usb-1.0.0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(PkgConfig)
    pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
    set(LINK_LIBS usb-1.0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_directories(${APP_NAME} PRIVATE "${xua_libusb_path}/Win32")
    set(libusb-1.0_INCLUDE_DIRS "${xua_libusb_path}/Win32")
    set(LINK_LIBS libusb-1.0)
endif()

target_include_directories(${APP_NAME}
    PUBLIC
        src
        ${libusb-1.0_INCLUDE_DIRS}
)

target_link_libraries(${APP_NAME}
    PUBLIC
        ${LINK_LIBS}
)


