

#include <xs1.h>
#include <print.h>
#include "mixer.h"
#include "devicedefines.h"

#ifdef MIXER

#define FAST_MIXER 1
#warning USING FAST MIXER

#ifdef OUT_VOLUME_IN_MIXER
static unsigned int multOut[NUM_USB_CHAN_OUT + 1];
#endif
#ifdef IN_VOLUME_IN_MIXER
unsigned int multIn[NUM_USB_CHAN_IN + 1];
#endif

#if defined (LEVEL_METER_LEDS) || defined (LEVEL_METER_HOST) 
static unsigned abs(int x)
{
#if 0
    if (x < 0)
        return x*-1;
    return x;
#else
    int const mask = x >> sizeof(int) * 8 - 1;
    return (x + mask) ^ mask;
#endif
}
#endif

int samples[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT + 1]; /* One larger for an "off" channel for mixer sources" */
int savedsamples2[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT];

int samples_to_host_map[NUM_USB_CHAN_IN];

int samples_to_device_map[NUM_USB_CHAN_OUT];

#if MAX_MIX_COUNT > 0
int mix_mult[MAX_MIX_COUNT][MIX_INPUTS];
#ifndef FAST_MIXER  
int mix_map[MAX_MIX_COUNT][MIX_INPUTS];
#endif
#endif

/* Arrays for level data */
int samples_to_host_inputs[NUM_USB_CHAN_IN];             /* Audio transmitted to host i.e. dev inputs */
#ifdef LEVEL_METER_LEDS
int samples_to_host_inputs_buff[NUM_USB_CHAN_IN];        /* Audio transmitted to host i.e. dev inputs */
#endif
static int samples_to_host_streams[NUM_USB_CHAN_OUT];    /* Audio stream to host from host */
static int samples_to_host_outputs[NUM_USB_CHAN_OUT];    /* Device outputs */

#if 0
#pragma xta command "add exclusion mixer1_rate_change"
#pragma xta command "analyse path mixer1_req mixer1_req"
#pragma xta command "set required - 10400 ns"             /* 96kHz */
#endif

#if 0
#pragma xta command "add exclusion mixer2_rate_change"
#pragma xta command "analyse path mixer2_req mixer2_req"
#pragma xta command "set required - 10400 ns"             /* 96kHz */
#endif

#if defined (LEVEL_METER_LEDS) || defined (LEVEL_METER_HOST) 
static inline void ComputeMixerLevel(int sample, int i)
{
    int x;
    int y;
    unsigned ptr;


    x = abs(sample);

    /* y = samples_to_host_outputs[i] */
    asm("ldaw %0, dp[samples_to_host_outputs]":"=r"(ptr):); /* Might want to hoist this */          
    asm("ldw %0, %1[%2]":"=r"(y):"r"(ptr),"r"(i));          

    if(x > y)
    {
        /* samples_to_host_outputs[i] = x; */
        asm("stw %0, %1[%2]"::"r"(y),"r"(ptr),"r"(i));
    }
}
#endif
#ifdef FAST_MIXER
void setPtr(int src, int dst, int mix);
int doMix0(const int samples[], int mult[]);
int doMix1(const int samples[], int mult[]);
int doMix2(const int samples[], int mult[]);
int doMix3(const int samples[], int mult[]);
int doMix4(const int samples[], int mult[]);
int doMix5(const int samples[], int mult[]);
int doMix6(const int samples[], int mult[]);
int doMix7(const int samples[], int mult[]);
int doMix8(const int samples[], int mult[]);
#else
/* DO NOT inline, causes 10.4.2 tools to add extra loads in loop */
/* At 18 x 12dB we could get 64 x bigger */
#pragma unsafe arrays
int doMix(const int samples[], const int ptr[], int mult[])
{
    int h=0; 
    int l=0; 

/* By breaking up the loop we keep things in the encoding for ldw (0-11) */
#pragma loop unroll
    for (int i=0; i<MIX_INPUTS; i++) 
    {
        {h,l} = macs(samples[ptr[i]], mult[i], h, l);
    }
    
#if 1
    /* Perform saturation */
    l = sext(h, 25);

    if(l != h)
    {
        //if(h < 0)
        if(h>>32)  
            h = (0x80000000>>7);
        else
            h = (0x7fffff00>>7);
    }
#endif
    return h<<7;
}
#endif

#pragma unsafe arrays
void giveSamplesToHost(chanend c, const int samples[], const int ptr[], const unsigned multIn[])
{
#if defined(IN_VOLUME_IN_MIXER) && defined(IN_VOLUME_AFTER_MIX)
    int mult;
    int h;
    unsigned l;
#endif

#pragma loop unroll
  for (int i=0;i<NUM_USB_CHAN_IN;i++) 
  {
        int sample;
        sample = samples[ptr[i]];

#if defined(IN_VOLUME_IN_MIXER) && defined(IN_VOLUME_AFTER_MIX)
#warning IN Vols in mixer, AFTER mix & map
        //asm("ldw %0, %1[%2]":"=r"(mult):"r"(multIn),"r"(i));          
        mult = multIn[i];
        {h, l} = macs(mult, sample, 0, 0);
    
        //h <<= 3 done on other side */
 
        outuint(c, h);
#else
        outuint(c,sample);
#endif

  }
}

#pragma unsafe arrays
static void getSamplesFromHost(chanend c, const int samples[], int base)
{
#pragma loop unroll
    for (int i=0;i<NUM_USB_CHAN_OUT;i++)
    {
        int sample, x;
#if defined(OUT_VOLUME_IN_MIXER) && !defined(OUT_VOLUME_AFTER_MIX)
        int mult;
        int h;
        unsigned l;
#endif
        /* Receive sample from decouple */ 
        sample = inuint(c);

#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
        /* Compute peak level data */
        x = abs(sample);
        if(x > samples_to_host_streams[i])
            samples_to_host_streams[i] = x;
#endif

#if defined(OUT_VOLUME_IN_MIXER) && !defined(OUT_VOLUME_AFTER_MIX)
#warning OUT Vols in mixer, BEFORE mix & map
        asm("ldw %0, %1[%2]":"=r"(mult):"r"(multOut),"r"(i));          
        {h, l} = macs(mult, sample, 0, 0);
        h<<=3;
        asm("stw %0, %1[%2]"::"r"(h),"r"(samples),"r"(base+i));
#else
        asm("stw %0, %1[%2]"::"r"(sample),"r"(samples),"r"(base+i));
#endif
  }
}

#pragma unsafe arrays
void giveSamplesToDevice(chanend c, const int samples[],const int ptr[], const unsigned multOut[])
{
#pragma loop unroll
    for (int i=0;i<NUM_USB_CHAN_OUT;i++) 
    {
        int sample,x;
#if defined(OUT_VOLUME_IN_MIXER) && defined(OUT_VOLUME_AFTER_MIX)
        int mult;
        int h;
        unsigned l;
#endif

        sample = samples[ptr[i]];

#if defined(OUT_VOLUME_IN_MIXER) && defined(OUT_VOLUME_AFTER_MIX)
#warning OUT Vols in mixer, AFTER mix & map
        mult = multOut[i];
        //asm("ldw %0, %1[%2]":"=r"(mult):"r"(multOut),"r"(i));          
        {h, l} = macs(mult, sample, 0, 0);
        //h <<= 3 done in audio thread
        outuint(c, h);
#else
        outuint(c, sample);
#endif
    }
}

#pragma unsafe arrays
void getSamplesFromDevice(chanend c, const int samples[], int base)
{
#if defined(IN_VOLUME_IN_MIXER) && !defined(IN_VOLUME_AFTER_MIX)
    int mult;
    int h;
    unsigned l;
#endif

#pragma loop unroll
    for (int i=0;i<NUM_USB_CHAN_IN;i++) 
    {
        int sample;
        int x;
        sample = inuint(c);

#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
        /* Compute peak level data */
        x = abs(sample);
        if(x > samples_to_host_inputs[i])
            samples_to_host_inputs[i] = x;
#endif

#if defined(IN_VOLUME_IN_MIXER) && !defined(IN_VOLUME_AFTER_MIX)
        asm("ldw %0, %1[%2]":"=r"(mult):"r"(multIn),"r"(i));          
        {h, l} = macs(mult, sample, 0, 0);
        h <<=3;
        asm("stw %0, %1[%2]"::"r"(h),"r"(samples),"r"(base+i));
#else
        asm("stw %0, %1[%2]"::"r"(sample),"r"(samples),"r"(base+i));
#endif
  }
}

int mixer1_mix2_flag = (DEFAULT_FREQ > 96000);

#pragma unsafe arrays
void mixer1(chanend c_host, chanend c_mix_ctl, chanend c_mixer2)
{
    int mixed;
    unsigned cmd;

    while (1) 
    {

#pragma xta endpoint "mixer1_req"
        inuint(c_mixer2);

        /* Request data from decouple thread */
        outuint(c_host, 0);
        
        /* Between request to decouple and respose ~ 400nS latency for interrupt to fire */ 
         select 
        {
            case inuint_byref(c_mix_ctl, cmd):
            {
                int mix, index, val;
                switch (cmd) 
                {
#if MAX_MIX_COUNT > 0
                    case SET_SAMPLES_TO_HOST_MAP:
                        index = inuint(c_mix_ctl);
                        val = inuint(c_mix_ctl);
                        inct(c_mix_ctl);
                        
                        asm("stw %0, %1[%2]"::"r"(val),"r"(samples_to_host_map),"r"(index));
                        break;

                    case SET_SAMPLES_TO_DEVICE_MAP:
                        index = inuint(c_mix_ctl);
                        val = inuint(c_mix_ctl);
                        inct(c_mix_ctl);
                        asm("stw %0, %1[%2]"::"r"(val),"r"(samples_to_device_map),"r"(index));
                        break;
        
                    case SET_MIX_MULT:
                        mix = inuint(c_mix_ctl);
                        index = inuint(c_mix_ctl);
                        val = inuint(c_mix_ctl);
                        inct(c_mix_ctl);
                        
                        asm("stw %0, %1[%2]"::"r"(val),"r"(mix_mult[mix]),"r"(index));
                        break;
        
                    case SET_MIX_MAP:
                        mix = inuint(c_mix_ctl);
                        index = inuint(c_mix_ctl); /* mixer input */
                        val = inuint(c_mix_ctl);   /* source */
                        inct(c_mix_ctl);
#ifdef FAST_MIXER
                        setPtr(index, val, mix);
#else
                        asm("stw %0, %1[%2]"::"r"(val),"r"(mix_map[mix]),"r"(index));
#endif
                        break;
#endif /* if MAX_MIX_COUNT > 0 */

#ifdef IN_VOLUME_IN_MIXER
                    case SET_MIX_IN_VOL:
                        index = inuint(c_mix_ctl);
                        val = inuint(c_mix_ctl);
                        inct(c_mix_ctl);

                        asm("stw %0, %1[%2]"::"r"(val),"r"(multIn),"r"(index));
                        break;
#endif
#ifdef OUT_VOLUME_IN_MIXER
                    case SET_MIX_OUT_VOL:
                        index = inuint(c_mix_ctl);
                        val = inuint(c_mix_ctl);
                        inct(c_mix_ctl);

                        asm("stw %0, %1[%2]"::"r"(val),"r"(multOut),"r"(index));
                        break;
#endif

                    case GET_STREAM_LEVELS:
                        index = inuint(c_mix_ctl);
                        chkct(c_mix_ctl, XS1_CT_END);
                        outuint(c_mix_ctl, samples_to_host_streams[index]);
                        outct(c_mix_ctl, XS1_CT_END);
                        samples_to_host_streams[index] = 0;
                        break;
                    
                    case GET_INPUT_LEVELS:
                        index = inuint(c_mix_ctl);
                        chkct(c_mix_ctl, XS1_CT_END);
#ifdef LEVEL_METER_LEDS    
                        /* Level LEDS process reseting samples_to_host_inputs 
                         * Other side makes sure we don't miss a peak */ 
                        asm("ldw %0, %1[%2]":"=r"(val):"r"(samples_to_host_inputs_buff),"r"(index));
                        asm("stw %0, %1[%2]"::"r"(0),"r"(samples_to_host_inputs_buff),"r"(index));
#else
                        /* We dont have a level LEDs process, so reset ourselves */
                        asm("ldw %0, %1[%2]":"=r"(val):"r"(samples_to_host_inputs),"r"(index));
                        asm("stw %0, %1[%2]"::"r"(0),"r"(samples_to_host_inputs),"r"(index));
#endif
                        outuint(c_mix_ctl, val);
                        outct(c_mix_ctl, XS1_CT_END);
                        break;

#if MAX_MIX_COUNT > 0                   
                    case GET_OUTPUT_LEVELS:
                        index = inuint(c_mix_ctl);
                        chkct(c_mix_ctl, XS1_CT_END);
                        asm("ldw %0, %1[%2]":"=r"(val):"r"(samples_to_host_outputs),"r"(index));
                        asm("stw %0, %1[%2]"::"r"(mix),"r"(samples_to_host_outputs),"r"(index));
                        outuint(c_mix_ctl, val);
                        outct(c_mix_ctl, XS1_CT_END);
                        break;
#endif
                }
                break;
            }
            default:
            /* Select default */
                break;
        }


        /* Get response from decouple */
        if(testct(c_host))
        {
            int sampFreq;
#pragma xta endpoint "mixer1_rate_change"
            inct(c_host);
            sampFreq = inuint(c_host);

            mixer1_mix2_flag = sampFreq > 96000;

#pragma loop unroll 
            for (int i=0;i<MAX_MIX_COUNT;i++) 
            {
                asm("stw %0, %1[%2]"::"r"(0),"r"(samples),"r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + i));
            }

            /* Inform mixer 2 about freq change */
            outct(c_mixer2, XS1_CT_END);
            outuint(c_mixer2, sampFreq);

            /* Wait for handshake and pass on */
            chkct(c_mixer2, XS1_CT_END);
            outct(c_host, XS1_CT_END);
        }
        else 
        {
            inuint(c_host);
#if MAX_MIX_COUNT > 0
            outuint(c_mixer2, 0);
            giveSamplesToHost(c_host, samples, samples_to_host_map, multIn);

            outuint(c_mixer2, 0);
            inuint(c_mixer2);
            getSamplesFromHost(c_host, samples, 0);
            outuint(c_mixer2, 0);
            inuint(c_mixer2);
#ifdef FAST_MIXER
            mixed = doMix0(samples, mix_mult[0]);
#else
            mixed = doMix(samples,mix_map[0],mix_mult[0]);
#endif
            asm("stw %0, %1[%2]"::
          "r"(mixed),"r"(samples),
          "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 0));

#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
            ComputeMixerLevel(mixed, 0);
#endif

#if (MAX_FREQ > 96000) 
            if (!mixer1_mix2_flag)
#endif 
            {

#if MAX_MIX_COUNT > 2
#ifdef FAST_MIXER
                mixed = doMix2(samples, mix_mult[2]);
#else
                mixed = doMix(samples,mix_map[2],mix_mult[2]);
#endif     
                asm("stw %0, %1[%2]"::
                    "r"(mixed),"r"(samples),
                    "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 2));

#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
                ComputeMixerLevel(mixed, 2);
#endif
#endif

#if MAX_MIX_COUNT > 4
#ifdef FAST_MIXER
                mixed = doMix4(samples, mix_mult[4]);
#else
                mixed = doMix(samples,mix_map[4],mix_mult[4]);
#endif
                asm("stw %0, %1[%2]"::
                    "r"(mixed),"r"(samples),
                    "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 4));
            
#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
                ComputeMixerLevel(mixed, 4);
#endif
#endif

#if MAX_MIX_COUNT > 6
#ifdef FAST_MIXER
                mixed = doMix6(samples, mix_mult[6]);
#else
                mixed = doMix(samples,mix_map[6],mix_mult[6]);
#endif
                    asm("stw %0, %1[%2]"::
                        "r"(mixed),"r"(samples),
                        "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 6));      
            
#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
                ComputeMixerLevel(mixed, 6);
#endif
#endif
            }
#else       /* IF MAX_MIX_COUNT > 0 */
            /* No mixes, this thread runs on its own doing just volume */
            giveSamplesToDevice(c_mixer2, samples, samples_to_device_map, multOut);
            getSamplesFromDevice(c_mixer2, samples, NUM_USB_CHAN_OUT);
            giveSamplesToHost(c_host, samples, samples_to_host_map, multIn);
            getSamplesFromHost(c_host, samples, 0);
#endif
        }
    }
}

int mixer2_mix2_flag = (DEFAULT_FREQ > 96000);

#pragma unsafe arrays
void mixer2(chanend c_mixer1, chanend c_audio)
{
  int mixed;

  while (1) {
    outuint(c_mixer1, 0);
#pragma xta endpoint "mixer2_req"
    inuint(c_audio);
    if(testct(c_mixer1)) 
    {
        int sampFreq;
#pragma xta endpoint "mixer2_rate_change"
        inct(c_mixer1);
        sampFreq = inuint(c_mixer1);


        mixer2_mix2_flag = sampFreq > 96000;
        
        for (int i=0;i<MAX_MIX_COUNT;i++) 
        {
          asm("stw %0, %1[%2]":: "r"(0),"r"(samples),"r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + i));
        }

        /* Inform audio thread about freq change */
        outct(c_audio, XS1_CT_END);
        outuint(c_audio, sampFreq);

        /* Wait for handshake and pass on */
        chkct(c_audio, XS1_CT_END);
        outct(c_mixer1, XS1_CT_END);
      }
    else {
      (void) inuint(c_mixer1);
      giveSamplesToDevice(c_audio, samples, samples_to_device_map, multOut);
      inuint(c_mixer1);
      outuint(c_mixer1, 0);
      getSamplesFromDevice(c_audio, samples, NUM_USB_CHAN_OUT);
      inuint(c_mixer1);
      outuint(c_mixer1, 0);

#if MAX_MIX_COUNT > 1
#ifdef FAST_MIXER
      mixed = doMix1(samples, mix_mult[1]);
#else
      mixed = doMix(samples,mix_map[1],mix_mult[1]);
#endif     
     
      asm("stw %0, %1[%2]"::
          "r"(mixed),"r"(samples),
          "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 1));

#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
        ComputeMixerLevel(mixed, 1);
#endif
#endif



#if (MAX_FREQ > 96000) 
      if (!mixer2_mix2_flag) 
#endif
      {
#if MAX_MIX_COUNT > 3
#ifdef FAST_MIXER
      mixed = doMix3(samples, mix_mult[3]);
#else
    mixed = doMix(samples,mix_map[3],mix_mult[3]);
#endif

        asm("stw %0, %1[%2]"::
            "r"(mixed),"r"(samples),
            "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 3));
#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
            ComputeMixerLevel(mixed, 3);
#endif
#endif

#if MAX_MIX_COUNT > 5
#ifdef FAST_MIXER
     mixed = doMix5(samples, mix_mult[5]);
#else
    mixed = doMix(samples,mix_map[5],mix_mult[5]);
#endif
        asm("stw %0, %1[%2]"::
            "r"(mixed),"r"(samples),
            "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 5));
#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
            ComputeMixerLevel(mixed, 5);
#endif
#endif
        
#if MAX_MIX_COUNT > 7
#ifdef FAST_MIXER
     mixed = doMix7(samples, mix_mult[7]);
#else
    mixed = doMix(samples,mix_map[7],mix_mult[7]);
#endif
        asm("stw %0, %1[%2]"::
            "r"(mixed),"r"(samples),
            "r"(NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + 7));      
#if defined (LEVEL_METER_HOST) || defined(LEVEL_METER_LEDS)
            ComputeMixerLevel(mixed, 7);
#endif
#endif
      }

    }
  }
}

void mixer(chanend c_mix_in, chanend c_mix_out, chanend c_mix_ctl)
{
    chan c;
    for (int i=0;i<NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT;i++) 
    {
        samples[i] = 0;
    }

    {
        int num_mixes = DEFAULT_FREQ > 96000 ? 2 : MAX_MIX_COUNT;
        for (int i=0;i<NUM_USB_CHAN_OUT;i++) 
        {
            asm("stw %0, %1[%2]"::
            "r"(i),
            "r"(samples_to_device_map),
            "r"(i));
        }
    }

#ifdef OUT_VOLUME_IN_MIXER
    for (int i=0;i<NUM_USB_CHAN_OUT;i++) 
    {
        multOut[i] = MAX_VOL;
    }
#endif

#ifdef IN_VOLUME_IN_MIXER
    for (int i=0;i<NUM_USB_CHAN_IN;i++) 
    {
        multIn[i] = MAX_VOL;
    }
#endif

    for (int i=0;i<NUM_USB_CHAN_IN;i++) 
    {
        samples_to_host_map[i] = NUM_USB_CHAN_OUT + i;
    }

#if MAX_MIX_COUNT> 0
    for (int i=0;i<MAX_MIX_COUNT;i++)
        for (int j=0;j<MIX_INPUTS;j++) 
        {
#ifndef FAST_MIXER
            mix_map[i][j] = j < 16 ? j : j + 2;
#endif
            mix_mult[i][j] = i==j ? MAX_VOL >> 3 : 0;
        }
#endif

    par 
    {
#if (MAX_MIX_COUNT > 0) 
        mixer1(c_mix_in, c_mix_ctl, c);
        mixer2(c, c_mix_out);
#else
        mixer1(c_mix_in, c_mix_ctl, c_mix_out);
#endif
    }
}

#endif
