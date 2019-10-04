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

float theta0, r0;

char* exec_cmd(string cmd, pthread_t tid, string* mode_change_to);
float talk(char* s);
int get_obs(float *prm1, float *prm2);

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

    JuliusResults jrs;
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

    bool jinit_done = false;
//    while (getline(s, line))
//    {
//        AQUES_Talk("maikunotesuto");
//        jinit_done = jrs.jinit(line);
//        sleep(5);
//        if (jinit_done==true) break;
//    }

    /******************************************************** end ***/

    /// NakBotに関する変数の定義
    string mode = "normal";
    char* resp;
    bool valid_flag;
    int cmd_id;
    float said_stamp;
    while (1)
    {
        while (getline(s, line))
        {
            /// ここをコメントアウトすると入力文字列lineの中身を見れる
//            cout << line << endl;
            cmd_id = jrs.jmerge_data(line);
            if (cmd_id >= 0)
            {
                jrs.emit_log(cmd_id, "console");
                break;
            }
        }

        ///　しきい値処理
//        if (is_nakbot_voice(jrs.select(cmd_id), jrs.select(jrs.init_id), said_stamp)==true) valid_flag = false;
//        else valid_flag = threshold_turning(jrs.select(cmd_id));

        valid_flag = threshold_turning(jrs.select(cmd_id));
//
        /// 命令の実行
        if (mode=="sleep")
        {
                if(jrs.select(cmd_id)->word == "yes")
                {
                    mode = "normal";
//                    say_sleep();
//                    alutSleep(0.7);
//                    s << "DIE\n" << std::flush;
//                    return 0;
                }
//                else
//                {
//                    s << "PAUSE\n" << std::flush;
//                    say_ready();   /// "hello, master. i'm ready."
//                    s << "RESUME\n" << std::flush;
//                    state=0;
//                }
        }
        else if (mode == "stay")
        {
            if(jrs.select(cmd_id)->word == "ok")
            {
                mode = "normal";
//                s << "PAUSE\n" << std::flush;
//                say_ready();   /// "hello, master. i'm ready."
//                s << "RESUME\n" << std::flush;
//                state=0;
            }
        }
        else if (mode == "normal")
        {
            if (valid_flag==true)
            {
                resp = exec_cmd(jrs.select(cmd_id)->word, tid2, &mode);
//                AQUES_Talk(resp);
//                said_stamp = talk(resp);
            }
        }
    }
    return 0;
}/****************************************************************** END ***/

char* exec_cmd(string cmd, pthread_t tid, string* mode_change_to)
{
    if (cmd=="right")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_right, NULL);
        return "raito";
    }
    else if (cmd=="left")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_left, NULL);
        return "rehuto";
    }
    else if (cmd=="go")
    {
        if (get_obs(&theta0, &r0)==0)
        {
            pthread_cancel(tid);
            pthread_create(&tid, NULL, cmd_go, NULL);
            return "go";
        }
        else
        {
            pthread_cancel(tid);
            pthread_create(&tid, NULL, cmd_stop, NULL);
            return "dekimasenn";
        }
    }
    else if (cmd=="back")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_back, NULL);
        return "bakku";
    }
    else if (cmd=="stop")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_stop, NULL);
        return "sutoppu";
    }
    else if (cmd=="come")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_come, NULL);
        return "camu";
    }
    else if (cmd=="push")
    {
        if (get_obs(&theta0, &r0)==1)
        {
            pthread_cancel(tid);
            pthread_create(&tid, NULL, cmd_look, &theta0);
            if(abs(theta0)<=70)
            {
                pthread_cancel(tid);
                pthread_create(&tid, NULL, cmd_push, &r0);
            }
            return "pushu";
        }
        else
        {
            pthread_cancel(tid);
            pthread_create(&tid, NULL, cmd_stop, NULL);
            return "dekimasenn";
        }
    }
    else if (cmd=="stay")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_stop, NULL);
        *mode_change_to = "stay";
        return "sutei";
    }
    else if (cmd=="sleep")
    {
        pthread_cancel(tid);
        pthread_create(&tid, NULL, cmd_stop, NULL);
        *mode_change_to = "sleep";
        return "suriipu";
    }
}

float talk(char* s)
{
    AQUES_Talk(s);

    struct timeval _time;
    gettimeofday(&_time, NULL);
    /// たぶんタイムスタンプ
    long sec = _time.tv_sec;
    //マイクロ秒　0から999999までの値を取り、一杯になると秒がカウントアップされていく
    long usec = _time.tv_usec;

    float said_stamp = (float)sec + 0.001 * 0.001 * (float)usec;
    return said_stamp;
}

int get_obs(float *prm1, float *prm2)   /// 衝突判定
{
    int state=0;
    long border_r=200;  /// 距離のしきい値[mm]
    long border_rmax=500;  /// 最大距離のしきい値[mm]
    long x, y, x1, y1, x2, y2;
    long r_min=1000;
    long r_max=0;
    float phi, theta, x0, y0, r0;    /// x0,y0 = 目標地点

    long max_distance;///for URG sensor
    long min_distance;
    long time_stamp;
    int i, n;

    urg_start_measurement(&urg, URG_DISTANCE, 1, 0); /// URG sample
    n = urg_get_distance(&urg, urg_data, &time_stamp);
    if (n < 0)
    {
        printf("urg_get_distance: %s\n", urg_error(&urg));
//        urg_close(&urg);
        return 0;
    }

    urg_distance_min_max(&urg, &min_distance, &max_distance);
//    for (i = 0; i < n; i+=2)    /// 0 <= i <= 680, -120deg ~ 120deg
    for (i = 84; i < 597; i+=2)    /// 90deg ~ -90deg
    {
        long distance = urg_data[i];
        double radian;

        if ((distance < min_distance) || (distance > max_distance)) continue;

        radian =- urg_index2rad(&urg, i);   /// なんか逆になってるよ。
        x=(distance * sin(radian));
        y=(distance * cos(radian)-100);
//        y=(distance * cos(radian));
//        cout<<"the= "<<radian/M_PI*180<<endl;
//        cout<<"(x,y)= "<<x<<", "<<y<<endl;
//        cout<<"r= "<<distance<<endl;

        if(distance < border_r) state=1;
        if(distance < border_rmax && distance<r_min)
        {
            r_min=distance;
            x1=x;
            y1=y;
        }
        if(distance<border_rmax && distance>r_max)
        {
            r_max=distance;
            x2=x;
            y2=y;
        }
    }
//	urg_close(&urg);

    phi=atan((float)(y2-y1)/(x2-x1))/M_PI*180;
    if (phi>=0) theta=90-phi;
    else theta=-90-phi;
    x0=(float)(x2*x2-x1*x1+y2*y2-y1*y1)*(x2-x1)/(2*(x2-x1)*(x2-x1)+2*(y2-y1)*(y2-y1));
    y0=(float)(x2*x2-x1*x1+y2*y2-y1*y1)*(y2-y1)/(2*(x2-x1)*(x2-x1)+2*(y2-y1)*(y2-y1));
    r0=(float)sqrt(x0*x0+y0*y0);
//    cout<<"(x1,y1)= "<<x1<<", "<<y1<<"(x2,y2)"<<x2<<", "<<y2<<endl;
//    cout<<"(x0,y0)= "<<x0<<", "<<y0<<endl;
    cout<<"phi= "<<phi<<", theta= "<<theta<<endl;
    cout<<"r= "<<r0<<endl;
    *prm1=theta;
    if(theta >= 0) *prm2=r0/1000;
    else *prm2=-r0/1000;

    if (state ==1) return 1;
    else return 0;
}
