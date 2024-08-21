TARGET = xk-audio-216-mc.xn
USED_MODULES = lib_xua lib_i2c lib_logging

BUILD_FLAGS = -O3 -g -lflash -DXUD_CORE_CLOCK=600 -fxscope -save-temps -march=xs2a -DUSB_TILE=tile[1] \
			  -DXUA_ADAT_RX_EN=0 -DXUA_ADAT_TX_EN=0 -DXUA_SPDIF_RX_EN=0 -DXUA_SPDIF_TX_EN=0 -DMIDI=0 \
			  -DSIMULATION=1

ifndef pcm_format
$(error pcm_format is not set)
endif

ifndef i2s_role
$(error i2s_role is not set)
endif

ifndef channel_count
$(error channel_count is not set)
endif

ifndef sample_rate
$(error sample_rate is not set)
endif

ifndef word_length
$(error word_length is not set)
endif

ifeq ($(pcm_format),tdm)
	BUILD_FLAGS += -DXUA_PCM_FORMAT=XUA_PCM_FORMAT_TDM
endif
ifeq ($(i2s_role),slave)
	BUILD_FLAGS += -DCODEC_MASTER=1
endif

XCC_FLAGS_simulation_${pcm_format}_${i2s_role}_$(channel_count)in_$(channel_count)out_$(sample_rate)_$(word_length)bit = $(BUILD_FLAGS) \
   -DNUM_USB_CHAN_IN=${channel_count} \
   -DNUM_USB_CHAN_OUT=${channel_count} \
   -DI2S_CHANS_DAC=${channel_count} \
   -DI2S_CHANS_ADC=${channel_count} \
   -DDEFAULT_FREQ=${sample_rate} \
   -DXUA_I2S_N_BITS=${word_length}

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
