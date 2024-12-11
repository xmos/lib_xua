cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(lib_xua_examples)

add_subdirectory(AN00246_xua_example)
add_subdirectory(AN00247_xua_example_spdif_tx)
add_subdirectory(AN00248_xua_example_pdm_mics)
add_subdirectory(app_xua_example_pdm_mics_vdk)
