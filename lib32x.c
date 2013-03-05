#include "lib32x.h"
#include "../32x.h"

//***************************************************************************
// decompress_slz
// Decompresses data stored in SLZ format
//---------------------------------------------------------------------------
// param out: where to store decompressed data
// param in: pointer to SLZ-compressed data
//***************************************************************************
void decompress_slz(uint8 *out, const uint8 *in) {
   // Retrieve uncompressed size
   uint16 size = in[0] << 8 | in[1];
   in += 2;
   
   // To store the tokens
   uint8 num_tokens = 1;
   uint8 tokens;
   
   // Go through all compressed data until we're done decompressing
   while (size != 0) {
      // Need more tokens?
      num_tokens--;
      if (num_tokens == 0) {
         tokens = *in++;
         num_tokens = 8;
      }
      
      // Compressed string?
      if (tokens & 0x80) {
         // Get distance and length
         uint16 dist = in[0] << 8 | in[1];
         uint8 len = dist & 0x0F;
         dist = dist >> 4;
         in += 2;
         
         // Discount string length from size
         size -= len + 3;
         
         // Copy string using Duff's device
         // Code looks crazy, doesn't it? :)
         uint8 *ptr = out - dist - 3;
         switch (len) {
            case 15: *out++ = *ptr++;
            case 14: *out++ = *ptr++;
            case 13: *out++ = *ptr++;
            case 12: *out++ = *ptr++;
            case 11: *out++ = *ptr++;
            case 10: *out++ = *ptr++;
            case  9: *out++ = *ptr++;
            case  8: *out++ = *ptr++;
            case  7: *out++ = *ptr++;
            case  6: *out++ = *ptr++;
            case  5: *out++ = *ptr++;
            case  4: *out++ = *ptr++;
            case  3: *out++ = *ptr++;
            case  2: *out++ = *ptr++;
            case  1: *out++ = *ptr++;
            case  0: *out++ = *ptr++;
                     *out++ = *ptr++;
                     *out++ = *ptr++;
         }
      }
      
      // Uncompressed byte?
      else {
         // Store byte as-is
         *out++ = *in++;
         size--;
      }
      
      // Go for next token
      tokens += tokens;
   }
}

//***************************************************************************
// decompress_slz24
// Decompresses data stored in SLZ24 format
//---------------------------------------------------------------------------
// param out: where to store decompressed data
// param in: pointer to SLZ24-compressed data
//***************************************************************************
void decompress_slz24(uint8 *out, const uint8 *in) {
   // Retrieve uncompressed size
   uint32 size = in[0] << 16 | in[1] << 8 | in[2];
   in += 3;
   
   // To store the tokens
   uint8 num_tokens = 1;
   uint8 tokens;
   
   // Go through all compressed data until we're done decompressing
   while (size != 0) {
      // Need more tokens?
      num_tokens--;
      if (num_tokens == 0) {
         tokens = *in++;
         num_tokens = 8;
      }
      
      // Compressed string?
      if (tokens & 0x80) {
         // Get distance and length
         uint16 dist = in[0] << 8 | in[1];
         uint8 len = dist & 0x0F;
         dist = dist >> 4;
         in += 2;
         
         // Discount string length from size
         size -= len + 3;
         
         // Copy string using Duff's device
         // Code looks crazy, doesn't it? :)
         uint8 *ptr = out - dist - 3;
         switch (len) {
            case 15: *out++ = *ptr++;
            case 14: *out++ = *ptr++;
            case 13: *out++ = *ptr++;
            case 12: *out++ = *ptr++;
            case 11: *out++ = *ptr++;
            case 10: *out++ = *ptr++;
            case  9: *out++ = *ptr++;
            case  8: *out++ = *ptr++;
            case  7: *out++ = *ptr++;
            case  6: *out++ = *ptr++;
            case  5: *out++ = *ptr++;
            case  4: *out++ = *ptr++;
            case  3: *out++ = *ptr++;
            case  2: *out++ = *ptr++;
            case  1: *out++ = *ptr++;
            case  0: *out++ = *ptr++;
                     *out++ = *ptr++;
                     *out++ = *ptr++;
         }
      }
      
      // Uncompressed byte?
      else {
         // Store byte as-is
         *out++ = *in++;
         size--;
      }
      
      // Go for next token
      tokens += tokens;
   }
}

/* Cache related things */
inline void cache_on() {
	CCR |= 0x01;
}

inline void cache_off() {
	CCR &= 0xFE;
}

inline void cache_purge() {
	CCR |= 0x10;
}

/* PWM related routines */
uint16 lib32x_pwm_num_samples = 441;

#ifdef LIB32X_PWM_ENABLED
int16 __attribute__((aligned(16))) snd_buffer[MAX_NUM_SAMPLES*2*2]; // each sample is 2 channels; we need data for 2 buffers
#endif

void lib32x_pwm_dma1_handler() {
    static int32 which = 0; 

//  while (MARS_SYS_COMM6 == MIXER_LOCK_MSH2) ; // locked by MSH2 

    SH2_DMA_CHCR1; // read TE 
    SH2_DMA_CHCR1 = 0; // clear TE 

    if (which) { 
        // start DMA on first buffer and fill second 
        SH2_DMA_SAR1 = ((uint32)&snd_buffer[0]) | 0x20000000; 
        SH2_DMA_TCR1 = lib32x_pwm_num_samples; // number longs 
        SH2_DMA_CHCR1 = 0x18E5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled 

        pwm_fill_buffer(&snd_buffer[MAX_NUM_SAMPLES * 2]); 
    } else { 
        // start DMA on second buffer and fill first 
        SH2_DMA_SAR1 = ((uint32)&snd_buffer[MAX_NUM_SAMPLES * 2]) | 0x20000000; 
        SH2_DMA_TCR1 = lib32x_pwm_num_samples; // number longs 
        SH2_DMA_CHCR1 = 0x18E5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled 

        pwm_fill_buffer(&snd_buffer[0]); 
    } 

    which ^= 1; // flip audio buffer
}

void lib32x_pwm_init() { 
    uint16 sample, ix; 

    // init DMA 
    SH2_DMA_SAR0 = 0; 
    SH2_DMA_DAR0 = 0; 
    SH2_DMA_TCR0 = 0; 
    SH2_DMA_CHCR0 = 0; 
    SH2_DMA_DRCR0 = 0; 
    SH2_DMA_SAR1 = 0; 
    SH2_DMA_DAR1 = 0x20004034; // storing a long here will set left and right 
    SH2_DMA_TCR1 = 0; 
    SH2_DMA_CHCR1 = 0; 
    SH2_DMA_DRCR1 = 0; 
    SH2_DMA_DMAOR = 1; // enable DMA 

    SH2_DMA_VCR1 = 72; // set exception vector for DMA channel 1 
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0F00; // set DMA INT to priority 15 

    // init the sound hardware 
    MARS_PWM_M_PULSE_W = 1; 
    MARS_PWM_M_PULSE_W = 1; 
    MARS_PWM_M_PULSE_W = 1; 
    
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) {
        MARS_PWM_CYCLE = (((23011361 << 1) / SAMPLE_RATE + 1) >> 1) + 1; // for NTSC clock 
    } else { 
        MARS_PWM_CYCLE = (((22801467 << 1) / SAMPLE_RATE + 1) >> 1) + 1; // for PAL clock 
    }
    
    MARS_PWM_CTRL = MARS_PWM_LEFT_L | MARS_PWM_RIGHT_R | MARS_PWM_DREQ1 | 0x0100; // TM = 1, RTP, RMD = right, LMD = left 

    sample = SAMPLE_MIN; 
    /* ramp up to SAMPLE_CENTER to avoid click in audio (real 32X) */ 
    while (sample < SAMPLE_CENTER) { 
        for (ix=0; ix<(SAMPLE_RATE*2)/(SAMPLE_CENTER - SAMPLE_MIN); ix++) { 
            while (MARS_PWM_M_PULSE_W & 0x8000) ; // wait while full 
            MARS_PWM_M_PULSE_W = sample; 
        } 
        sample++; 
    } 

    // initialize mixer 
    pwm_fill_buffer(&snd_buffer[0]); // fill first buffer 
    lib32x_pwm_dma1_handler(); // start DMA 

    lib32x_set_sh2_sr(2); 
} 

/*****************************************************************************************
 * Converts signed 16 bit samples to unsigned 16 bit used by PWM driver.				 *
 ****************************************************************************************/

inline void lib32x_pwm_convert_signedUnsigned_samples(int16 *bufferIn, uint16 *bufferOut, int samplesToConvert) {
	int i;
	for (i = 0; i < samplesToConvert * 2; i++) {
        int16 s = *bufferIn++ + SAMPLE_CENTER;
        *bufferOut++ = (s < 0) ? SAMPLE_MIN : (s > SAMPLE_MAX) ? SAMPLE_MAX : s;
    }
}

/*****************************************************************************************
 * Initialises the serial communications interface of the SH2.							 *
 ****************************************************************************************/
void lib32x_sci_init(int master) {
	return;

	SH2_INT_IPRB |= 0xE000; // SCI interrupt priority is 14.
	SH2_INT_VCRA = (73 << 0x08) | 74; // RxD error is vector 73, RxD full is 74
	SH2_INT_VCRB = (75 << 0x08) | 76; // TxD empty is vector 75, TxD end is 76
	
	SH2_SCI_SMR = 0x20; // async mode, parity bit on, even parity, 1 stop bit, 8 bit data, clock div 4.
	
	if(master == 1) {
		SH2_SCI_SCR = 0x71; // transmitter on, receiver on, SCK clock out, RxD empty/err int on
	} else {
		SH2_SCI_SCR = 0x72; // transmitter on, receiver on, SCK clock in, RxD empty/err int on	
	}
	
	// To get a constant bitrate regardless of hardware region, we have to do baud rate
	// calculations here.
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) { // NTSC — 23011361Hz main clock
		//uint32 freqVal = (23011361 / (256 * 2^((2*0)-1) * 50000);
		//uint32 BRRVal = (freqVal * (10^6)) - 1;
		
		SH2_SCI_BRR = 42;
    } else { // PAL — 22801467Hz main clock
		SH2_SCI_BRR = 42;
    }

}