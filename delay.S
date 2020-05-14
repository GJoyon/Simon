	;; delay.S
	;; Sua "Joshua" Lee
	;; implementation of delay1us and delay1ms

	.text

	.macro	nop2		; 2 cycle, 1 word NOP
	rjmp .
	.endm

	.global delay1us	; void delay1us(unsigned n);

	;; delay in units of 16 cycles
	;; number of microseconds will be in register pair r24/r25
delay1us:			; assume call is used (4 cycles)
	nop			; 1 cycle
	nop2			; 2 cycles
	rjmp delay2		; 2 cycles

delay1:
	nop2
	nop2
	nop2
	nop2
	nop2
	nop2

delay2:
	sbiw r24, 1		; subtract 1 from r24/r25 (2 cycles)
	brne delay1		; branch if not zero (1 or 2 cycles)

	ret			; return (4 cycles)

	.global delay1ms	; void delay1ms(unsigned n)
	
	;; delay n milliseconds = 1000 * n microseconds (at 16MHz)
	;; number of milliseconds will be in register pair r24/r25
delay1ms:
	movw r26, r24		; save microseconds count (1 cycle)
	rjmp delay4		; 2 cycles

delay3:
	nop2			; 2 cycles
	nop2
	nop2

delay4:
	ldi r24, 231		; r24/r25 contains 231+3*256=999 (1 cycle)
	ldi r25, 3		; 1 cycle
	call delay1us		; delays 999 microseconds
	sbiw r26, 1		; subtract 1 from r26/r27 (2 cycles)
	brne delay3		; branch if not zero (1 or 2 cycles)

	ret			; return (4 cycles)
