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
    JuliusResults rs;
    _julius_result test;
    test.sid = 1;
    rs.jpush(&test);
    cout << rs.select(1)->sid << endl;

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

    /// ready to fetch texts that you want
    boost::regex score_regex("SCORE=\"([^\"]+)\"");
    boost::smatch score_match;
    boost::regex cm_regex("CM=\"([^\"]+)\"");
    boost::smatch cm_match;
    boost::regex word_regex("WORD=\"([^\"]+)\"");
    boost::smatch word_match;
    boost::regex msec_regex("MSEC=\"([^\"]+)\"");
    boost::smatch msec_match;
    boost::regex sourceid_regex("SOURCEID=\"([^\"]+)\"");
    boost::smatch sourceid_match;
    boost::regex azimuth_regex("AZIMUTH=\"([^\"]+)\"");
    boost::smatch azimuth_match;

//    string line, score, cm, word, reply, msec;
//    string cmnd0, cmnd1, cmnd2, cmfm0, cmfm1;
//    string word0, word1, word2, word3, word4, word5;
//    float cmscore0, cmscore1, cmscore2, cmscore3, cmscore4, cmscore5, rcmscore, cmtime;
//    float abscore0, abscore1, abscore2, abscore3, abscore4, abscore5;
//    float rsltcmscore0, rsltcmscore1, rsltcmscore2, rsltcmscore3, rsltcmscore4, rsltcmscore5;
//    float rsltscore0, rsltscore1, rsltscore2, rsltscore3, rsltscore4, rsltscore5;
//    float dscore = 0;
    string line, sourceid, azimuth, cm, word, msec;
    float cmscore, direction;
    int sid, cmtime;

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

    while (1)
    {
        while (getline(s, line))
        {
            /// This printf function show you all the text date sent from Julius
            printf("debug: %s\n", line.c_str());

            /********************* An example of text data sent from Julius *********************
            > <STARTPROC/>
            > <SOURCEINFO SOURCEID="351" AZIMUTH="-80.001717" ELEVATION="16.702362" SEC="1568966750" USEC="466169"/>
            > <STARTRECOG SOURCEID="351"/>
            > <ENDRECOG SOURCEID="351"/>
            > <INPUTPARAM SOURCEID="351" FRAMES="272" MSEC="2720"/>
            > <RECOGOUT SOURCEID="351">
            >   <SHYPO RANK="1" SCORE="2123.848389" GRAM="0">
            >     <WHYPO WORD="<s>" CLASSID="0" PHONE="silB" CM="1.000"/>
            >     <WHYPO WORD="come" CLASSID="2" PHONE="k a m u" CM="0.373"/>
            >     <WHYPO WORD="</s>" CLASSID="1" PHONE="silE" CM="1.000"/>
            >   </SHYPO>
            > </RECOGOUT>
            > <RECOGEND SOURCEID="351" SEC="1568966753" USEC="334428"/>
            ********************************************************************************/

            if (line.find("STARTPROC") != string::npos)
            {
                cm = "";
                word = "";
                msec = "";
                sourceid = "";
                azimuth = "";
            }
            else if (line.find("SOURCEINFO") != string::npos)
            {
                boost::regex_search(line, sourceid_match, sourceid_regex);
                sourceid = sourceid_match.str(1);
                sid = atoi(sourceid.c_str());     /// string -> int

                boost::regex_search(line, azimuth_match, azimuth_regex);
                azimuth = azimuth_match.str(1);
                direction = atof(azimuth.c_str());
            }
            else if (line.find("INPUTPARAM") != string::npos)
            {
                boost::regex_search(line, msec_match, msec_regex);
                msec += msec_match.str(1);
                cmtime = atoi(msec.c_str());    /// string -> float
            }
            else if (line.find("CLASSID=\"2\"") != string::npos)
            {
                boost::regex_search(line, cm_match, cm_regex);
                cm = cm_match.str(1);
                cmscore = atof(cm.c_str());     /// string -> float

                boost::regex_search(line, word_match, word_regex);
                word += word_match.str(1);
            }
            else if (line.find("/RECOGOUT") != string::npos)
            {
                printf("WORD=%s, SCORE=%f, TIME=%d, ID=%d, DIRECTION=%f\n",
                       word.c_str(), cmscore, cmtime, sid, direction);
                break;
            }
        }
    }
    return 0;
}/****************************************************************** END ***/

