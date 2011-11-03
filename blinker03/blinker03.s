
.device ATmega168

.equ DDRB       = 0x04
.equ PORTB      = 0x05

.equ TCCR0A     = 0x24
.equ TCCR0B     = 0x25
.equ TCNT0      = 0x26

.equ CLKPR      = 0x61

.org 0x0000
    rjmp RESET

RESET:
    ldi R16,0x80
    ldi R17,0x07 ;0x7 1/128  0x8 1/256
    sts CLKPR,R16
    sts CLKPR,R17

    ldi R16,0x00
    out TCCR0A,R16
    ldi R16,0x05 ;0x5 1/1024 0x06 1/256
    out TCCR0B,R16

    ldi R16,0x20
    out DDRB,R16

    ldi R17,0x00
    ldi R20,0x20
Loop:

    in R17,TCNT0
aloop:
    in R18,TCNT0
    sub r18,r17
    cpi r18,0x1E
    brlo aloop

    eor R16,R20
    out PORTB, R16

    rjmp Loop
