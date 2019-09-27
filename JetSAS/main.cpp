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

#include <pthread.h>///並列処理

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

/// 20190921 for fetched data
#include "julius_struct.h"

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
    JuliusResults jrs;
//    _julius_result test;
//    test.sid = 1;
//    jrs.jpush(&test);
//    cout << jrs.select(1)->sid << endl;

    ///
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

    string line;
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

    /// NakBotに関する変数の定義
    string mode = "normal";
    string resp;
    while (1)
    {
        while (getline(s, line))
        {
            int cmd_id = jrs.jmerge_data(line);
            if (cmd_id != -1)
            {
                cout << "id: " << jrs.select(cmd_id)->sid << " ,direction: " << jrs.select(cmd_id)->direction << " ,cmscore: " << jrs.select(cmd_id)->cmscore << endl;
                break;
            }
        }

        /// 命令の認識
        switch(mode)
        {
        case "sleep":
//                if(word == "yes")
//                {
//                    say_sleep();
//                    alutSleep(0.7);
//                    s << "DIE\n" << std::flush;
//                    return 0;
//                }
//                else
//                {
//                    s << "PAUSE\n" << std::flush;
//                    say_ready();   /// "hello, master. i'm ready."
//                    s << "RESUME\n" << std::flush;
//                    state=0;
//                }
        case "stay":
//            if(word == "ok")
//            {
//                s << "PAUSE\n" << std::flush;
//                say_ready();   /// "hello, master. i'm ready."
//                s << "RESUME\n" << std::flush;
//                state=0;
//            }
        case "normal":
            string threshold = threshold_turning(jrs.select(cmd_id)->word);

            if (jrs.select(cmd_id)->cmscore > threshold)
            {
                resp = exec_cmd(jrs.select(cmd_id)->word, tid2);
//                AQUES_Talk(resp);
            }


//            switch(jrs.select(cmd_id)->word)
//            {
//                exec
//            case "right":
//
//            case "left":
//            case "go":
//            case "back":
//            case "stop":
//            case "come":
//            case "push":
//            case "stay":
//            case "sleep":
        }
    }
//        if (state==1)   /// sleepの確認
//        {
//            if(word == "yes")
//            {
//                say_sleep();
//                alutSleep(0.7);
//                s << "DIE\n" << std::flush;
//                return 0;
//            }
//            else
//            {
//                s << "PAUSE\n" << std::flush;
//                say_ready();   /// "hello, master. i'm ready."
//                s << "RESUME\n" << std::flush;
//                state=0;
//            }
//        }
//        else if (state==2)   /// stayの確認
//        {
//            if(word == "ok")
//            {
//                s << "PAUSE\n" << std::flush;
//                say_ready();   /// "hello, master. i'm ready."
//                s << "RESUME\n" << std::flush;
//                state=0;
//            }
//        }
//        else if (word == "stop")
//        {
//            pthread_cancel(tid2);
//            pthread_create(&tid2, NULL, cmd_stop, NULL);
//            s << "PAUSE\n" << std::flush;
//            say_stop();
//            s << "RESUME\n" << std::flush;
//        }
//        else if (800 < cmtime && cmtime < 1150)
//        {
//            if (word == "right" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_right, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_right();
//                s << "RESUME\n" << std::flush;
//            }
//            else if (word == "left" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_left, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_left();
//                s << "RESUME\n" << std::flush;
//            }
//            else if (word == "go" && cmscore > 0.5)
////            else if (word == "go" && cmscore > border1)
//            {
//                if (get_obs(&theta0, &r0)==0)
//                {
//                    pthread_cancel(tid2);
//                    pthread_create(&tid2, NULL, cmd_go, NULL);
//                    s << "PAUSE\n" << std::flush;
//                    say_go();
//                    s << "RESUME\n" << std::flush;
//                }
//                else
//                {
//                    pthread_cancel(tid2);
//                    pthread_create(&tid2, NULL, cmd_stop, NULL);
//                    s << "PAUSE\n" << std::flush;
//                    say_no();
//                    s << "RESUME\n" << std::flush;
//                }
//            }
//            else if (word == "back" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_back, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_back();
//                s << "RESUME\n" << std::flush;
//            }
//            else if (word == "come" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_come, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_come();
//                s << "RESUME\n" << std::flush;
//            }
//            else if (word == "push" && cmscore > 0.5)
//            {
//                if (get_obs(&theta0, &r0)==1)
//                {
//                    pthread_cancel(tid2);
//                    pthread_create(&tid2, NULL, cmd_look, &theta0);
//                    s << "PAUSE\n" << std::flush;
//                    say_push();
//                    s << "RESUME\n" << std::flush;
//                    if(abs(theta0)<=70)
//                    {
//                        pthread_cancel(tid2);
//                        pthread_create(&tid2, NULL, cmd_push, &r0);
//                    }
//                }
//                else
//                {
//                    pthread_cancel(tid2);
//                    pthread_create(&tid2, NULL, cmd_stop, NULL);
//                    s << "PAUSE\n" << std::flush;
//                    say_no();
//                    s << "RESUME\n" << std::flush;
//                }
//            }
//            else if (word == "stay" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_stop, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_stay();
//                s << "RESUME\n" << std::flush;
//                state = 2;
//            }
//            else if (word == "sleep" && cmscore > border1)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_stop, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_comfirm();
//                s << "RESUME\n" << std::flush;
//                state = 1;
//            }/*
//            else if (cmscore > border0)
//            {
//                pthread_cancel(tid2);
//                pthread_create(&tid2, NULL, cmd_stop, NULL);
//                s << "PAUSE\n" << std::flush;
//                say_pardon();
//                s << "RESUME\n" << std::flush;
//            }*/
//        }

    return 0;
}/****************************************************************** END ***/

