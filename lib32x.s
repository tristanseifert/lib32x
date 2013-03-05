/*****************************************************************************************
 * int lib32x_set_sh2_sr(int level);													 *
 * On entry: r4 = new irq level															 *
 * On exit:  r0 = old irq level															 *
 ****************************************************************************************/
        .align  4
        .global _lib32x_set_sh2_sr
        
_lib32x_set_sh2_sr:
        stc     sr,r1
        mov     #0x0F,r0
        shll2   r0
        shll2   r0
        and     r0,r1                   /* just the irq mask */
        shlr2   r1
        shlr2   r1
        not     r0,r0
        stc     sr,r2
        and     r0,r2
        shll2   r4
        shll2   r4
        or      r4,r2
        ldc     r2,sr
        rts
        mov     r1,r0

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
		mov.l	_main_init_complete_comms, r0
		mov.l	_main_init_complete_value, r1
		mov.l	r1, @r0												/* Write to mem loc */
		rts															/* Return to caller. */
		nop

/*****************************************************************************************
 * extern void lib32x_slave_wait_master_init_ok();										 *
 ****************************************************************************************/	
		.align 4
		.global _lib32x_slave_wait_master_init_ok

_lib32x_slave_wait_master_init_ok:	
		mov.l	_main_init_complete_comms, r0
		mov.l	_main_init_complete_value, r1
	
0:
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
		nop															/* Delay some cycles to reduce bus contention */
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
_main_init_complete_comms:
		.long	0x2603FF00											/* note how we are avoiding
																	 * the cache. */
		
_main_init_complete_value:
		.ascii	"M_OK"
		
