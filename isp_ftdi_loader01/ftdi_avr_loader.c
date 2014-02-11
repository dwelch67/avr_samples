
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ftdi.h>
//#include <usb.h>

#define VENDOR 0x0403
#define PRODUCT 0x6014

//#define DEBUG

//gcc -o ftdi_avr_loader ftdi_avr_loader.c -lftdi

unsigned char mem[0x10000];
#define ROMMASK 0x00FFFF
unsigned short rom[ROMMASK+1];


unsigned char xstring[80];
unsigned char newline[1024];

unsigned int maxadd;


struct ftdi_context ftdi;
unsigned char buf[256];
unsigned int signature;

int pdi_command ( unsigned int command, unsigned int *retval )
{
    unsigned int rb;
    unsigned int ra;
    int sa;

    *retval=0;

    rb=0;
    buf[rb++]=0x11;
      buf[rb++]=2-1; //length low
      buf[rb++]=0; //length high
      buf[rb++]=(command>>24)&0xFF;
      buf[rb++]=(command>>16)&0xFF;
    buf[rb++]=0x31;
      buf[rb++]=2-1; //length low
      buf[rb++]=0; //length high
      buf[rb++]=(command>>8)&0xFF;
      buf[rb++]=(command>>0)&0xFF;
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        ftdi_usb_reset(&ftdi);
        ftdi_usb_close(&ftdi);
        ftdi_deinit(&ftdi);
        return(1);
    }

    for(ra=0;ra<10;ra++)
    {
        sa = ftdi_read_data(&ftdi, buf, 2 );
#ifdef DEBUG
        printf("%d\n",sa);
#endif
        if(sa<0)
        {
            printf("ftdi_read_data error %d\n",sa);
            return(1);
        }
        if(sa==2)
        {
            *retval=buf[0]; rb<<=8;
            *retval|=buf[1];
#ifdef DEBUG
            printf("0x%04X\n",*retval);
#endif
            return(0);
        }
    }
    printf("pdi_command timed out\n");
    return(1);
}


int pdi_start ( void )
{
    unsigned int rb;
    unsigned int ra;
    unsigned int rc;

    //release reset for 2-3 avr clocks
    rb=0;
    buf[rb++]=0x80; //SET_BITS_LOW
      buf[rb++]=0x10; //value
      buf[rb++]=0x13; //direction
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        return(1);
    }
    usleep(10000); //9ms or more for the erase?
    //assert reset
    rb=0;
    buf[rb++]=0x80; //SET_BITS_LOW
      buf[rb++]=0x00; //value
      buf[rb++]=0x13; //direction
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        return(1);
    }
    //wait for 20ms
    usleep(20000);

    printf("Program Enable\n");
    if(pdi_command(0xAC530000,&ra))
    {
        printf("Write failed\n");
        return(1);
    }
    if((ra&0x00FF)!=0x0053)
    {
        printf("Program Enable command failed\n");
        return(1);
    }

    printf("Read Signature Byte 1\n");
    if(pdi_command(0x30000000,&ra))
    {
        printf("Write failed\n");
        return(1);
    }
    rb=ra&0xFF;
    //ATmEGA32U4 has a signature byte of 0x1E
    printf("Read Signature Byte 2\n");
    if(pdi_command(0x30000100,&ra))
    {
        printf("Write failed\n");
        return(1);
    }
    rb<<=8; rb|=ra&0xFF;
    printf("Read Signature Byte 3\n");
    if(pdi_command(0x30000200,&ra))
    {
        printf("Write failed\n");
        return(1);
    }
    rb<<=8; rb|=ra&0xFF;
    signature=rb;
#ifdef DEBUG
    printf("0x%06X\n",signature);
#endif
    // ATmega16U4 Signature Bytes:
    // 1. 0x000: 0x1E (indicates manufactured by ATMEL).
    // 2. 0x001: 0x94 (indicates 16KB Flash memory).
    // 3. 0x002: 0x88 (indicates ATmega16U4 device).
    // ATmega32U4 Signature Bytes:
    // 1. 0x000: 0x1E (indicates manufactured by ATMEL).
    // 2. 0x001: 0x95 (indicates 32KB Flash memory).
    // 3. 0x002: 0x87 (indicates ATmega32U4 device).
    switch(signature)
    {
        case 0x1E9488:
        {
            printf("ATMEL ATmega16U4 16KB\n");
            break;
        }
        case 0x1E9587:
        {
            printf("ATMEL ATmega32U4 32KB\n");
            break;
        }
        default:
        {
            //not really an error, this code is written for the ATmega32u4
            //should be easy to adapt to another part.
            printf("Unknown signature 0x%06X\n",signature);
            return(1);
        }
    }
    return(0);
}

int program_avr ( void )
{
    unsigned int rb;
    unsigned int ra;
    unsigned int rc;

    rb=0;
    buf[rb++]=0x80; //SET_BITS_LOW
      buf[rb++]=0x00; //value
      buf[rb++]=0x13; //direction
    buf[rb++]=0x86; //TCK_DIVISOR,
      buf[rb++]=0x02;
      buf[rb++]=0x00;
    buf[rb++]=0x82; //SET_BITS_HIGH,
      buf[rb++]=0x00;
      buf[rb++]=0x00;
    buf[rb++] = 0x87; //SEND_IMMEDIATE;
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        return(1);
    }
    usleep(20000);

    if(pdi_start()) return(1);

    printf("----------\n");
    for(ra=0;ra<10;ra++)
    {
        if(pdi_command(0x28000000|(ra<<8),&rb)) return(1);
        if(pdi_command(0x20000000|(ra<<8),&rc)) return(1);
        rb<<=8;
        rb|=rc&0xFF;
        rb&=0xFFFF;
        printf("[0x%04X] 0x%04X\n",ra,rb);
    }
    printf("----------\n");

    printf("Erase\n");
    if(pdi_command(0xAC800000,&ra)) return(1);
    if(pdi_start()) return(1);

    printf("----------\n");
    for(ra=0;ra<10;ra++)
    {
        if(pdi_command(0x28000000|(ra<<8),&rb)) return(1);
        if(pdi_command(0x20000000|(ra<<8),&rc)) return(1);
        rb<<=8;
        rb|=rc&0xFF;
        rb&=0xFFFF;
        printf("[0x%04X] 0x%04X\n",ra,rb);
    }
    printf("----------\n");




    ////program one page
    for(ra=0;ra<64;ra++)
    {
        if(pdi_command(0x40000000|(ra<<8)|((rom[ra]>>0)&0xFF),&rb)) return(1); //low first
        if(pdi_command(0x48000000|(ra<<8)|((rom[ra]>>8)&0xFF),&rb)) return(1); //then high
    }
    if(pdi_command(0x4D000000,&rb)) return(1);
    if(pdi_command(0x4C000000,&rb)) return(1);
    //asmdelay(40000); //4.5ms or more
    usleep(5000);


    printf("----------\n");
    for(ra=0;ra<30;ra++)
    {
        if(pdi_command(0x28000000|(ra<<8),&rb)) return(1);
        if(pdi_command(0x20000000|(ra<<8),&rc)) return(1);
        rb<<=8;
        rb|=rc&0xFF;
        rb&=0xFFFF;
        printf("[0x%04X] 0x%04X\n",ra,rb);
    }
    printf("----------\n");




    //assert reset
    rb=0;
    buf[rb++]=0x80; //SET_BITS_LOW
      buf[rb++]=0x00; //value
      buf[rb++]=0x13; //direction
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        return(1);
    }
    usleep(20000);
    //release reset and let the part run.
    rb=0;
    buf[rb++]=0x80; //SET_BITS_LOW
      buf[rb++]=0x10; //value
      buf[rb++]=0x13; //direction
    if (ftdi_write_data(&ftdi,buf, rb) != rb)
    {
        printf("Write failed\n");
        return(1);
    }
    return(0);
}





//-----------------------------------------------------------------------------
int readhex ( FILE *fp )
{
    //unsigned int errors;

    unsigned int addhigh;
    unsigned int add;

    unsigned int ra;
    //unsigned int rb;

    //unsigned int pages;
    //unsigned int page;

    unsigned int line;

    unsigned char checksum;

    unsigned int len;

    unsigned char t;

    addhigh=0;
    memset(rom,0x55,sizeof(rom));

    line=0;
    while(fgets(newline,sizeof(newline)-1,fp))
    {
        line++;
        //printf("%s",newline);
        if(newline[0]!=':')
        {
            printf("Syntax error <%u> no colon\n",line);
            continue;
        }
        xstring[0]=newline[1];
        xstring[1]=newline[2];
        xstring[2]=0;

        len=strtoul(xstring,NULL,16);
        checksum=0;
        for(ra=0;ra<(len+5);ra++)
        {
            xstring[0]=newline[(ra<<1)+1];
            xstring[1]=newline[(ra<<1)+2];
            xstring[2]=0;
            checksum+=(unsigned char)strtoul(xstring,NULL,16);
        }
        if(checksum)
        {
            printf("checksum error <%u>\n",line);
        }
        xstring[0]=newline[3];
        xstring[1]=newline[4];
        xstring[2]=newline[5];
        xstring[3]=newline[6];
        xstring[4]=0;
        add=strtoul(xstring,NULL,16);
        add|=addhigh;


        if(add>0x80000)
        {
            printf("address too big 0x%04X\n",add);
            return(1);
        }
        if(len&1)
        {
            printf("length odd\n");
            return(1);
        }



        xstring[0]=newline[7];
        xstring[1]=newline[8];
        xstring[2]=0;
        t=(unsigned char)strtoul(xstring,NULL,16);

        //;llaaaattdddddd
        //01234567890

        switch(t)
        {
            default:
                printf("UNKOWN type %02X <%u>\n",t,line);
                break;
            case 0x00:
                len>>=1;
                for(ra=0;ra<len;ra++)
                {

                    if(add>maxadd) maxadd=add;

                    xstring[0]=newline[(ra<<2)+9+2];
                    xstring[1]=newline[(ra<<2)+9+3];
                    xstring[2]=newline[(ra<<2)+9+0];
                    xstring[3]=newline[(ra<<2)+9+1];
                    xstring[4]=0;

                    rom[add>>1]=(unsigned short)strtoul(xstring,NULL,16);
                    //fprintf(fpout," 0x%04X, //%08X\n",rom[add>>1],add>>1);
                    printf("[0x%04X] 0x%04X\n",add>>1,rom[add>>1]);
                    add+=2;
                }
                break;
            case 0x01:
                printf("End of data\n");
                break;
            case 0x02:
                xstring[0]=newline[9];
                xstring[1]=newline[10];
                xstring[2]=newline[11];
                xstring[3]=newline[12];
                xstring[4]=0;
                addhigh=strtoul(xstring,NULL,16);
                addhigh<<=16;
                printf("addhigh %08X\n",addhigh);
                break;

        }
    }

    //printf("%u lines processed\n",line);
    //printf("%08X\n",maxadd);

    //if(maxadd&0x7F)
    //{
        //maxadd+=0x80;
        //maxadd&=0xFFFFFF80;
        //printf("%08X\n",maxadd);
    //}

    //for(ra=0;ra<maxadd;ra+=2)
    //{
        //printf("0x%04X: 0x%04X\n",ra,rom[ra>>1]);
    //}

    return(0);


}



//-------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    FILE *fp;

    if(argc<2)
    {
        printf("hex file not specified\n");
        return(1);
    }
    fp=fopen(argv[1],"rt");
    if(fp==NULL)
    {
        printf("error opening file [%s]\n",argv[1]);
        return(1);
    }
    if(readhex(fp)) return(1);
    fclose(fp);

    maxadd>>=1;
    printf("maxadd 0x%04X\n",maxadd);

    ftdi_init(&ftdi);
    if (ftdi_usb_open_desc(&ftdi, VENDOR, PRODUCT, 0, 0) < 0)
    {
        printf("Can't open FTDI device\n");
        return 1;
    }
    ftdi_usb_reset(&ftdi);
    ftdi_set_interface(&ftdi, INTERFACE_A);
    ftdi_set_latency_timer(&ftdi, 1);
    ftdi_set_bitmode(&ftdi, 0xfb, BITMODE_MPSSE);

    program_avr();

    ftdi_usb_reset(&ftdi);
    ftdi_usb_close(&ftdi);
    ftdi_deinit(&ftdi);
    return 0;
}
