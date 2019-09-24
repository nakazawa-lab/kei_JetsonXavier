/*************************************************************** K-MAIL ***
* File Name : julius_struct.h
* Contents : Julius data header file 1.00
* Use Module:
* Master CPU1 : Jetson TX2 NVidia
* Slave  CPU2 : R5f72165ADF RENESAS
* Compiler : gcc
* history : 2019.09.21 ver.1.00.00
* Revised : ver.1.00.00
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/

// refer to http://www.kinjo-u.ac.jp/ushida/tekkotsu/cpp/ifndef.html
//#ifndef _JULIUS_STRUCT_H_
//#define _JULIUS_STRUCT_H_

#ifndef JULIUS_STRUCT_H_
#define JULIUS_STRUCT_H_

using namespace std;

/// _julius_result is each result for utterance
struct _julius_result
{
    int sid;
    int duration;
    string word;
    float cmscore, direction;
};

class JuliusResults
{
    private:
        int id;
        _julius_result julius_err, result;
        vector<_julius_result> vect;
    public:
        /// refer to julius_helper.cpp
        _julius_result* select(int id);
        void jpush(_julius_result* jr);
        void insert_julius(string line);
};

#endif /* JULIUS_STRUCT_H_*/
