cmake_minimum_required(VERSION 3.18)

enable_language(CXX C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

project(xmosdfu)

add_executable(xmosdfu ${CMAKE_CURRENT_LIST_DIR}/src/xmosdfu.cpp)

# Discern OS for libusb library location
if ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64"))
    target_link_directories(xmosdfu PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libusb/OSX64")
    set(libusb-1.0_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libusb/OSX64")
    set(LINK_LIBS usb-1.0.0)
elseif ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64"))
    target_link_directories(xmosdfu PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libusb/OSXARM")
    set(libusb-1.0_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libusb/OSXARM")
    set(LINK_LIBS usb-1.0.0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(PkgConfig)
    pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
    set(LINK_LIBS usb-1.0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(nologo WAll WX- O2 EHa)
    target_link_directories(xmosdfu PRIVATE "${CMAKE_CURRENT_LIST_DIR}/libusb/Win64")
    set(libusb-1.0_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/libusb/Win64")
    set(LINK_LIBS libusb-1.0)
endif()

target_link_libraries(xmosdfu PRIVATE ${LINK_LIBS})

target_include_directories(xmosdfu PRIVATE ${libusb-1.0_INCLUDE_DIRS})

