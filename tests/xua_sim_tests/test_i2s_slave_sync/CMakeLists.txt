cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_i2s_slave_sync)

macro(get_json_list json_key out_list)
    string(JSON json_array GET ${params_json} ${json_key})
    string(JSON array_len LENGTH ${json_array})
    math(EXPR last_idx "${array_len} - 1")
    set(out_list "")
    foreach(idx RANGE ${last_idx})
        string(JSON elem GET ${json_array} ${idx})
        list(APPEND ${out_list} ${elem})
    endforeach()
endmacro()

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
                 -DCODEC_MASTER=1)

# Do configs for resynch test
file(READ ${CMAKE_CURRENT_LIST_DIR}/../i2s_slave_sync_params.json params_json)

get_json_list(pcm_format pcm_format_list)
get_json_list(channel_count channel_count_list)
get_json_list(word_length word_length_list)
get_json_list(sample_rate sample_rate_list)

foreach(channel_count ${channel_count_list})
    foreach(pcm_format ${pcm_format_list})
        foreach(word_length ${word_length_list})
            foreach(sample_rate ${sample_rate_list})
                set(EXTRA_FLAGS "")
                if(pcm_format STREQUAL tdm)
                        list(APPEND EXTRA_FLAGS -DXUA_PCM_FORMAT=XUA_PCM_FORMAT_TDM)
                endif()

                set(cfg_name simulation_${pcm_format}_${channel_count}in_${channel_count}out_${sample_rate}_${word_length}bit)
                set(APP_COMPILER_FLAGS_${cfg_name} ${COMMON_FLAGS} ${EXTRA_FLAGS}
                                                    -DNUM_USB_CHAN_IN=${channel_count}
                                                    -DNUM_USB_CHAN_OUT=${channel_count}
                                                    -DI2S_CHANS_DAC=${channel_count}
                                                    -DI2S_CHANS_ADC=${channel_count}
                                                    -DDEFAULT_FREQ=${sample_rate}
                                                    -DXUA_I2S_N_BITS=${word_length})
                message(STATUS ${APP_COMPILER_FLAGS_${cfg_name}})
            endforeach()
        endforeach()
    endforeach()
endforeach()

# Backpressure test
foreach(channel_count ${channel_count_list})
    foreach(word_length ${word_length_list})
        foreach(backpressure_step 1 2 5 10)
            set(cfg_name backpressure_${channel_count}in_${channel_count}out_${word_length}bit_${backpressure_step})
            set(sample_rate 48000)
            math(EXPR backpressure_start "100000000 / ${sample_rate}")
            set(APP_COMPILER_FLAGS_${cfg_name} ${COMMON_FLAGS} ${EXTRA_FLAGS}
                                                -DNUM_USB_CHAN_IN=${channel_count}
                                                -DNUM_USB_CHAN_OUT=${channel_count}
                                                -DI2S_CHANS_DAC=${channel_count}
                                                -DI2S_CHANS_ADC=${channel_count}
                                                -DDEFAULT_FREQ=${sample_rate}
                                                -DXUA_I2S_N_BITS=${word_length}
                                                -DBACKPRESSURE_TICKS_START=${backpressure_start}
                                                -DBACKPRESSURE_TICKS_STEP=${backpressure_step}
                                                -DTOTAL_TEST_FRAMES=64)
            message(STATUS ${APP_COMPILER_FLAGS_${cfg_name}})
        endforeach()
    endforeach()
endforeach()

XMOS_REGISTER_APP()
