/*************************************************************** K-MAIL ***
* File Name : main.cpp
* Contents : JetSAS main program 1.00
* Use Module:
* Master CPU1 : Jetson Xavier NVidia
* Slave  CPU2 : R5f72165ADF RENESAS
* Compiler : gcc
* history : 2018.10.08 ver.1.00.00
* Revised : 2019.07.18 ver.1.10.00
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <pthread.h>///

#include <iostream>
#include <string>
#include <unistd.h>

#include <opencv2/opencv.hpp>

#include <urg_c/urg_sensor.h>
#include <urg_c/urg_utils.h>

#include "SimpleGPIO.h"
//#include <GL/glut.h>
#include "jetsas.h"
#include "open_urg_sensor.h"

/// 20190911 for speech recognition
#include <boost/regex.hpp>
#include <boost/asio.hpp>
using namespace boost::asio;

using namespace std;
void* thread1(void* pParam); ///
pthread_mutex_t mutex; ///
int count1=0;///
urg_t urg;  ///for URG sensor
long *urg_data;
///extern int open_urg_sensor(urg_t *urg, int argc, char *argv[]);

/******************************************************** JetsonTK1_init ***/
int JetsonXavier_init()
{
    char *dummy[1];
    LCD_init();
    gpio_open();
///    spi_open();
    UART_init();
    I2C1_init();
///    AQUE_init(); ///need not init
    if (open_urg_sensor(&urg, 1, dummy) < 0) return 1;
    urg_data = (long *)malloc(urg_max_data_size(&urg) * sizeof(urg_data[0]));

    return 0;
}/****************************************************************** END ***/

/****************************************************************** main ***/
int main(int argc, char *argv[])
{
    int fd,i,ret;
    int addr = 0x3e;
    unsigned char buf[20];
    char buf2[30] = "JetSAS on NakBot";
    JET_TIMER jt;
    int t_s, t_ns;
    float g[20], y, z;

    long max_distance;///for URG sensor
    long min_distance;
    long time_stamp;
    int n;

    cv::Mat img = cv::Mat::zeros(500, 680, CV_8UC3);
    cv::namedWindow("URG data", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    cv::imshow("URG data", img);

    pthread_t tid1, tid2, tid3;
    pthread_mutex_init(&mutex, NULL);

    printf("Start JetSAS on NakBot\n");

    JetsonXavier_init();

    LCD_printf(0, 0,"JetSAS-X on NakBot");
//    AQUES_Talk("#J"); ///chime
//    AQUES_Talk("watasiha nakkubottodesu");
//    AQUES_Talk("mai neimu izu nakkubotto");
    jt.reset();
    pthread_create(&tid1, NULL, th_receive, NULL);

    printf("jt time0 %d[sec] %d[nsec]\n",jt.get_sec(),jt.get_nsec());


    /**20190911*********************************** julius setup ***/
    /// connect with Julius
    const std::string ip = "127.0.0.1";
    const int port = 10500;
    ip::address ADDR = ip::address::from_string(ip);
    ip::tcp::endpoint ep(ADDR, port);
    ip::tcp::iostream s(ep);

    /// ready to fetch texts that you want
    boost::regex o_regex("SCORE=\"([^\"]+)\"");
    boost::smatch l_match;
    boost::regex p_regex("CM=\"([^\"]+)\"");
    boost::smatch m_match;
    boost::regex q_regex("WORD=\"([^\"]+)\"");
    boost::smatch n_match;
    boost::regex r_regex("MSEC=\"([^\"]+)\"");
    boost::smatch o_match;

    string line, score, cm, word, reply, msec;
    string cmnd0, cmnd1, cmnd2, cmfm0, cmfm1;
    string word0, word1, word2, word3, word4, word5;
    float cmscore0, cmscore1, cmscore2, cmscore3, cmscore4, cmscore5, rcmscore, cmtime;
    float abscore0, abscore1, abscore2, abscore3, abscore4, abscore5;
    float rsltcmscore0, rsltcmscore1, rsltcmscore2, rsltcmscore3, rsltcmscore4, rsltcmscore5;
    float rsltscore0, rsltscore1, rsltscore2, rsltscore3, rsltscore4, rsltscore5;
    float dscore = 0;

//    pthread_create(&tid2, NULL, cmd_stop, NULL);

    getline(s, line);   /// サーバーのチェック
    if (line.find("<STARTPROC/>") != string::npos)
    {
        printf("============= Started speech recognition =============\n");
//        s << "PAUSE\n" << std::flush;
//        alutSleep(0.7);
//        say_ready();   /// "hello, master. i'm ready."
//        s << "RESUME\n" << std::flush;
    }
    else    /// Juliusのサーバーが動いてないとここで止まります。
    {
//        alutSleep(0.7);
//        say_error();   /// "master, check the connection."
        printf("============= Can't connect with Julius module! =============\n");
        return 1;
    }

    /******************************************************** end ***/

    while (1){

    }
//    for(i=0; i<2; i++)
//    {
//        gpio_led(LED_RED,LED_OFF);
//        gpio_led(LED_BLUE,LED_OFF);
//        gpio_led(LED_GREEN,LED_OFF);
//
//        usleep(500000);         // on for 200ms
//        gpio_led(LED_RED,LED_ON);
//        gpio_led(LED_BLUE,LED_ON);
//        gpio_led(LED_GREEN,LED_ON);
//        usleep(500000);         // on for 200ms
//    }
//    gpio_led(LED_RED,LED_OFF);
//    gpio_led(LED_BLUE,LED_OFF);
//    gpio_led(LED_GREEN,LED_OFF);
//
//    printf("jt time1 %d[sec] %d[nsec]\n",jt.get_sec(),jt.get_nsec());
//
//    jetsas(1,2,3);
//
//    LCD_printf(0, 0,"START");
//
//
/////    sensor(SHT31,g);
/////    sensor(S11059,g);
//    /*****/
//    sensor(LPS25H,g);
//    printf("LPS25H %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",g[0],g[1],g[2],g[3],g[4],g[5]);
//    while(gpio_sw(SW1));
//    /****/
//
//    usleep(1000000);         // on for 200ms
//
//    while(1)
//    {
//        sensor(MPU6050,g);
//        printf("MPU6050 %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",g[0],g[1],g[2],g[3],g[4],g[5]);
//
/////        usleep(500000);         // on for 200ms
//        LCD_printf(0,0,"%7.4f %7.4f",g[0],g[1]);
//        LCD_printf(0,1,"%7.4f ",g[2]);
//
//        if (gpio_sw(SW1)==LOW) break;
//    };
//    // Wait for the push button to be pressed
//    cout << "Please press the button!" << endl;
//
//    unsigned int value = LOW;
//    int j=0;
//    do
//    {
//        jetsas(1,2,3);
//
//        /**       sensor(S11059,g); ///        sensor(MPU6050,g);
//               LCD_printf(0,0,"%6.1f %6.1f ",g[0],g[1]);
//               LCD_printf(0,1,"%6.1f %6.1f",g[2],g[3]);
//
//               jetsas(1,2,3);
//               send_tocos(j%26);
//               j++;
//        **/
//
//        /*****/
//        urg_start_measurement(&urg, URG_DISTANCE, 1, 0); /// URG sample
//        n = urg_get_distance(&urg, urg_data, &time_stamp);
//        if (n < 0)
//        {
//            printf("urg_get_distance: %s\n", urg_error(&urg));
//            urg_close(&urg);
//            return 1;
//        }
//        for (i = 0; i < n; ++i)
//        {
//            /// printf("i=%d d=%d \n",i,data[i]);
//            cv::Point f=cv::Point(i, 0);
//            cv::line(img, cv::Point(i, 0), cv::Point(i, 500), cv::Scalar(0,0,0), 1, 4);
//            cv::line(img, cv::Point(i, 500), cv::Point(i, 500-urg_data[i]/5), cv::Scalar(200,0,0), 1, 4);
//        }
//        cv::line(img, cv::Point(0, 100), cv::Point(680, 100), cv::Scalar(0,0,200), 1, 4);
//        cv::line(img, cv::Point(0, 200), cv::Point(680, 200), cv::Scalar(0,0,200), 1, 4);
//        cv::line(img, cv::Point(0, 300), cv::Point(680, 300), cv::Scalar(0,0,200), 1, 4);
//        cv::line(img, cv::Point(0, 400), cv::Point(680, 400), cv::Scalar(0,0,200), 1, 4);
//        cv::imshow("URG data", img);
//        cv::waitKey(1);
//        usleep(1000);      // sleep for one millisecond
//        /************/
//        value=gpio_sw(SW1);
//    }
//    while (value==HIGH);
//
//    cout << endl <<  "Button was just pressed!" << endl;
//    cout << "Finished Testing the GPIO Pins" << endl;
//    gpio_close();
//
//    printf("time2 %d[sec] %d[nsec]\n",jt.get_sec(),jt.get_nsec());
////    pthread_join(tid1,NULL);
//    pthread_mutex_destroy(&mutex);
/////    spi_close();
////    urg_close(&urg);

    return 0;
}/****************************************************************** END ***/

