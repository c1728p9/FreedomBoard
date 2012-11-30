/*
 * interrupts.s
 *
 */


.global interrupt_disabled


.text

@.thumb
.thumb_func			@this is needed to tell the assembler that the label is for a thumb function
interrupt_disabled:
	mrs r0,PRIMASK
	bx	lr
