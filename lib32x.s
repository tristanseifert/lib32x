/*****************************************************************************************
 * int lib32x_set_sh2_sr(int level);													 *
 * On entry: r4 = new irq level															 *
 * On exit:  r0 = old irq level															 *
 *																						 *
 * 0x00F0 is the mask for int level														 *
 ****************************************************************************************/
        .align  4
        .global _lib32x_set_sh2_sr
        
_lib32x_set_sh2_sr:
        stc     sr, r1
        mov     #0x0F, r0
        shll2   r0
        shll2   r0
        and     r0, r1                   							/* just the irq mask */
        shlr2   r1
        shlr2   r1
        not     r0, r0
        stc     sr, r2
        and     r0, r2
        shll2   r4
        shll2   r4
        or      r4, r2
        ldc     r2, sr
        rts
        mov     r1, r0

/*****************************************************************************************
 * PWM DMA interrupt
 ****************************************************************************************/	
		.align 4
		.global lib32x_pwm_slave_dma1_int
		
lib32x_pwm_slave_dma1_int:
        /* save registers */
        sts.l   pr, @-r15 
        mov.l   r0, @-r15 
        mov.l   r1, @-r15 
        mov.l   r2, @-r15 
        mov.l   r3, @-r15 
        mov.l   r4, @-r15 
        mov.l   r5, @-r15 
        mov.l   r6, @-r15 
        mov.l   r7, @-r15 

		/* Debug code to count DMA PWM interrupts */
		mov.l	slave_dma_handler_count, r0
		mov.w	@r0, r1
		add		#1, r1
		mov.w	r1, @r0

        mov.l   slave_dma1_handler, r0 
        jsr     @r0 
        nop 

        /* restore registers */
        mov.l   @r15+, r7 
        mov.l   @r15+, r6 
        mov.l   @r15+, r5 
        mov.l   @r15+, r4 
        mov.l   @r15+, r3 
        mov.l   @r15+, r2 
        mov.l   @r15+, r1 
        mov.l   @r15+, r0 
        lds.l   @r15+, pr 
        rte 
        nop 

        .align  2 
        
slave_dma1_handler: 
        .long   _slave_pwm_dma1_handler 

slave_dma_handler_count:
		.long	0x20004024

/*****************************************************************************************
 * extern void lib32x_clear_sample_mem(int16 *buffer, int16 numSamples);				 *
 * buffer = r4																			 *
 * numSamples = r5																		 *
 ****************************************************************************************/	
        .align  4
        .global _lib32x_clear_sample_mem
        		
_lib32x_clear_sample_mem:
        mov     #0, r0											/* Clear r0. */
        
_loopClearSampleMem:
		mov.l	r0, @r4											/* Clear a 2 words of sample data */
																/* Clears both L/R data in 1 instr */
		dt		r5												/* decrement counter, test it */
		bf/s	_loopClearSampleMem								/* if not 0, loop */
		add		#4, r4											/* Increase pointer.*/
		
        rts														/* return to caller */
        nop														/* Pad for prefetch */

/*****************************************************************************************
 * extern void lib32x_signal_master_init_ok();											 *
 ****************************************************************************************/	
		.align 4
		.global _lib32x_signal_master_init_ok
		
_lib32x_signal_master_init_ok:
		mov.l	lib32x_main_init_complete_comms, r0
		mov.l	lib32x_main_init_complete_value, r1
		mov.l	r1, @r0												/* Write to mem loc */
		rts															/* Return to caller. */
		nop

/*****************************************************************************************
 * extern void lib32x_slave_wait_master_init_ok();										 *
 ****************************************************************************************/	
		.align 4
		.global _lib32x_slave_wait_master_init_ok

_lib32x_slave_wait_master_init_ok:
		mov.l	lib32x_main_init_complete_comms, r0
		mov.l	lib32x_main_init_complete_value, r1
	
0:
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		
		mov.l	@r0, r2												/* Read value of init reg */
		cmp/eq	r1, r2												/* Is it equal to r1? */
		bf		0b													/* If not, keep checking. */
		nop	
		rts															/* Return to caller. */
		nop	
		
		.align 2
		
/* this location is initialised to "M_OK" once master is initialised. */
lib32x_main_init_complete_comms:
		.long	0x2603FF00											/* note how we are avoiding
																	 * the cache. */

lib32x_main_init_complete_value:
		.ascii	"M_OK"

/*****************************************************************************************
 * extern void lib32x_vbi_handler_flag();												 *
 * Updates a flag in memory to indicate VBlank has occurred. 							 *
 * Destriys r0-r2; this is important if ran from int routines!							 *
 ****************************************************************************************/	
		.align 4
		.global _lib32x_vbi_handler_flag
		
_lib32x_vbi_handler_flag:
		mov		#0x0001, r2											/* Bits to set in flag for VBL */
		mov.l	lib32x_vbi_handler_flag_mem, r0						/* Memory location to r0. */
		mov.w	@r0, r1												/* Read memory location. */
		or		r2, r1												/* Set LSB */
		mov.w	r1, @r0												/* Restore to memory. */

		rts
		nop

		.align 2
lib32x_vbi_handler_flag_mem:
		.long 	0x20004020

/*****************************************************************************************
 * extern void lib32x_waitForVBlank();													 *
 * Waits until the above routine updates a flag in memory.								 *
 ****************************************************************************************/	
		.align 4
		.global _lib32x_waitForVBlank
		
_lib32x_waitForVBlank:
		mov.l	lib32x_vbi_flag, r0									/* Memory location to r0. */
		mov		#0x0001, r2											/* Bits to set in flag for VBL */
		
1:
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		
		mov.w	@r0, r1												/* Read flag. */
		and		r2, r1												/* Get only the low bit. */
		cmp/eq	r2, r1												/* Are they equal? */
		bf		1b													/* If not, keep checking. */
		nop
		
		mov		#0x0000, r1											/* Clear flag. */
		mov.w	r1, @r0												/* Write it to memory. */
		
		rts
		nop

		.align 2
lib32x_vbi_flag:
		.long 	0x20004020
		
lib32x_vbi_vbi_mask:
		.word	0xFFFE
