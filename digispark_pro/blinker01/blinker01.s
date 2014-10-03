
.equ  PINB       = 0x03
.equ  DDRB       = 0x04
.equ  PORTB      = 0x05

.org 0x0000
    rjmp RESET

RESET:
    in R16,DDRB
    ori R16,0x02
    out DDRB,R16

    ldi R20,0x02
    out PINB,R20
    ;rjmp RESET

    ldi R18,0x00
    ldi R17,0x00
    ldi R20,0x02
Loop:

    ldi R19,0xF8
aloop:
    inc R17
    cpi R17,0x00
    brne aloop

    inc R18
    cpi R18,0x00
    brne aloop

    inc R19
    cpi R19,0x00
    brne aloop

    out PINB,R20
    rjmp Loop

