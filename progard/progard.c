
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000, 2003, 2008
//-----------------------------------------------------------------------------

// very crude AVR168 (arduino) serial programming routine

//seems like the atmega168 boards want 19200 baud and the atmega328
//boards want 57600 //dev/ttyUSBx
//the uno 115200  /dev/ttyACM0


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include <unistd.h>

#include "ser.h"

FILE *dat;

struct termios oldsettings;

unsigned char data[4096];

//DONT USE
unsigned char sdata[2048]; //dont use
unsigned char rdata[8192]; //dont use
unsigned short records;
unsigned short count;


FILE *fp;

unsigned int errors;

unsigned int addhigh;
unsigned int add;

unsigned int ra;
unsigned int rb;

unsigned int pages;
unsigned int page;

unsigned int line;

unsigned char checksum;

unsigned int len;
unsigned int maxadd;

unsigned char gstring[80];
unsigned char newline[1024];

unsigned char memory[600*1024];

unsigned char t;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int readhex ( void )
{

    memset(memory,0xFF,sizeof(memory));
    maxadd=0;


    line=0;
    while(fgets(newline,sizeof(newline)-1,fp))
    {
        line++;
        if(newline[0]!=':')
        {
            printf("Syntax error <%u> no colon\n",line);
            continue;
        }
        gstring[0]=newline[1];
        gstring[1]=newline[2];
        gstring[2]=0;

        len=strtoul(gstring,NULL,16);
        checksum=0;
        for(ra=0;ra<(len+5);ra++)
        {
            gstring[0]=newline[(ra<<1)+1];
            gstring[1]=newline[(ra<<1)+2];
            gstring[2]=0;
            checksum+=(unsigned char)strtoul(gstring,NULL,16);
        }
        if(checksum)
        {
            printf("checksum error <%u>\n",line);
        }
        gstring[0]=newline[3];
        gstring[1]=newline[4];
        gstring[2]=newline[5];
        gstring[3]=newline[6];
        gstring[4]=0;
        add=strtoul(gstring,NULL,16);
        add|=addhigh;


        if(add>0x80000)
        {
            printf("address too big\n");
            exit(1);
        }

        gstring[0]=newline[7];
        gstring[1]=newline[8];
        gstring[2]=0;
        t=(unsigned char)strtoul(gstring,NULL,16);

        //;llaaaattdddddd
        //01234567890

        switch(t)
        {
            default:
                printf("UNKOWN type %02X <%u>\n",t,line);
                break;
            case 0x00:
                for(ra=0;ra<len;ra++)
                {

                    if(add>maxadd) maxadd=add;

                    gstring[0]=newline[(ra<<1)+9];
                    gstring[1]=newline[(ra<<1)+10];
                    gstring[2]=0;

                    memory[add]=(unsigned char)strtoul(gstring,NULL,16);
//                    printf("%08X: %02X\n",add,t);
                    add++;

                }
                break;
            case 0x01:
                printf("End of data\n");
                break;
            case 0x02:
                gstring[0]=newline[9];
                gstring[1]=newline[10];
                gstring[2]=newline[11];
                gstring[3]=newline[12];
                gstring[4]=0;
                addhigh=strtoul(gstring,NULL,16);
                addhigh<<=16;
                printf("addhigh %08X\n",addhigh);
                break;

        }
    }

    printf("%u lines processed\n",line);
    printf("%08X\n",maxadd);

    if(maxadd&0x7F)
    {
        maxadd+=0x80;
        maxadd&=0xFFFFFF80;
        printf("%08X\n",maxadd);
    }

    return(0);


}


//-----------------------------------------------------------------------------
void showstring ( unsigned short x)
{
    int i;

    if(x==0) return;
    for(i=0;i<x;i++)
    {
        printf("[%02X]",data[i]);
        if((data[i]>0x20)&&(data[i]<127)) printf("%c",data[i]);
    }
    printf("\n");
}
//-----------------------------------------------------------------------------
int get_sync ( void )
{
    int ra,rb;
    for(ra=0;ra<10;ra++)
    {
        sdata[0]=0x30;
        sdata[1]=0x20;
        ser_senddata(sdata,2);
        while(1)
        {
            rb=ser_copystring(data);
            if(rb==2) break;
        }
        ser_dump(rb);
        showstring(rb);
        if((data[0]==0x14)&&(data[1]==0x10))
        {
            printf("got_sync\n");
            return(0);
        }
    }
    printf("no_sync\n");
    return(1);
}
//-----------------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
  //struct termios newsettings;

    if(argc<2)
    {
        printf(".hex file not specified\n");
        return(1);
    }
    fp=fopen(argv[1],"rt");
    if(fp==NULL)
    {
        printf("error opening file %s\n",argv[1]);
        return(1);
    }
    if(readhex()) return(1);
    fclose(fp);

    pages=maxadd>>8;

        printf("pages %u maxadd %u\n",pages,maxadd);



    if(ser_open())
    {
        printf("ser_open() failed\n");
        return(1);
    }
    printf("port opened\n");


    if(get_sync())
    {
        ser_close();
        printf("\n\n");
        return(1);
    }

        sdata[0]=0x41;
        sdata[1]=0x81;
        sdata[2]=0x20;
        ser_senddata(sdata,3);
        while(1)
        {
            rb=ser_copystring(data);
            if(rb==3) break;
        }
        ser_dump(rb);
        showstring(rb);

        sdata[0]=0x41;
        sdata[1]=0x82;
        sdata[2]=0x20;
        ser_senddata(sdata,3);
        while(1)
        {
            rb=ser_copystring(data);
            if(rb==3) break;
        }
        ser_dump(rb);
        showstring(rb);


        sdata[0]=0x50;
        sdata[1]=0x20;
        ser_senddata(sdata,2);
        while(1)
        {
            rb=ser_copystring(data);
            if(rb==2) break;
        }
        ser_dump(rb);
        showstring(rb);

        if((data[0]==0x14)&&(data[1]==0x10))
        {
        }
        else
        {
            ser_close();
            printf("\n\n");
            return(1);
        }


        for(page=0;page<=maxadd;page+=256)
        {
            printf("set address 0x%04X\n",page);
            sdata[0]=0x55;
            sdata[1]=(page>>1)&0xFF;
            sdata[2]=(page>>9)&0xFF;
            sdata[3]=0x20;
            ser_senddata(sdata,4);
            while(1)
            {
                rb=ser_copystring(data);
                if(rb==2) break;
            }
            ser_dump(rb);
            showstring(rb);

            printf("write page\n");
            sdata[0]=0x64;
            sdata[1]=0x01;
            sdata[2]=0x00;
            sdata[3]='F';
            for(ra=0;ra<256;ra++) sdata[4+ra]=memory[ra+page];
            sdata[4+ra]=0x20;
            ser_senddata(sdata,ra+5);
            while(1)
            {
                rb=ser_copystring(data);
                if(rb==2) break;
            }
            ser_dump(rb);
            showstring(rb);

        }


        sdata[0]=0x51;
        sdata[1]=0x20;
        ser_senddata(sdata,2);
        while(1)
        {
            rb=ser_copystring(data);
            if(rb==2) break;
        }
        ser_dump(rb);
        showstring(rb);

        if((data[0]==0x14)&&(data[1]==0x10))
        {
        }
        else
        {
            ser_close();
            printf("\n\n");
            return(1);
        }



  ser_close();
  printf("\n\n");
  return(0);
}
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000
//-----------------------------------------------------------------------------

