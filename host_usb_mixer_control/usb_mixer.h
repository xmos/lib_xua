// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define USB_MIXER_SUCCESS 0
#define USB_MIXER_FAILURE -1

#define USB_MIXERS 1
#define USB_MIXER_INPUTS 18
#define USB_MIXER_OUTPUTS 8
#define USB_MAX_CHANNEL_MAP_SIZE 40
#define USB_MIXER_MAX_NAME_LEN 64

enum usb_chan_type {
  USB_CHAN_OUT=0,
  USB_CHAN_IN=1,
  USB_CHAN_MIXER=2
};

/* A.14 Audio Class-Specific Request Codes */
#define REQUEST_CODE_UNDEFINED      0x00
#define CUR   (1)
#define RANGE (2)
#define MEM   (3)

int usb_mixer_connect();
int usb_mixer_disconnect();

/* MIXER UNIT(s) INTERFACE */

/* Returns total number of mixers in device */
int usb_mixer_get_num_mixers();

/* Returns number of inputs and outputs for a selected mixer */
int usb_mixer_get_layout(unsigned int mixer, unsigned int *inputs, unsigned int *outputs);

/* Returns the name for a selected mixer input */
char *usb_mixer_get_input_name(unsigned int mixer, unsigned int input);

/* Returns the name for a selected mixer output */
char *usb_mixer_get_output_name(unsigned int mixer, unsigned int output);

/* Returns the current value of a selected mixer unit */
double usb_mixer_get_value(unsigned int mixer, unsigned int mixer_unit);

/* Sets the current value for a selected mixer unit */
int usb_mixer_set_value(unsigned int mixer, unsigned int mixer_unit, double val);

/* Returns the range values for a selected mixer unit */
int usb_mixer_get_range(unsigned int mixer, unsigned int mixer_unit, double *min, double *max, double *res);

/* Returns the number of bytes read from a mem request, data is stored in data */
int usb_mixer_mem_get(unsigned int mixer, unsigned offset, unsigned char *data);


/* INPUT / OUTPUT / MIXER MAPPING UNIT INTERFACE */

/* Get the number of selectable inputs */
int usb_mixsel_get_input_count(unsigned int mixer);

/* Get the string of a input */
char *usb_mixsel_get_input_string(unsigned int mixer, unsigned int channel);

int usb_mixsel_get_output_count(unsigned int mixer);

int usb_mixer_get_num_outputs(unsigned int mixer);

int usb_mixer_get_num_inputs(unsigned int mixer);

unsigned char usb_mixsel_get_state(unsigned int mixer, unsigned int channel);

void usb_mixsel_set_state(unsigned int mixer, unsigned int dst, unsigned int src);

int usb_set_usb_channel_map(int channel, int val);


/* Get the current map for a specified input / output / mixer channel */
int usb_get_usb_channel_map(int channel);
int usb_get_aud_channel_map(int channel);

/* Maps an input / output / mixer channel to another input / output / mixer channel */
int usb_set_aud_channel_map(int channel, int val);
int usb_set_usb_channel_map(int channel, int val);

/* Gets the name of a specified channel */
char *usb_get_aud_channel_map_name(int channel);
char *usb_get_usb_channel_map_name(int channel);

/* Get the type of a channel map */
enum usb_chan_type usb_get_aud_channel_map_type(int channel);
enum usb_chan_type usb_get_usb_channel_map_type(int channel);

int usb_get_aud_channel_map_num_outputs();
int usb_get_usb_channel_map_num_outputs();

int usb_get_aud_channel_map_num_inputs();
int usb_get_usb_channel_map_num_inputs();

/* CUSTOM/GENERIC AUDIO CLASS REQUESTS */

int usb_audio_class_get(unsigned char bRequest, unsigned char cs, unsigned char cn, unsigned short unitID, unsigned short wLength, unsigned char *data);

int usb_audio_class_set(unsigned char bRequest, unsigned char cs, unsigned char cn, unsigned short unitID, unsigned short wLength, unsigned char *data);

double usb_mixer_get_res(unsigned int mixer, unsigned int nodeId);

double usb_mixer_get_min(unsigned int mixer, unsigned int nodeId) ;

double usb_mixer_get_max(unsigned int mixer, unsigned int nodeId) ;
