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

#ifndef _JULIUS_STRUCT_H_
#define _JULIUS_STRUCT_H_

using namespace std;

//word.c_str(), cmscore, cmtime, sid, direction
/// _julius_result is each result for utterance
struct _julius_result
{
    int sid, duration;
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
        _julius_result* select(int id);
        void jpush(_julius_result* jr);
};

_julius_result* JuliusResults::select(int id)
{
    for(auto r : vect){
       if (id == r.sid){
            result = r;
            return &result;
       }
    }
    return &julius_err;
}

void JuliusResults::jpush(_julius_result* jr)
{
    vect.push_back(*jr);
}


#endif // _JULIUS_STRUCT_H_
