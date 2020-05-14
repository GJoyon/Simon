	;; overflow.S
	;; Sua "Joshua" Lee
	;; Assembly code for interrupt services

	#define __SFR_OFFSET 0
	#include <avr/interrupt.h>

	.text
	.global TIMER1_OVF_vect
TIMER1_OVF_vect:
	push r16		; store registers to be used in service to stack
	in r16, SREG
	push r16

	ldi r16, 0x01		; overflow = 1
	sts overflow, r16

	pop r16			; restore registers
	out SREG, r16
	pop r16

	reti

	.global TIMER2_OVF_vect
TIMER2_OVF_vect:
	push r18		; store registers to be used in service to stack
	push r17
	push r16
	in r16, SREG
	push r16

	ldi r17, 0x3b
	in r16, PINC
	cp r17, r16		; compare 0x3b with PINC
	brsh button_pressed	; go to button_pressed if 0x3b >= PINC

	;; button not pressed
	lsr r16
	lsr r16
	com r16
	sts pinc, r16		; pinc = ~(PINC >> 2)

	clr r17
	sts count, r17		; count = 0
	sts pressed, r17	; pressed = 0
	sts read, r17		; read = 0

	lds r18, idle_count	; idle_count++
	inc r18
	sts idle_count, r18

	cpi r18, 0x00		; check for overflow
	brne tim2_int_finish	; branch to tim2_int_finish if r18 != 0

	lds r18, idle_overflow	; idle_overflow++ if
	inc r18			; an overflow happened in idle_count
	sts idle_overflow, r18

	rjmp tim2_int_finish

button_pressed:
	lds r18, count		; count++
	inc r18
	sts count, r18
	ldi r17, 2
	cp r17, r18		; compare 2 with count
	brsh tim2_int_finish	; go to tim2_int_finish if 2 >= count

	;; pressed long enough
	lsr r16
	lsr r16
	com r16
	sts pinc, r16		; pinc = ~(PINC >> 2)

	ldi r17, 3
	sts count, r17		; count = 3

	clr r17
	sts waiting, r17	; waiting = 0
	sts TCNT1H, r17		; store high-bytes of TCNT1
	sts TCNT1L, r17		; then low-bytes
	sts idle_count, r17	; idle_count = 0
	sts idle_overflow, r17	; idle_overflow = 0

	ldi r17, 1
	sts pressed, r17	; pressed = 1

tim2_int_finish:
	pop r16			; restore registers
	out SREG, r16
	pop r16
	pop r17
	pop r18
	
	reti
	
	.global PCINT0_vect
PCINT0_vect:
	push r16		; save original registers
	in r16, SREG
	push r16

	clr r16			; reset TCNT1, idle_count and
	sts idle_count, r16	; idle_overflow
	sts idle_overflow, r16
	sts TCNT1H, r16
	sts TCNT1L, r16

	pop r16			; restore original registers
	out SREG, r16
	pop r16

	reti
