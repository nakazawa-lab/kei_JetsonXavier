/*************************************************************** K-MAIL ***
* File Name : serial.cpp
* Contents : JetSAS RS232C control program 1.00
* Use Module:
* Master CPU1 : Jetson Xavier NVidia
* Slave  CPU2 : R5f72165ADF RENESAS
* Compiler : gcc
* history : 2015.04.12 ver.1.00.00
* Revised : ver.1.00.00
* Revised : 2019.07.18 ver.1.10.00
* avairable buad rate  110, 300, 600, 1200, 2400, 4800, 9600, 14400,
* 19200, 38400, 57600, 115200, 230400, 460800, 921600
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset

#include "jetsas.h"

int tty_fd0, tty_fd1 ;

/*************************************************************** UART_init ***/
int UART_init(void)
{
    struct termios tio;
    char tty0[]="/dev/ttyTHS0";
///    char tty0[]="/dev/ttyTHS2";
///    char tty1[]="/dev/ttyTHS1";
//    int tty_fd;
    int count=0;
    fd_set rdset;

    unsigned char c='D', r='R';

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;
    // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;

    tio.c_cc[VTIME]=5;

    tty_fd0=open(tty0, O_RDWR | O_NONBLOCK);
    printf("tty_fd0=%d \n",tty_fd0);
    if(tty_fd0<0) return 0;
    cfsetospeed(&tio,B230400);            // 921600 baud
    cfsetispeed(&tio,B230400);            // 921600 baud
    tcsetattr(tty_fd0,TCSANOW,&tio);
/***
    tty_fd1=open(tty1, O_RDWR | O_NONBLOCK);
    printf("tty_fd1=%d \n",tty_fd1);
    if(tty_fd1<0) return 0;
    cfsetospeed(&tio,B115200);            /// 921600 baud
    cfsetispeed(&tio,B115200);            /// 921600 baud
    tcsetattr(tty_fd1,TCSANOW,&tio);
***/
    return 0;
}/****************************************************************** END ***/

/*************************************************************** jetsas ***/
int jetsas(int cmd,int prm1, int prm2)
{
    static int c=0x45, ret;
///    int dat[]= {'v',',','5','0','1','0',',','5','0','1','0',0x0d};
///    int dat[]= {'m',',','0','0','0','1',',','0','0','0','0',0x0d};
///    char dat[]= {'m',',','0','0','0','1',',','0','0','0','0',0x0d};
    char dt1[]= {'m',',','0','0','0','1',',','0','0','0','0',0x0d};
    char dt2[]= {'v',',','5','0','1','0',',','4','9','9','0',0x0d};

//    char dat[]= "m,0001,0000";//{'m',',','0','0','0','1',',','0','0','0','0',0x0d};
printf("jetsas\n");

    send(dt1,12);
    usleep(1000);
    send(dt2,12);
//    printf("%d ret=%d\n",c,ret);
//    if(c>100)c=32;

    return 0;
}/****************************************************************** END ***/

/*************************************************************** jetsas ***/
int send(char dat[], int n)
{
    int i, ret;
    for(i=0; i<n; i++)
    {
        printf("%d \n",i);
///        ret=write(tty_fd0,&dat[i],1);
        ret=write(tty_fd0,&dat[i],1);
        usleep(5);
    }
    return 0;
}/****************************************************************** END ***/
/*************************************************************** jetsas ***/
int send_tocos(int d)
{
    int i, ret;
    char buf = '0x55';

///        ret=write(tty_fd0,&dat[i],1);
    buf= 0x41+d;
    ret=write(tty_fd1,&buf,1);
    usleep(5);
    if (d==25)
    {
        buf= 0x0d;
        ret=write(tty_fd1,&buf,1);
    }
    usleep(5);
    return 0;
}/****************************************************************** END ***/

/*************************************************************** th_receive ***/
void* th_receive(void* pParam)
{
    unsigned char r='R';
    int a=1,ct=0;

    while(1)
    {
        while(read(tty_fd0,&r,1)>0) printf("received from NakBot %x \n",r);
///        while(read(tty_fd1,&r,1)>0) printf("received from TOCOS %x \n",r);

        if(ct>500)
        {
            gpio_led(LED_BLUE,a^=1);
            ct=0;
        }
        ct++;
        usleep(1000);
//       printf("pass ");
    }
}/****************************************************************** END ***/


/****
c=32;
        while ((count++)<100)
        {
		c++;
		write(tty_fd,&c,1);
//		printf("data=%c /n\n",c);
		usleep(10);
//		pauseNanoSec(10000000);


        }

	while(read(tty_fd,&r,1)>0) printf("       buffered data = %c \n",r);

        close(tty_fd);
}
*****/

void jetsas0(char cmd, int prm1, int prm2)
{
    char dat[12];
    if(cmd=='v')
    {
        prm1=10000-prm1; /// 左車輪の速度指令値
        prm2=10000-prm2; /// 右車輪の速度指令値
        sprintf(dat, "%c,%04d,%04d\n", cmd, prm2, prm1);
    }
    else
    {
        sprintf(dat, "%c,%04d,%04d\n", cmd, prm1, prm2);
    }
    send0(dat,12);
}/****************************************************************** END ***/

/************************************************************* モータ制御 ***
void jetsas0(char cmd, int prm1, int prm2){
	char dat[12];
	sprintf(dat, "%c,%04d,%04d\n", cmd, prm1, prm2);
///	printf("%s", dat);
    send0(dat,12);
}/****************************************************************** END ***/

