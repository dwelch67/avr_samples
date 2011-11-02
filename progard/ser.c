
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <time.h>

int ser_hand;
unsigned short ser_buffcnt;
unsigned short ser_maincnt;
unsigned char ser_buffer[8192];

//-----------------------------------------------------------------------------
unsigned char ser_open ( void )
{
  struct termios options;
//  int ret;
  int dtr_bit=TIOCM_DTR;
//  int setbits;

  if(0) {  ser_hand=open("/dev/ttyACM0",O_RDWR|O_NOCTTY|O_NDELAY); } //UNO
  else  {  ser_hand=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY|O_NDELAY); } //fill in usb number
  if(ser_hand==-1)
  {
    fprintf(stderr,"open: error - %s\n",strerror(errno));
    return(1);
  }
  fcntl(ser_hand,F_SETFL,FNDELAY);
  bzero(&options,sizeof(options));
  if(0)      { options.c_cflag=B115200|CS8|CLOCAL|CREAD; } //UNO
  else if(0) { options.c_cflag=B19200|CS8|CLOCAL|CREAD; } //atmega168 3.3v 19200 pro mini 5v? lilypad 3.3v
  else if(1) { options.c_cflag=B57600|CS8|CLOCAL|CREAD; } //atmega328 5.0v 57600
  options.c_iflag=IGNPAR;
  tcflush(ser_hand,TCIFLUSH);
  tcsetattr(ser_hand,TCSANOW,&options);
  ser_maincnt=ser_buffcnt=0;


    if(0) //UNO, pro 5v but not mini
    {
        dtr_bit=TIOCM_DTR;
        ioctl(ser_hand,TIOCMBIC,&dtr_bit);
        sleep(1);
    }
    else if(0)
    {
        dtr_bit=TIOCM_DTR;
        ioctl(ser_hand,TIOCMBIC,&dtr_bit);
        sleep(1);
        dtr_bit=TIOCM_DTR;
        ioctl(ser_hand,TIOCMBIS,&dtr_bit);
    }
    else //pro mini wanted this/ pro was okay with it too. so was uno! and lilypad 3.3v!
    {
        dtr_bit=TIOCM_DTR;
        ioctl(ser_hand,TIOCMBIC,&dtr_bit);
        sleep(1);
        dtr_bit=TIOCM_DTR;
        ioctl(ser_hand,TIOCMBIS,&dtr_bit);
        sleep(1);
    }

  return(0);
}
//-----------------------------------------------------------------------------
void strobedtr ( void )
{
  int dtr_bit;
  time_t x;

  dtr_bit=TIOCM_DTR;

  ioctl(ser_hand,TIOCMBIS,&dtr_bit);
  x=time(NULL)+5;
  while(time(NULL)<x);
  ioctl(ser_hand,TIOCMBIC,&dtr_bit);
}
//----------------------------------------------------------------------------
void ser_close ( void )
{
  close(ser_hand);
}
//-----------------------------------------------------------------------------
void ser_senddata ( unsigned char *s, unsigned short len )
{
    ssize_t ret;
  ret=write(ser_hand,s,len);
  if(ret<0)
  {
      printf("write error\n");
  }
  tcdrain(ser_hand);
}
//-----------------------------------------------------------------------------
void ser_sendstring ( char *s )
{
    ssize_t ret;
    ret=write(ser_hand,s,strlen(s));
    if(ret<0)
    {
        printf("write errors\n");
    }
  tcdrain(ser_hand);
}
//-----------------------------------------------------------------------------
void ser_update ( void )
{
  int r;

  r=read(ser_hand,&ser_buffer[ser_maincnt],4000);
  if(r>0)
  {
    ser_maincnt+=r;
    if(ser_maincnt>4095)
    {
      ser_maincnt&=0xFFF;
      memcpy(ser_buffer,&ser_buffer[4096],ser_maincnt);
    }
  }

}
//-----------------------------------------------------------------------------
unsigned short ser_copystring ( unsigned char *d )
{
    unsigned short r;
    unsigned short buffcnt;
    unsigned short maincnt;

    ser_update();
    buffcnt=ser_buffcnt;
    maincnt=ser_maincnt;
    for(r=0;buffcnt!=maincnt;buffcnt=(buffcnt+1)&0xFFF,r++) *d++=ser_buffer[buffcnt];
    return(r);
}
//-----------------------------------------------------------------------------
unsigned short ser_dump ( unsigned short x )
{
    unsigned short r;

    for(r=0;r<x;r++)
    {
        if(ser_buffcnt==ser_maincnt) break;
        ser_buffcnt=(ser_buffcnt+1)&0xFFF;
    }
    return(r);
}
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000
//-----------------------------------------------------------------------------

