TARGET = xk-audio-216-mc.xn
USED_MODULES = lib_xua lib_i2c lib_logging

BUILD_FLAGS = -O0 -g -lflash -DXUD_SERIES_SUPPORT=4 -DXUD_CORE_CLOCK=600 -fxscope -save-temps -march=xs2a -DUSB_TILE=tile[1]

BUILD_FLAGS_i2s_master_2in_2out_48khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=2 -D NUM_USB_CHAN_OUT=2 -DI2S_CHANS_ADC=2 -DI2S_CHANS_DAC=2 \
  -D DEFAULT_FREQ=48000

BUILD_FLAGS_i2s_slave_2in_2out_48khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=2 -D NUM_USB_CHAN_OUT=2 -DI2S_CHANS_ADC=2 -DI2S_CHANS_DAC=2 \
  -D DEFAULT_FREQ=48000 -DCODEC_MASTER=1

BUILD_FLAGS_i2s_master_2in_2out_192khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=2 -D NUM_USB_CHAN_OUT=2 -D I2S_CHANS_ADC=2 -D I2S_CHANS_DAC=2 \
  -D DEFAULT_FREQ=192000

BUILD_FLAGS_i2s_slave_2in_2out_192khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=2 -D NUM_USB_CHAN_OUT=2 -DI2S_CHANS_ADC=2 -DI2S_CHANS_DAC=2 \
  -D DEFAULT_FREQ=192000 -DCODEC_MASTER=1

BUILD_FLAGS_i2s_master_8in_8out_48khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=48000

BUILD_FLAGS_i2s_slave_8in_8out_48khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=48000 -DCODEC_MASTER=1

BUILD_FLAGS_i2s_master_8in_8out_192khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=192000 \
  -O2 # optimisations to meet timing

BUILD_FLAGS_i2s_slave_8in_8out_192khz = $(BUILD_FLAGS) \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=192000 -DCODEC_MASTER=1 \
  -O2 # optimisations to meet timing

BUILD_FLAGS_tdm_master_8in_8out_48khz = $(BUILD_FLAGS) -D I2S_MODE_TDM=1 \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=48000 \
  -O2 # optimisations to meet timing

BUILD_FLAGS_tdm_slave_8in_8out_48khz = $(BUILD_FLAGS) -D I2S_MODE_TDM=1 \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=8 -D NUM_USB_CHAN_OUT=8 -D I2S_CHANS_ADC=8 -D I2S_CHANS_DAC=8 \
  -D DEFAULT_FREQ=48000 -DCODEC_MASTER=1 \
  -O2 # optimisations to meet timing

BUILD_FLAGS_tdm_master_16in_16out_48khz = $(BUILD_FLAGS) -D I2S_MODE_TDM=1 \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=16 -D NUM_USB_CHAN_OUT=16 -D I2S_CHANS_ADC=16 -D I2S_CHANS_DAC=16 \
  -D DEFAULT_FREQ=48000 \
  -O2 # optimisations to meet timing

BUILD_FLAGS_tdm_slave_16in_16out_48khz = $(BUILD_FLAGS) -D I2S_MODE_TDM=1 \
  -D ADAT_RX=0 -D ADAT_TX=0 -D SPDIF_RX=0 -D SPDIF_TX=0 -D MIDI=0 \
  -D NUM_USB_CHAN_IN=16 -D NUM_USB_CHAN_OUT=16 -D I2S_CHANS_ADC=16 -D I2S_CHANS_DAC=16 \
  -D DEFAULT_FREQ=48000 -DCODEC_MASTER=1 \
  -O2 # optimisations to meet timing

#XCC_FLAGS_hardware_i2s_master_2in_2out_48khz    = -D HARDWARE   $(BUILD_FLAGS_i2s_master_2in_2out_48khz)
#XCC_FLAGS_hardware_i2s_master_2in_2out_192khz   = -D HARDWARE   $(BUILD_FLAGS_i2s_master_2in_2out_192khz)
#XCC_FLAGS_hardware_i2s_master_8in_8out_48khz    = -D HARDWARE   $(BUILD_FLAGS_i2s_master_8in_8out_48khz)
#XCC_FLAGS_hardware_i2s_master_8in_8out_192khz   = -D HARDWARE   $(BUILD_FLAGS_i2s_master_8in_8out_192khz)
#XCC_FLAGS_hardware_tdm_master_8in_8out_48khz    = -D HARDWARE   $(BUILD_FLAGS_tdm_master_8in_8out_48khz)

XCC_FLAGS_simulation_i2s_master_2in_2out_48khz  = -D SIMULATION $(BUILD_FLAGS_i2s_master_2in_2out_48khz)
XCC_FLAGS_simulation_i2s_slave_2in_2out_48khz  = -D SIMULATION $(BUILD_FLAGS_i2s_slave_2in_2out_48khz)

XCC_FLAGS_simulation_i2s_master_2in_2out_192khz = -D SIMULATION $(BUILD_FLAGS_i2s_master_2in_2out_192khz)
XCC_FLAGS_simulation_i2s_slave_2in_2out_192khz = -D SIMULATION $(BUILD_FLAGS_i2s_slave_2in_2out_192khz)

XCC_FLAGS_simulation_i2s_master_8in_8out_48khz  = -D SIMULATION $(BUILD_FLAGS_i2s_master_8in_8out_48khz)
XCC_FLAGS_simulation_i2s_slave_8in_8out_48khz  = -D SIMULATION $(BUILD_FLAGS_i2s_slave_8in_8out_48khz)

XCC_FLAGS_simulation_i2s_master_8in_8out_192khz = -D SIMULATION $(BUILD_FLAGS_i2s_master_8in_8out_192khz)
XCC_FLAGS_simulation_i2s_slave_8in_8out_192khz = -D SIMULATION $(BUILD_FLAGS_i2s_slave_8in_8out_192khz)

XCC_FLAGS_simulation_tdm_master_8in_8out_48khz  = -D SIMULATION $(BUILD_FLAGS_tdm_master_8in_8out_48khz)
XCC_FLAGS_simulation_tdm_slave_8in_8out_48khz  = -D SIMULATION $(BUILD_FLAGS_tdm_slave_8in_8out_48khz)

XCC_FLAGS_simulation_tdm_master_16in_16out_48khz  = -D SIMULATION $(BUILD_FLAGS_tdm_master_16in_16out_48khz)
XCC_FLAGS_simulation_tdm_slave_16in_16out_48khz  = -D SIMULATION $(BUILD_FLAGS_tdm_slave_16in_16out_48khz)

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
