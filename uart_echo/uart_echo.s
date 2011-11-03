
.device ATmega168

.equ  UDR       = 0xC6
.equ  UBRRH     = 0xC5
.equ  UBRRL     = 0xC4
.equ  UCSRC     = 0xC2
.equ  UCSRB     = 0xC1
.equ  UCSRA     = 0xC0

mainloop:

getit:
    lds r20,UCSRA
    andi r20,0x80
    breq getit
    lds r18,UDR

sendit:
    lds r20,UCSRA
    andi r20,0x20
    breq sendit
    sts UDR,r18

    rjmp mainloop
