/*************************************************************** K-MAIL ***
* File Name ) exec_cmd.cpp
* Contents ) JetSAS main program 1.00
* Use Module)
* Master CPU1 ) Jetson Xavier NVidia
* Slave  CPU2 ) R5f72165ADF RENESAS
* Compiler ) gcc
* history ) 2019.09.27 ver.1.00.00
* Revised ) 2019.09.27 ver.1.10.00
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>///
#include "open_urg_sensor.h"

#include "julius_struct.h"
#include "jetsas.h"

using namespace std;

int get_obs(float *prm1, float *prm2);
int deg(float prm);
int meter(float prm);

float en_reso=5.95;      /// エンコーダの値（旋回）
float en_meter=10.95;   /// エンコーダの値（並進）
float al_delay=0.2;     /// 固定マイクの場合

bool threshold_turning(_julius_result* jr)
{
    if (jr->word=="yes" || jr->word=="ok" || jr->word=="stop") return true;
    else if (800 < jr->duration && jr->duration < 1150)
    {
        if (jr->word=="go" || jr->word=="push")
        {
            if (jr->cmscore > 0.5) return true;
        }
        else
        {
            if(jr->cmscore > 0.6) return true;
        }
    }
    else return false;
}

int time1, rot1, rot2, time2;
int i_time1, i_rot1, i_rot2, i_time2;
int ret;
float null;
//
//
///********************************************************** get duration ***/
//float getDuration(ALuint buffer)    /// 再生時間[秒]を返します
//{
//    ALint al_size, al_frequency, al_channels, al_bits;
//
//    alGetBufferi(buffer, AL_SIZE, &al_size);
//    alGetBufferi(buffer, AL_FREQUENCY, &al_frequency);
//    alGetBufferi(buffer, AL_CHANNELS, &al_channels);
//    alGetBufferi(buffer, AL_BITS, &al_bits);
//
//    return (float)al_size/(float)(al_frequency*al_channels*(al_bits/8));
//}/****************************************************************** END ***/
//

///******************************************************** static command ***/
void* cmd_right(void* pParam)
{
    get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
    rot1 = i_rot1;                                      /// エンコーダ初期値
    rot2 = i_rot2;

    jetsas0('m',0001,0000); /// motor_on=1
    jetsas0('v',5050,4950);
    while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(180))
    {
        get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
    }
    jetsas0('v',5000,5000);
}
void* cmd_left(void* pParam)
{
    get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
    rot1 = i_rot1;                                      /// エンコーダ初期値
    rot2 = i_rot2;

    jetsas0('m',0001,0000); /// motor_on=1
    jetsas0('v',4950,5050);
    while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(180))
    {
        get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
    }
    jetsas0('v',5000,5000);
}
void* cmd_stop(void* pParam)
{
    jetsas0('m',0001,0000); /// motor_on=1
    jetsas0('v',5000,5000);
}/****************************************************************** END ***/

/******************************************************* dynamic command ***/
void* cmd_go(void* pParam)
{
    jetsas0('m',0001,0000); /// motor_on=1
    jetsas0('v',5200,5200);
    while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(1000))
    {
        if(get_obs(&null, &null)>0) break;              /// ぶつかる手前まで進む
        get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
    }
    jetsas0('v',5000,5000);
}
void* cmd_back(void* pParam)
{
    get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
    rot1 = i_rot1;                                      /// エンコーダ初期値
    rot2 = i_rot2;

    jetsas0('m',0001,0000); /// motor_on=1
    jetsas0('v',4900,4900);
    while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(0.3))
    {
        get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
    }
    jetsas0('v',5000,5000);
}
void* cmd_push(void* pParam)
{
    float *Param = (float *)pParam;
    float meter_f =*Param;

    if(meter_f>=0)
    {
        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('m',0001,0000); /// motor_on=1
        jetsas0('v',5200,5200);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(meter_f))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
//        jetsas0('v',5000,5000);

        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('v',4900,5100);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(90))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
//        jetsas0('v',5000,5000);

        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('v',5050,5050);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(0.3))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
        jetsas0('v',5000,5000);
    }
    else
    {
        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('m',0001,0000); /// motor_on=1
        jetsas0('v',5200,5200);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(-meter_f))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
//        jetsas0('v',5000,5000);

        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('v',5100,4900);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(90))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
//        jetsas0('v',5000,5000);

        jetsas0('v',5050,5050);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(0.3))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
        jetsas0('v',5000,5000);
    }
}
void* cmd_look(void* pParam)
{
    float *Param = (float *)pParam;
    float theta =*Param;

    if(abs(theta)>70)
    {
        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
        rot1 = i_rot1;                                      /// エンコーダ初期値
        rot2 = i_rot2;

        jetsas0('m',0001,0000); /// motor_on=1
        jetsas0('v',5050,5050);
        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(0.3))
        {
            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
        }
        jetsas0('v',5000,5000);
    }
    else
    {
        if(theta>0)   /// θ>0 の時は右旋回
        {
            get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
            rot1 = i_rot1;                                      /// エンコーダ初期値
            rot2 = i_rot2;

            jetsas0('m',0001,0000); /// motor_on=1
            jetsas0('v',5100,4900);
            while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(theta))
            {
                get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
            }
        }
        else                    /// θ<0 の時は左旋回
        {
            get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
            rot1 = i_rot1;                                      /// エンコーダ初期値
            rot2 = i_rot2;

            jetsas0('m',0001,0000); /// motor_on=1
            jetsas0('v',4900,5100);
            while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(theta))
            {
                get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
            }
        }
    }
}
void* cmd_come(void* pParam)
{
//    float doa_f = doa;
//    if(doa_f>0)   /// θ>0 の時は右旋回
//    {
//        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
//        rot1 = i_rot1;                                      /// エンコーダ初期値
//        rot2 = i_rot2;
//
//        jetsas0('m',0001,0000); /// motor_on=1
//        jetsas0('v',5100,4900);
//        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(doa_f))
//        {
//            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
//        }
//        jetsas0('v',5000,5000);
//
//        jetsas0('v',5200,5200);
//        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(1000))
//        {
//            if(get_obs(&null, &null)>0) break;              /// ぶつかる手前まで進む
//            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
//        }
//        jetsas0('v',5000,5000);
//    }
//    else                    /// θ<0 の時は左旋回
//    {
//        get_encoder(&i_time1, &i_rot1, &i_rot2, &i_time2);  /// (時間、左車輪、右車輪、時間)
//        rot1 = i_rot1;                                      /// エンコーダ初期値
//        rot2 = i_rot2;
//
//        jetsas0('m',0001,0000); /// motor_on=1
//        jetsas0('v',4900,5100);
//        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < deg(doa_f))
//        {
//            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
//        }
//        jetsas0('v',5000,5000);
//
//        jetsas0('v',5200,5200);
//        while(abs(rot1-i_rot1)+abs(rot2-i_rot2) < meter(1000))
//        {
//            if(get_obs(&null, &null)>0) break;              /// ぶつかる手前まで進む
//            get_encoder(&time1, &rot1, &rot2, &time2);      /// (時間、左車輪、右車輪、時間)
//        }
//        jetsas0('v',5000,5000);
//    }
}/****************************************************************** END ***/

/*************************************************** get encoder command ***/
void get_encoder(int *time1, int *prm1, int *prm2, int *time2)  /// (時間、左車輪、右車輪、時間)
{
    char r, t1[8]= {0}, p1[8]= {0}, p2[8]= {0}, t2[8]= {0};
    int i;

    jetsas0('e',0001,0001); /// encoder info.

    while(1)    /// t,l,r,t
    {
        receive0(&r);
        if(r=='\n') break;
    }
    for(int i=0; i<8; i++)    /// time2
    {
        receive0(&r);
        if(r==' ') break;
        t2[i]=r;
    }
    for(int i=0; i<8; i++)    /// rot2
    {
        receive0(&r);
        if(r==' ') break;
        p2[i]=r;
    }
    for(int i=0; i<8; i++)    /// rot1
    {
        receive0(&r);
        if(r==' ') break;
        p1[i]=r;
    }
    for(int i=0; i<8; i++)    /// time1
    {
        receive0(&r);
        if(r==' ') break;
        t1[i]=r;
    }
    while(1)                 /// 終了待機
    {
        receive0(&r);
        if(r=='\n') break;
    }
    *time1  = atoi(t1);         /// string -> int
    *prm1   = 5000000-atoi(p1); /// 中心値からの変化量に直す。プラスが正転（前進回転）
    *prm2   = atoi(p2)-5000000;
    *time2  = atoi(t2);
    cout<<"time="<<*time1<<", left="<<*prm1<<", right="<<*prm2<<", time="<<*time2<<endl;

    usleep(8300);    /// wait for time_out
}/****************************************************************** END ***/

int deg(float prm)  /// prm ) 旋回する角度[deg]
{
    int ret;
    ret=(int)en_reso*abs(prm);
    return ret;
}

int meter(float prm)  /// prm ) 進む距離[m]
{
    int ret;
    ret=(int)en_meter*prm*100*2;
    return ret;
}

//int dft() /// HARKを使う以前の音源定位に関する記述
//{
//    int i,NFFT;
//    float dummy[101];
//
//    NFFT=cvGetOptimalDFTSize(sizeof(dummy)/sizeof(float));
//
//    CvMat *m1 = cvCreateMat (1, NFFT, CV_32FC1);
//    CvMat *mm1 = cvCreateMat (1, NFFT, CV_32FC1);
//    CvMat *M1 = cvCreateMat (1, NFFT, CV_32FC2);
//    CvMat *M1_re = cvCreateMat (1, NFFT, CV_32FC1);
//    CvMat *M1_im = cvCreateMat (1, NFFT, CV_32FC1);
//
//    for(i=0; i<NFFT; i++){
//        cvmSet(m1, 0, i, 2);
//    }
//    for(i=0; i<NFFT; i++){
//        cout<<cvmGet(m1, 0, i)<<" ";
//    }
//    cout<<endl;
//
//    cvDFT(m1,M1, CV_DXT_FORWARD);   /// フーリエ変換
//    cvSplit (M1, M1_re, M1_im, 0, 0);
//
//    for(i=0; i<NFFT; i++){
//        cout<<cvmGet(M1_re, 0, i)<<" ";
//    }
//    cout<<endl;
//    for(i=0; i<NFFT; i++){
//        cout<<cvmGet(M1_im, 0, i)<<" ";
//    }
//    cout<<endl;
//
//    cvDFT(M1,mm1,CV_DXT_INV_SCALE); /// フーリエ逆変換
//    for(i=0; i<NFFT; i++){
//        cout<<cvmGet(mm1, 0, i)<<" ";
//    }
//    cout<<endl;
//
//    return 0;
//}

/** 時間計測用
#include <time.h>
clock_t c_start, c_end;
c_start=clock();
c_end=clock();
cout<<(float)(c_end-c_start)/CLOCKS_PER_SEC<<endl;
*/
