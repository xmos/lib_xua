# xua_unit_tests test application

This example builds the xua_unit_tests application for XCORE AI

## Prerequisites for building

[XMOS Toolchain 15.0.3](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.13 or newer.

## Building for xCORE

Set environment variable for lib_xua path:

    > export XUA_PATH=<path to lib_xua>

cd to lib_xua/tests/xua_unit_tests

Run cmake and build

    > cmake .
    > make

## Run on hardware

Ensure your XCORE AI EXPLORER board is powered up and connected to the XTAG debugger.
Make sure the input.wav file is copied into the build directory

    > pytest -n 1


You should see the tests collected by pytest pass 
