/*************************************************************** K-MAIL ***
* File Name : jetsas.h
* Contents : JetSAS header file 1.00
* Use Module:
* Master CPU1 : Jetson TX2 NVidia
* Slave  CPU2 : R5f72165ADF RENESAS
* Compiler : gcc
* history : 2018.10.08 ver.1.00.00
* Revised : ver.1.00.00
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/
#include "SimpleGPIO.h"
#include "timer.h"
//#include <urg_c/urg_sensor.h>
#include "julius_struct.h"
#include <string>

#define LED_RED 0
#define LED_GREEN 1
#define LED_BLUE 2
#define LED_WHITE 3
///#define LED_YELLOW 2
#define SW1 5
#define SW2 6
#define LED_ON 0
#define LED_OFF 1
#define GPIO_LOW 0
#define GPIO_HIGH 1

#define MPU6050 1
#define S11059  2
#define LPS25H  3
#define SHT31 4

int LCD_init(void);///lcd.cpp
int LCD_DrawString(int x, int y, char *buf);
int LCD_printf(int x, int y, const char*, ...);

int gpio_open(void);///gpio.cpp
void gpio_led(int, int);
int gpio_sw(int);
void gpio_close(void);

int UART_init(void);///serial.cpp
int jetsas(int cmd, int prm1, int prm2);
void* th_receive(void* pParam);
int send(char dat[], int n);
int send_tocos(int );

int spi_open();///spi.cpp
int spi_close();


int I2C1_init(void);///i2c1_sensor.cpp
int mpu6050(float g[]);
int sht31(float g[]);
int lps25h(float g[]);
int s11059(float g[]);
int sensor(int, float *);

int AQUE_init(void);///aquestalk.cpp
int AQUES_Talk(char *);


bool threshold_turning(_julius_result* jr);///exec_cmd.cpp
void* cmd_right(void* pParam);
void* cmd_left(void* pParam);
void* cmd_go(void* pParam);
void* cmd_back(void* pParam);
void* cmd_stop(void* pParam);
void* cmd_come(void* pParam);
void* cmd_push(void* pParam);
void* cmd_look(void* pParam);
void get_encoder(int *time1, int *prm1, int *prm2, int *time2);
//extern int open_urg_sensor(urg_t *urg, int argc, char *argv[]);
