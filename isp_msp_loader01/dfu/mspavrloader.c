

#define  USE_MSP_UART


//promicro board JP6 (right side)

//1
//2   gnd Awire
//3   reset
//4   vcc
//5
//6
//7
//8
//9   sck
//10  miso
//11  mosi
//12

//msp

//p1.4    to mosi
//p1.5    to miso
//p1.6    to sck
//p1.7    to reset

#include "blinker.h"

extern void asmdelay ( unsigned short );

#define WDTCTL     (*((volatile unsigned short *)0x0120))

#define CALBC1_1MHZ (*((volatile unsigned char *)0x10FF))
#define CALDCO_1MHZ (*((volatile unsigned char *)0x10FE))
#define CALBC1_8MHZ (*((volatile unsigned char *)0x10FD))
#define CALDCO_8MHZ (*((volatile unsigned char *)0x10FC))
#define CALBC1_12MHZ (*((volatile unsigned char *)0x10FB))
#define CALDCO_12MHZ (*((volatile unsigned char *)0x10FA))
#define CALBC1_16MHZ (*((volatile unsigned char *)0x10F9))
#define CALDCO_16MHZ (*((volatile unsigned char *)0x10F8))

#define DCOCTL  (*((volatile unsigned char *)0x56))
#define BCSCTL1 (*((volatile unsigned char *)0x57))
#define BCSCTL2 (*((volatile unsigned char *)0x58))

#define TACTL   (*((volatile unsigned short *)0x0160))
#define TAR     (*((volatile unsigned short *)0x0170))
#define TACCR0  (*((volatile unsigned short *)0x0172))
#define TACCTL0 (*((volatile unsigned short *)0x0162))

#ifdef USE_MSP_UART

#define UCA0CTL0    (*((volatile unsigned char *)0x060 ))
#define UCA0CTL1    (*((volatile unsigned char *)0x061 ))
#define UCA0BR0     (*((volatile unsigned char *)0x062 ))
#define UCA0BR1     (*((volatile unsigned char *)0x063 ))
#define UCA0MCTL    (*((volatile unsigned char *)0x064 ))
#define UCA0STAT    (*((volatile unsigned char *)0x065 ))
#define UCA0RXBUF   (*((volatile unsigned char *)0x066 ))
#define UCA0TXBUF   (*((volatile unsigned char *)0x067 ))
#define UCA0ABCTL   (*((volatile unsigned char *)0x05D ))
#define UCA0IRTCTL  (*((volatile unsigned char *)0x05E ))
#define UCA0IRRCTL  (*((volatile unsigned char *)0x05F ))
#define IE2         (*((volatile unsigned char *)0x001 ))
#define IFG2        (*((volatile unsigned char *)0x003 ))

#endif

#define P1IN  (*((volatile unsigned char *)0x0020))
#define P1OUT (*((volatile unsigned char *)0x0021))
#define P1DIR (*((volatile unsigned char *)0x0022))
#define P1REN (*((volatile unsigned char *)0x0027))
#define P1SEL (*((volatile unsigned char *)0x0026))
#define P1SEL2 (*((volatile unsigned char *)0x0041))

//-------------------------------------------------------------------
#ifdef USE_MSP_UART
void uart_init ( void )
{
    BCSCTL2 &=~0x06;

    UCA0CTL0 = 0x00;
    UCA0CTL1 = 0xC0;
    UCA0BR0  = 0x08;
    UCA0BR1  = 0x00;
    UCA0MCTL = 0xB1;

    UCA0STAT  = 0x00;
    UCA0ABCTL = 0x00;
    IE2       = 0x00;

    P1SEL  |= 0x06;
    P1SEL2 |= 0x06;
}
#endif
//-------------------------------------------------------------------
#ifdef USE_MSP_UART
void uart_putc ( unsigned char c )
{
    while(UCA0STAT&0x01) continue;
    UCA0TXBUF=c;
}
#endif
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void hexstrings ( unsigned short d )
{
#ifdef USE_MSP_UART
    //unsigned short ra;
    unsigned short rb;
    unsigned short rc;

    rb=16;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart_putc(rc);
        if(rb==0) break;
    }
    uart_putc(0x20);
#endif
}
//-------------------------------------------------------------------
void hexstring ( unsigned short d )
{
#ifdef USE_MSP_UART
    hexstrings(d);
    uart_putc(0x0D);
    uart_putc(0x0A);
#endif
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#define AVR_MOSI_PIN 4
#define AVR_MISO_PIN 5
#define AVR_SCK_PIN  6
#define AVR_RESET_PIN 7

#define SET_MOSI  P1OUT|=(1<<AVR_MOSI_PIN)
#define SET_SCK   P1OUT|=(1<<AVR_SCK_PIN)
#define SET_RESET P1OUT|=(1<<AVR_RESET_PIN)

#define CLR_MOSI  P1OUT&=~(1<<AVR_MOSI_PIN)
#define CLR_SCK   P1OUT&=~(1<<AVR_SCK_PIN)
#define CLR_RESET P1OUT&=~(1<<AVR_RESET_PIN)

#define DELX 5

//-------------------------------------------------------------------
unsigned short pdi_command ( unsigned short upper, unsigned short lower )
{
    unsigned short ra,rb,rc;//,rd;

    //hexstrings(upper); hexstring(lower);
    rb=upper;
    rc=0x0000;
    //rd=0x0000;
    asmdelay(DELX);
    for(ra=0;ra<32;ra++)
    {
        if(ra==16)
        {
            rb=lower;
            //rd=rc;
        }
        if(rb&0x8000) SET_MOSI; else CLR_MOSI; rb<<=1;
        rc<<=1; if(P1IN&(1<<AVR_MISO_PIN)) rc|=1;
        asmdelay(DELX);
        SET_SCK;
        asmdelay(DELX);
        asmdelay(DELX);
        CLR_SCK;
        asmdelay(DELX);
    }
    asmdelay(DELX);
    //hexstrings(rd); hexstring(rc);
    return(rc);
}

//-------------------------------------------------------------------
void longerdelay ( unsigned short a, unsigned short b )
{
    unsigned short ra;

    for(ra=0;ra<a;ra++) asmdelay(b);
}
//-------------------------------------------------------------------
int notmain ( void )
{
    unsigned short ra;
    unsigned short rb;
    unsigned short rc;
    unsigned short page;

    WDTCTL = 0x5A80;

    // use calibrated clock
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

#ifdef USE_MSP_UART
    uart_init();
    hexstring(0x1234);
#endif


    P1DIR|=(1<<AVR_MOSI_PIN)|(1<<AVR_SCK_PIN)|(1<<AVR_RESET_PIN);
    P1OUT&=~((1<<AVR_MOSI_PIN)|(1<<AVR_SCK_PIN)|(1<<AVR_RESET_PIN));

    SET_RESET;
    asmdelay(1000);
    CLR_RESET;
    //20ms or more
    longerdelay(4,40000);
    ra=pdi_command(0xAC53,0x0000);
    if((ra&0xFF00)!=0x5300)
    {
        hexstring(0xBAD);
        return(1);
    }
    ra=pdi_command(0x3000,0x0000);
    if((ra&0xFF)!=0x1E)
    {
        hexstring(0xBAD);
        return(1);
    }
    ra=pdi_command(0x3000,0x0100);
    rb=ra&0xFF;
    ra=pdi_command(0x3000,0x0200);
    rb<<=8; rb|=ra&0xFF;
    if(rb!=0x9587)
    {
        //not really an error, this code is written for the atmega32u4
        //should be easy to adapt to another part.
        hexstring(0xBAD);
        return(1);
    }

    for(ra=0;ra<10;ra++)
    {
        rb=pdi_command(0x2800,0x0000|(ra<<8));
        rc=pdi_command(0x2000,0x0000|(ra<<8));
        rb&=0xFF;
        rc&=0xFF;
        hexstrings(ra); hexstring((rb<<8)|rc);
    }

    //erase
    pdi_command(0xAC80,0x0000);
    SET_RESET;
    longerdelay(2,40000); //9ms or more
    CLR_RESET;

    //20ms or more
    longerdelay(4,40000);
    ra=pdi_command(0xAC53,0x0000);
    if((ra&0xFF00)!=0x5300)
    {
        hexstring(0xBAD);
        return(1);
    }
    ra=pdi_command(0x3000,0x0000);
    if((ra&0xFF)!=0x1E)
    {
        hexstring(0xBAD);
        return(1);
    }
    ra=pdi_command(0x3000,0x0100);
    rb=ra&0xFF;
    ra=pdi_command(0x3000,0x0200);
    rb<<=8; rb|=ra&0xFF;
    if(rb!=0x9587)
    {
        //not really an error, this code is written for the atmega32u4
        //should be easy to adapt to another part.
        hexstring(0xBAD);
        return(1);
    }

    for(ra=0;ra<10;ra++)
    {
        rb=pdi_command(0x2800,0x0000|(ra<<8));
        rc=pdi_command(0x2000,0x0000|(ra<<8));
        rb&=0xFF;
        rc&=0xFF;
        hexstrings(ra); hexstring((rb<<8)|rc);
    }

    for(rb=0;rb<32;rb++)
    {
        //program one page
        for(ra=0;ra<64;ra++)
        {
            pdi_command(0x4000,(ra<<8)|((rom[(rb<<6)|ra]>>0)&0xFF)); //low first
            pdi_command(0x4800,(ra<<8)|((rom[(rb<<6)|ra]>>8)&0xFF)); //then high
        }
        pdi_command(0x4D00,0x0000);
        page=0xE0+rb;
        pdi_command(0x4C00|(page>>2),(page&3)<<14);
        asmdelay(40000); //4.5ms or more
    }

    hexstrings(0x1111); hexstrings(0x2222); hexstring(0x3333);

    for(ra=0;ra<70;ra++)
    {
        rb=pdi_command(0x2800,0x0000|(ra<<8));
        rc=pdi_command(0x2000,0x0000|(ra<<8));
        rb&=0xFF;
        rc&=0xFF;
        hexstrings(ra); hexstring((rb<<8)|rc);
    }



    SET_RESET;

    return(0);


}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
