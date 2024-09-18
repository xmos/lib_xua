cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_i2s_loopback)

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

set(APP_HW_TARGET xk-audio-216-mc.xn)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

include(${CMAKE_CURRENT_LIST_DIR}/../../../deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(APP_INCLUDES src)

set(COMMON_FLAGS -O3
                 -g
                 -lflash
                 -DXUD_CORE_CLOCK=600
                 -fxscope
                 -DUSB_TILE=tile[1]
                 -DSIMULATION=1)

file(READ ${CMAKE_CURRENT_LIST_DIR}/../i2s_loopback_params.json params_json)

get_json_list(i2s_role i2s_role_list)
get_json_list(pcm_format pcm_format_list)
get_json_list(channel_count channel_count_list)
get_json_list(word_length word_length_list)
get_json_list(sample_rate sample_rate_list)

foreach(channel_count ${channel_count_list})
    foreach(i2s_role ${i2s_role_list})
        foreach(pcm_format ${pcm_format_list})
            foreach(word_length ${word_length_list})
                foreach(sample_rate ${sample_rate_list})
                    set(EXTRA_FLAGS "")
                    if(i2s_role STREQUAL slave)
                         list(APPEND EXTRA_FLAGS -DCODEC_MASTER=1)
                    endif()
                    if(pcm_format STREQUAL tdm)
                         list(APPEND EXTRA_FLAGS -DXUA_PCM_FORMAT=XUA_PCM_FORMAT_TDM)
                    endif()
                    set(cfg_name simulation_${pcm_format}_${i2s_role}_${channel_count}in_${channel_count}out_${sample_rate}_${word_length}bit)
                    set(APP_COMPILER_FLAGS_${cfg_name} ${COMMON_FLAGS} ${EXTRA_FLAGS}
                                                       -DNUM_USB_CHAN_IN=${channel_count}
                                                       -DNUM_USB_CHAN_OUT=${channel_count}
                                                       -DI2S_CHANS_DAC=${channel_count}
                                                       -DI2S_CHANS_ADC=${channel_count}
                                                       -DDEFAULT_FREQ=${sample_rate}
                                                       -DXUA_I2S_N_BITS=${word_length})
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endforeach()

XMOS_REGISTER_APP()
