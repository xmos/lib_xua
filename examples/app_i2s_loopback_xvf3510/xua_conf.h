
#define NUM_USB_CHAN_OUT 2
#define NUM_USB_CHAN_IN 2
#define I2S_CHANS_DAC 2
#define I2S_CHANS_ADC 2
#define MCLK_441 (512 * 44100)
#define MCLK_48 (512 * 48000)
#define FREQ 48000
#define MIN_FREQ FREQ
#define MAX_FREQ FREQ

#define EXCLUDE_USB_AUDIO_MAIN
#define NUM_PDM_MICS 0
#define XUD_TILE 1
#define AUDIO_IO_TILE 0
#define MIXER 0
#define XUA_USB_EN 1 //switch on transfer samples

#define SPDIF_TX_INDEX 0
#define VENDOR_STR "XMOS"
#define VENDOR_ID 0x20B1
#define PRODUCT_STR_A2 "XUA Example"
#define PRODUCT_STR_A1 "XUA Example"
#define PID_AUDIO_1 1
#define PID_AUDIO_2 2
#define AUDIO_CLASS 2
#define AUDIO_CLASS_FALLBACK 0
#define BCD_DEVICE 0x1234
#define XUA_DFU_EN 0

#define N_BITS_I2S 32
#define AUD_TO_USB_RATIO 1
#define CODEC_MASTER 0


/* TODO */
#define XUA_DFU XUA_DFU_EN