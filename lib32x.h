#ifndef H_32XLIB
#define H_32XLIB

#include "../types.h"

// Comment out the following line to remove PWM sound component (Frees up RAM)
#define LIB32X_PWM_ENABLED 1

// Change the sample rate as you see fit — higher sample rate = more quality, but also
// you need to take into account that larger buffers get more efficient with higher rates.
#define SAMPLE_RATE    22050

// Leave these as is — they're used for conversion from signed -> unsigned samples.
#define SAMPLE_MIN         2
#define SAMPLE_CENTER    517
#define SAMPLE_MAX      1032

// This is the number of samples processed per kerjigger
#define MAX_NUM_SAMPLES 1024
#define MAX_NUM_SAMPLES_BUFFER MAX_NUM_SAMPLES/2

extern uint16 lib32x_pwm_num_samples;
extern int16 snd_buffer[];

void decompress_slz(uint8 *, const uint8 *);
void decompress_slz24(uint8 *, const uint8 *);

inline void cache_on();
inline void cache_off();
inline void cache_purge();

void lib32x_pwm_dma1_handler();
void lib32x_pwm_init();
inline void lib32x_pwm_convert_signedUnsigned_samples(int16 *bufferIn, uint16 *bufferOut, int samplesToConvert);

void lib32x_sci_init(int master);

extern int lib32x_set_sh2_sr(int level);
extern void lib32x_clear_sample_mem(int16 *buffer, int16 numSamples);
extern void lib32x_signal_master_init_ok();
extern void lib32x_slave_wait_master_init_ok();

// You *must* define this function somewhere in your code for the driver to work.
extern void pwm_fill_buffer(int16 *buffer);

#endif