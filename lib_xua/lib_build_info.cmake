set(LIB_NAME lib_xua)
set(LIB_VERSION 3.5.1)
set(LIB_INCLUDES api
                 src/core
                 src/core/audiohub
                 src/core/buffer/ep
                 src/core/endpoint0
                 src/dfu
                 src/core/buffer/decouple
                 src/core/clocking
                 src/core/mixer
                 src/core/pdm_mics
                 src/core/ports
                 src/core/support
                 src/core/user
                 src/core/user/audiostream
                 src/core/user/audiohw
                 src/core/user/hid
                 src/core/user/hostactive
                 src/hid
                 src/midi)
set(LIB_OPTIONAL_HEADERS xua_conf.h static_hid_report.h)
set(LIB_DEPENDENT_MODULES "lib_locks"
                          "lib_logging"
                          "lib_mic_array(feature/xcommon_cmake)"
                          "lib_spdif"
                          "lib_xassert"
                          "lib_xud"
                          "lib_adat"
                          "lib_sw_pll(feature/non_ack_reg_Write)")

set(LIB_COMPILER_FLAGS -O3 -DREF_CLK_FREQ=100 -fasm-linenum -fcomment-asm)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND LIB_COMPILER_FLAGS -DXASSERT_ENABLE_ASSERTIONS=1
                                   -DXASSERT_ENABLE_DEBUG=1
                                   -DXASSERT_ENBALE_LINE_NUMBERS=1)
else()
    list(APPEND LIB_COMPILER_FLAGS -DXASSERT_ENABLE_ASSERTIONS=0
                                   -DXASSERT_ENABLE_DEBUG=0
                                   -DXASSERT_ENABLE_LINE_NUMBERS=0)
endif()

set(LIB_COMPILER_FLAGS_xua_endpoint0.c ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_xua_ep0_uacreqs.xc ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_dbcalc.xc ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_audioports.c ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_audioports.xc ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_dfu.xc ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_flash_interface.c ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)
set(LIB_COMPILER_FLAGS_flashlib_user.c ${LIB_COMPILER_FLAGS} -Os -mno-dual-issue)

XMOS_REGISTER_MODULE()
