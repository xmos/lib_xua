// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usb_mixer.h"

#define MIXER_UNIT_DISPLAY_VALUE 2
#define MIXER_UNIT_DISPLAY_MIN 3
#define MIXER_UNIT_DISPLAY_MAX 4
#define MIXER_UNIT_DISPLAY_RES 5

// TODO
// res, min, max

int mixer_init(void)
{
    /* Open the connection to the USB mixer */
    if (usb_mixer_connect() == USB_MIXER_FAILURE) 
    {
        return USB_MIXER_FAILURE;
    }

  
    return USB_MIXER_SUCCESS;
}

int mixer_deinit(void) {
  // Close the connection to the USB mixer
  if (usb_mixer_disconnect() == USB_MIXER_FAILURE) {
    return USB_MIXER_FAILURE;
  }

  return USB_MIXER_SUCCESS;
}

int mixer_display(unsigned int mixer_index, unsigned int type) {
  int i = 0; 
  int j = 0;

    int num_inputs = usb_mixer_get_num_inputs(mixer_index);
    int num_outputs = usb_mixer_get_num_outputs(mixer_index);


  printf("\n");
  switch (type) {
    case MIXER_UNIT_DISPLAY_VALUE:
      //mixer_update_all_values(mixer_index);
      printf("  Mixer Values (%d)\n", mixer_index);
      printf("  ----------------\n\n");
      break;
    case MIXER_UNIT_DISPLAY_MIN:
      printf("  Mixer Ranges Min (%d)\n", mixer_index);
      printf("  --------------------\n\n");
      break;
    case MIXER_UNIT_DISPLAY_MAX:
      printf("  Mixer Ranges Max (%d)\n", mixer_index);
      printf("  --------------------\n\n");
      break;
    case MIXER_UNIT_DISPLAY_RES:
      printf("  Mixer Ranges Res (%d)\n", mixer_index);
      printf("  --------------------\n\n");
      break;
    default:
      return USB_MIXER_FAILURE;
      break;
  }

  printf("  \t\t\t");
  printf("Mixer Outputs\n");
  printf("\t\t  ");
  for (i = 0; i < num_outputs; i++) {
    printf("               %d", i+1);
  }
  printf("\n");
  for (i = 0; i < num_inputs; i++) {
    printf("  %-20s", usb_mixer_get_input_name(mixer_index,i));
    for (j = 0; j < num_outputs; j++) {
      switch (type) {
        case MIXER_UNIT_DISPLAY_VALUE:
          {
          double mixNodeVal = usb_mixer_get_value(mixer_index, (i*num_outputs)+j);
          int nodeid = (i*num_outputs)+j;
         
          if (mixNodeVal <= -127.996)// todo shoud be < min 
          {
                printf("\t%3d:[  %s  ]", nodeid,"-inf");
          } 
          else 
          {
                printf("\t%3d:[%08.03f]", nodeid, mixNodeVal);
          }
          }
          break;
        case MIXER_UNIT_DISPLAY_MIN:
        {
          int nodeid = (i*num_outputs)+j;
          printf("\t%3d:[%08.03f]", nodeid, usb_mixer_get_min(mixer_index, (i*num_outputs)+j)) ;
        }
          break;
        case MIXER_UNIT_DISPLAY_MAX:
            {
          int nodeid = (i*num_outputs)+j;
          printf("\t%3d:[%08.03f]", nodeid, usb_mixer_get_max(mixer_index, (i*num_outputs)+j)) ;
            }
          break;
        case MIXER_UNIT_DISPLAY_RES:
          {
          int nodeid = (i*num_outputs)+j;
          printf("\t%3d:[%08.03f]", nodeid, usb_mixer_get_res(mixer_index, (i*num_outputs)+j)) ;
          }
          break;
        default:
          return USB_MIXER_FAILURE;
          break;
      }
    }
    printf("\n");
  }
  printf("\n");
 
  return USB_MIXER_SUCCESS;
}

/* Displays basic mixer information */
int mixer_display_info(void) 
{
    unsigned int i = 0;
    int num_mixers = usb_mixer_get_num_mixers(); 
  
    printf("\n");
    printf("  Mixer Info\n");
    printf("  ----------\n\n");
    printf("  Mixers           : %d\n\n", num_mixers);
 
    for (i = 0; i < num_mixers; i++) 
    {
        int num_inputs = usb_mixer_get_num_inputs(i);
        int num_outputs = usb_mixer_get_num_outputs(i);


        printf("  Mixer %d\n", i);
        printf("  -------\n");
        
        printf("  Inputs           : %d\n"
               "  Outputs          : %d\n\n", num_inputs, num_outputs);

        printf("  Mixer Output Labels:\n");   
        for(int j = 0; j < num_outputs; j++)
        {
            printf("     %d:  %s\n", j,usb_mixer_get_output_name(i,j));
        }

        //printf("\n  Selectable Inputs (%d): \n", usb_mixsel_get_input_count(i));
        //for(int j = 0; j <  usb_mixsel_get_input_count(i); j++)
        //{
          //  printf("     %d:  %s\n", j, usb_mixsel_get_input_string(i,j));
        //}
    }

    printf("\n");

    return USB_MIXER_SUCCESS;
}

void display_available_mixer_sources(int mixIndex)
{
    printf("\n");
    printf("  Available Mixer Sources (%d)\n", mixIndex);
    printf("  -------------------------\n\n");

    for(int j = 0; j <  usb_mixsel_get_input_count(mixIndex); j++)
    {
        printf("     %d:  %s\n", j, usb_mixsel_get_input_string(mixIndex,j));
    }
}

/* Gets the current mixer inputs from the device an displays them */
void display_mixer_sources(int mixerIndex)
{
    printf("\n");
    printf("  Current Mixer Sources (%d)\n", mixerIndex);
    printf("  -------------------------\n\n");

    /* Note, mixSel output cound and mixer input chan count should be the same! */
    printf("    Number of mixer sources: %d\n", usb_mixsel_get_output_count(mixerIndex));
    
    /* Get the current channel number for every mixer input */
    for(int i = 0; i < usb_mixsel_get_output_count(mixerIndex); i++)
    {
        int inputChan = (int)usb_mixsel_get_state(mixerIndex, i);
        char *str = usb_mixer_get_input_name(mixerIndex,inputChan);
        printf("    Mixer input %d: Source chan id: %d (%s)\n", i, inputChan, str);
    }
}

/* set mixer source */
void set_mixer_source(unsigned mixerIndex, unsigned dst, unsigned src)
{
    usb_mixsel_set_state(mixerIndex, dst, src);

    /* String lookup */
    char *str = usb_mixer_get_input_name(mixerIndex, dst);
    int state = usb_mixsel_get_state(mixerIndex, dst);

    printf("\n   Set mixer(%d) input %d to device input %d (%s)\n", mixerIndex, dst, state, str);
}

void display_aud_channel_map()
{
  printf("\n");
  printf("  Audio Output Channel Map\n");
  printf("  ------------------------\n\n");

  for (int i=0;i<usb_get_aud_channel_map_num_outputs();i++) 
  {        
    int x = usb_get_aud_channel_map(i);    
    printf("%d (DEVICE OUT - %s) source is ",i, usb_get_aud_channel_map_name(i));
    
      switch (usb_get_aud_channel_map_type(x)) 
        {      
        case USB_CHAN_OUT:             
          printf(" %d (DAW OUT - %s)\n",x,usb_get_aud_channel_map_name(x));
          break;
        case USB_CHAN_IN:             
          printf("%d (DEVICE IN - %s)\n",x,usb_get_aud_channel_map_name(x));
          break;
        case USB_CHAN_MIXER:             
          printf("%d (%s)\n",x,usb_get_aud_channel_map_name(x));
          break;
        }
  }
}


void display_daw_channel_map()
{
  printf("\n");
  printf("  DAW Output To Host Channel Map\n");
  printf("  ------------------------\n\n");

  for (int i=0;i<usb_get_usb_channel_map_num_outputs();i++) 
  {        
    int x = usb_get_usb_channel_map(i);    
    printf("%d (DAW IN - %s) source is ",i, usb_get_usb_channel_map_name(i + usb_get_aud_channel_map_num_outputs()));
    
      switch (usb_get_usb_channel_map_type(x)) 
        {      
        case USB_CHAN_OUT:             
          printf(" %d (DAW OUT - %s)\n",x,usb_get_usb_channel_map_name(x));
          break;
        case USB_CHAN_IN:             
          printf("%d (DEVICE IN - %s)\n",x,usb_get_usb_channel_map_name(x));
          break;
        case USB_CHAN_MIXER:             
          printf("%d (%s)\n",x,usb_get_usb_channel_map_name(x));
          break;
        }
  }
}

void display_aud_channel_map_sources(void)
{
  printf("\n");
  printf("  Audio Output Channel Map Source List\n");
  printf("  ------------------------------------\n\n");
  for (int i=0;i<usb_get_aud_channel_map_num_inputs();i++) {   
    switch (usb_get_aud_channel_map_type(i)) 
      {      
      case USB_CHAN_OUT:             
        printf("%d (DAW OUT - %s)\n",i,usb_get_aud_channel_map_name(i));
        break;
      case USB_CHAN_IN:             
        printf("%d (DEVICE IN - %s)\n",i,usb_get_aud_channel_map_name(i));
        break;
      case USB_CHAN_MIXER:             
        printf("%d (%s)\n",i,usb_get_aud_channel_map_name(i));
        break;
      }  
    }
}

void display_daw_channel_map_sources(void) 
{
  printf("\n");
  printf("  DAW Output to Host Channel Map Source List\n");
  printf("  ------------------------------------------\n\n");
  for (int i=0;i<usb_get_usb_channel_map_num_inputs();i++) {   
    switch (usb_get_usb_channel_map_type(i)) 
      {      
      case USB_CHAN_OUT:             
        printf("%d (DAW OUT - %s)\n",i,usb_get_usb_channel_map_name(i));
        break;
      case USB_CHAN_IN:             
        printf("%d (DEVICE IN - %s)\n",i,usb_get_usb_channel_map_name(i));
        break;
      case USB_CHAN_MIXER:             
        printf("%d (%s)\n",i,usb_get_usb_channel_map_name(i));
        break;
      }  
    }
}


int usb_audio_request_get(unsigned bRequest, unsigned cs, unsigned cn, unsigned unitId, unsigned char *data)
{    
    char reqStr[] = "Custom";

    if(bRequest == CUR)
    {
      strcpy(reqStr, "CUR");
    }
    else if(bRequest == RANGE)
    {
      strcpy(reqStr, "RANGE");
    }
    else if(bRequest == MEM)
    {
      strcpy(reqStr, "MEM");
    } 
    
    printf("Performing class GET request to Audio Interface:\n\
    	bRequest: 0x%02x (%s)\n\
        wValue: 0x%04x (Control Sel: %d, Channel Number: %d)\n\
        wIndex: 0x%04x (Interface: 0, Entity: %d)\n\
	\n", bRequest, reqStr, (cs<<8)|cn, cs, cn, unitId<<8, unitId);

    return usb_audio_class_get(bRequest, cs, cn, unitId, 64, data);
}

int usb_audio_request_set(unsigned bRequest, unsigned cs, unsigned cn, unsigned unitId, 
  unsigned char *data, int datalength)
{    
    char reqStr[] = "Custom";

    if(bRequest == CUR)
    {
      strcpy(reqStr, "CUR");
    }
    else if(bRequest == RANGE)
    {
      strcpy(reqStr, "RANGE");
    }
    {
      strcpy(reqStr, "MEM");
    } 
    
    printf("Performing class SET request to Audio Interface:\n\
    	bRequest: 0x%02x (%s)\n\
        wValue: 0x%04x (Control Sel: %d, Channel Number: %d)\n\
        wIndex: 0x%04x (Interface: 0, Entity: %d)\n\
	\n", bRequest, reqStr, (cs<<8)|cn, cs, cn, unitId<<8, unitId);

    return usb_audio_class_set(bRequest, cs, cn, unitId, datalength, data);
}


int usb_audio_memreq_get(unsigned unitId, unsigned offset, unsigned char *data)
{
  /* Mem requests dont have CS/CN, just an offset.. */
  return usb_audio_request_get(MEM, (offset>>8), offset&0xff, unitId, data);
}

void print_levels(const char* levelTitle, unsigned char* levels, int levelBytes)
{
    unsigned levelCount = levelBytes/2;
    unsigned short* levelData = (unsigned short*) levels;

    printf("\n  %s Level Data\n"
             "  ----------------------\n\n"
           "%d bytes (%d channels) returned:\n"
           , levelTitle, levelBytes, levelCount);
    
    for(int i = 0; i<levelCount; i++)
    {
       printf("%s %d: 0x%04x\n", levelTitle, i,levelData[i]);
    }
}



void mixer_display_usage(void) {
    fprintf(stderr, "Usage :\n");
    fprintf(stderr, 
            "     --display-info\n"
            "     --display-mixer-nodes               mixer_id\n"
            "     --display-min                       mixer_id\n"
            "     --display-max                       mixer_id\n"
            "     --display-res                       mixer_id\n"
            "     --set-value                         mixer_id, mixer_node, value\n"
            "     --get-value                         mixer_id, mixer_node\n"
            "\n"
            "     --set-mixer-source                  mixer_id dst channel_id, src_channel_id\n"
            "     --display-current-mixer-sources     mixer_id\n"
            "     --display-available-mixer-sources   mixer_id\n"
            "\n"
            "     --set-aud-channel-map               dst_channel_id, src_channel_id\n"
            "     --display-aud-channel-map    \n"
            "     --display-aud-channel-map-sources\n"
            "     --set-daw-channel-map               dst_channel_id, src_channel_id\n"
            "     --display-daw-channel-map    \n"
            "     --display-daw-channel-map-sources\n"
            "\n"
            "     --get-mixer-levels-input            mixer_id\n"
            "     --get-mixer-levels-output           mixer_id\n"
            "     --vendor-audio-request-get   bRequest, ControlSelector, ChannelNumber, UnitId\n"
            "     --vendor-audio-request-set   bRequest, ControlSelector, ChannelNumber, UnitId, Data[0], Data[1],...\n"
            );

}

void usage_error()
{
 fprintf(stderr, "ERROR :: incorrect number of arguments passed.  See --help\n");

}

int main (int argc, char **argv) {

  unsigned int mixer_index = 0;
  unsigned int result = 0;



  if (argc < 2) {
    fprintf(stderr, "ERROR :: No options passed to mixer application\n");
    mixer_display_usage();
    return -1;
  }

  if (strcmp(argv[1], "--help") == 0) {
    mixer_display_usage();
    return 0;
  } 

  if (mixer_init() != USB_MIXER_SUCCESS) {
    fprintf(stderr, "ERROR :: Cannot connect\n");
    return -1;
  }

  if (strcmp(argv[1], "--display-info") == 0) 
  {
    mixer_display_info();
  } 
  else if (strcmp(argv[1], "--display-mixer-nodes") == 0) 
  {
    if (argv[2]) 
    {
      mixer_index = atoi(argv[2]);
    } else {
      fprintf(stderr, "ERROR :: No mixer index supplied\n");
      return -1;
    }
    mixer_display(mixer_index, MIXER_UNIT_DISPLAY_VALUE);
  } else if (strcmp(argv[1], "--display-mixer-nodes") == 0) {
    if (argv[2]) {
      mixer_index = atoi(argv[2]);
    } else {
      fprintf(stderr, "ERROR :: No mixer index supplied\n");
      return -1;
    }
    mixer_display(mixer_index, MIXER_UNIT_DISPLAY_VALUE);
  } else if (strcmp(argv[1], "--display-min") == 0) {
    if (argv[2]) {
      mixer_index = atoi(argv[2]);
    } else {
      fprintf(stderr, "ERROR :: No mixer index supplied\n");
      return -1;
    }
    mixer_display(mixer_index, MIXER_UNIT_DISPLAY_MIN);
  } else if (strcmp(argv[1], "--display-max") == 0) {
    if (argv[2]) {
      mixer_index = atoi(argv[2]);
    } else {
      fprintf(stderr, "ERROR :: No mixer index supplied\n");
      return -1;
    }
    mixer_display(mixer_index, MIXER_UNIT_DISPLAY_MAX);
  } else if (strcmp(argv[1], "--display-res") == 0) {
    if (argv[2]) {
      mixer_index = atoi(argv[2]);
    } else {
      fprintf(stderr, "ERROR :: No mixer index supplied\n");
      return -1;
    }
    mixer_display(mixer_index, MIXER_UNIT_DISPLAY_RES);
  } 
  else if (strcmp(argv[1], "--set-value") == 0) {
    unsigned int mixer_unit = 0;
    double value = 0;
    if (argc < 5) {
      fprintf(stderr, "ERROR :: incorrect number of arguments passed\n");
      return -1;
    }

    mixer_index = atoi(argv[2]);
    mixer_unit = atoi(argv[3]);
    if (strcmp(argv[4],"-inf")==0) 
      value = -128;
    else
      value = atof(argv[4]);

    usb_mixer_set_value(mixer_index, mixer_unit, value);
  } else if (strcmp(argv[1], "--get-value") == 0) {
    unsigned int mixer_unit = 0;
    double result = 0;
    if (argc < 4) {
      fprintf(stderr, "ERROR :: incorrect number of arguments passed\n");
      return -1;
    }

    mixer_index = atoi(argv[2]);
    mixer_unit = atoi(argv[3]);

    result = usb_mixer_get_value(mixer_index, mixer_unit);
    if (result <= -127.996)
      printf("%s\n", "-inf");
    else
      printf("%g\n",result);
  }
  else if (strcmp(argv[1], "--display-current-mixer-sources") == 0) 
  {
    if(argc < 3)
    {
        usage_error();
        return -1;
    }
    display_mixer_sources(atoi(argv[2]));
  }
  else if (strcmp(argv[1], "--display-available-mixer-sources") == 0) 
  {
    if(argc < 3)
    {
        usage_error();
        return -1;
    }
    display_available_mixer_sources(atoi(argv[2]));
  }
  else if(strcmp(argv[1], "--set-mixer-source") == 0)
  {
    if(argc < 5)
    {
        usage_error();
        return -1;
    }
    set_mixer_source(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
  }
    else if (strcmp(argv[1], "--display-aud-channel-map") == 0) 
    {
        /* Display the channel mapping to the devices audio outputs */
        display_aud_channel_map();
    }
    else if (strcmp(argv[1], "--display-aud-channel-map-sources") == 0) 
    {
        display_aud_channel_map_sources();
    }
    else if (strcmp(argv[1], "--display-daw-channel-map") == 0) 
    {
        /* Display the channel mapping to the devices DAW output to host */
        display_daw_channel_map();
    }
    else if (strcmp(argv[1], "--display-daw-channel-map-sources") == 0) 
    {
        display_daw_channel_map_sources();
    }
    else if (strcmp(argv[1], "--set-aud-channel-map") == 0) 
    { 
        unsigned int dst = 0;
        unsigned int src = 0;
        if (argc != 4) 
        {
            usage_error();
            return -1;
        }
        dst = atoi(argv[2]);
        src = atoi(argv[3]);

        usb_set_aud_channel_map(dst, src);
    } 
  else if (strcmp(argv[1], "--set-daw-channel-map") == 0) 
  { 
    unsigned int dst = 0;
    unsigned int src = 0;
    if (argc != 4) 
    {
        usage_error();
        return -1;
    }
    dst = atoi(argv[2]);
    src = atoi(argv[3]);

    usb_set_usb_channel_map(dst, src);
  }
  else if(strcmp(argv[1], "--get-mixer-levels-input") == 0 || 
    strcmp(argv[1],"--get-mixer-levels-output") == 0) 
  {
    unsigned int dst = 0;
    unsigned char levels[64];
    int datalength = 0;
    int offset = 0;

    if (argc < 3) {
      fprintf(stderr, "ERROR :: incorrect number of arguments passed\n");
      return -1;
    }

    if(strcmp(argv[1],"--get-mixer-levels-output") == 0) 
       offset = 1;

    for(int i = 0; i < 64; i++)
      levels[i] = 0;

    dst = atoi(argv[2]);
    
    /* Mem request to mixer with offset of 0 gives input levels */
    datalength = usb_mixer_mem_get(dst, offset, levels);

    if(datalength < 0)
    {    
	fprintf(stderr, "ERROR in control request: %d\n", datalength);
        return -1;
    }

    if(offset)
      print_levels("Mixer Output", levels, datalength);
    else
      print_levels("Mixer Input", levels, datalength);

  }
  else if(strcmp(argv[1], "--vendor-audio-request-get") == 0)
  {
    unsigned int bRequest = 0;
    unsigned int cs = 0;
    unsigned int cn = 0;
    unsigned int unitId = 0;
    int datalength = 0;
    unsigned char data[64];

    if(argc < 6)
    {
      fprintf(stderr, "ERROR :: incorrect number of arguments passed\n");
      return -1;
    }

    for(int i = 0; i < 64; i++)
      data[i] = 0;

    bRequest = atoi(argv[2]);
    cs = atoi(argv[3]);
    cn = atoi(argv[4]);
    unitId = atoi(argv[5]);
   
    /* Do request */ 
    datalength = usb_audio_request_get(bRequest, cs, cn, unitId, data);    
    
    /* Print result */
    if(datalength < 0)
    {    
	fprintf(stderr, "ERROR in control request: %d\n", datalength);
    }
    else 
    {
    	printf("Response (%d bytes):\n", datalength);
	for(int i = 0; i < datalength; i++)
            printf("0x%02x\n" ,data[i]);
    }
  }
  else if(strcmp(argv[1], "--vendor-audio-request-set") == 0)
  {
  
    unsigned int bRequest = 0;
    unsigned int cs = 0;
    unsigned int cn = 0;
    unsigned int unitId = 0;
    unsigned char data[64];

    for(int i=0; i<64; i++)
    {
      data[i] = 0;
    }

    if(argc < 7)
    {
      fprintf(stderr, "ERROR :: incorrect number of arguments passed - no data passed\n");
      return -1;
    }
    bRequest = atoi(argv[2]);
    cs = atoi(argv[3]);
    cn = atoi(argv[4]);
    unitId = atoi(argv[5]);
   
    /* Get data */
    for(int i=0; i < argc-6; i++)
    {
       data[i] = atoi(argv[i+6]);
    }
    
    result = usb_audio_request_set(bRequest, cs, cn, unitId, data, argc-6);    
   
    if(result < 0)
    {
      fprintf(stderr, "ERROR :: Error detected in Set request: %d\n", result);
      return -1;
    }
  }
  else 
  {
    fprintf(stderr, "ERROR :: Invalid option passed to mixer application\n");
    return -1;
  }

  mixer_deinit();

  return result;
}


