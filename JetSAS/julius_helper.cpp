/*************************************************************** K-MAIL ***
* File Name : julius_helper.cpp
* Contents : Jetsas helper for doing with data sent from Julius program 1.00
* Use Module:
* Master CPU1 : Jetson Xavier NVidia
* Slave  CPU2 : R5f72165ADF RENESAS
* Compiler : gcc
* history : 2019.09.20 ver.1.00.00
* Revised : 2019.09.20 ver.1.10.00
***************************** Copyright NAKAZAWA Lab Dept. of SD.keio.jp ***/

#include <stdio.h>
#include <iostream>

/// 20190911 for speech recognition
#include <boost/regex.hpp>
#include <boost/asio.hpp>

#include"julius_struct.h"

using namespace std;

string line, sourceid, azimuth, cm, word, msec;
float cmscore, direction;
int sid, duration, lineid;

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

_julius_result* JuliusResults::select(int id)
{
    for(auto r : vect)
    {
        if (id == r.sid)
        {
            result = r;
            return &result;
        }
    }
    return &julius_err;
}

/// jpush is append _julius_result object to the verctor.
void JuliusResults::jpush(_julius_result* jr)
{
    vect.push_back(*jr);
}

int test_hello()
{
    printf("Hello\n");
    return 0;
}

/*
debug: <STARTPROC/>
debug: <SOURCEINFO SOURCEID="81" AZIMUTH="84.998909" ELEVATION="16.702362" SEC="1569297403" USEC="865066"/>
debug: <STARTRECOG SOURCEID="81"/>
debug: <STARTPROC/>
debug: <SOURCEINFO SOURCEID="83" AZIMUTH="90.000000" ELEVATION="16.702362" SEC="1569297408" USEC="411834"/>
debug: <STARTRECOG SOURCEID="83"/>
debug: <ENDRECOG SOURCEID="83"/>
debug: <INPUTPARAM SOURCEID="83" FRAMES="572" MSEC="5720"/>
debug: <RECOGOUT SOURCEID="83">
debug:   <SHYPO RANK="1" SCORE="4256.807129" GRAM="0">
debug:     <WHYPO WORD="<s>" CLASSID="0" PHONE="silB" CM="1.000"/>
debug:     <WHYPO WORD="come" CLASSID="2" PHONE="k a m u" CM="0.979"/>
debug:     <WHYPO WORD="</s>" CLASSID="1" PHONE="silE" CM="1.000"/>
debug:   </SHYPO>
debug: </RECOGOUT>
WORD=come, SCORE=0.979000, TIME=5720, ID=83, DIRECTION=90.000000
debug: <RECOGEND SOURCEID="83" SEC="1569297414" USEC="591938"/>
debug: <ENDRECOG SOURCEID="81"/>
debug: <INPUTPARAM SOURCEID="81" FRAMES="1022" MSEC="10220"/>
debug: <RECOGOUT SOURCEID="81">
debug:   <SHYPO RANK="1" SCORE="7012.838379" GRAM="0">
debug:     <WHYPO WORD="<s>" CLASSID="0" PHONE="silB" CM="1.000"/>
debug:     <WHYPO WORD="come" CLASSID="2" PHONE="k a m u" CM="0.996"/>
debug:     <WHYPO WORD="</s>" CLASSID="1" PHONE="silE" CM="1.000"/>
debug:   </SHYPO>
debug: </RECOGOUT>
WORD=comecome, SCORE=0.996000, TIME=572010220, ID=83, DIRECTION=90.000000
debug: <RECOGEND SOURCEID="81" SEC="1569297414" USEC="707699"/>
*/
void JuliusResults::insert_julius(string line)
{
    /// はじめに"SOURCEINFO"を見つけたら_julius_resultオブジェクトを生成
    /// sourceidをjr.sidという変数として控えJuliusResultsの要素として新規に保存
    if (line.find("SOURCEINFO") != string::npos)
    {
        _julius_result tmp;

        boost::regex_search(line, sourceid_match, sourceid_regex);
        sourceid = sourceid_match.str(1);
        tmp.sid = atoi(sourceid.c_str());     /// string -> int

        boost::regex_search(line, azimuth_match, azimuth_regex);
        azimuth = azimuth_match.str(1);
        tmp.direction = atof(azimuth.c_str());   /// string -> float

        jpush(&tmp);
    }
    /// これ以降の記述は、生成された_julius_resultオブジェクトにcmscoreなどの要素を追加していくもの
    /// 送られてくる文字列データの中のsourceid(lineidとして保存)と、JuliusResultsがもつsidを照合する
    else if(line.find("SOURCEID") != string::npos)
    {
        boost::regex_search(line, sourceid_match, sourceid_regex);
        sourceid = sourceid_match.str(1);
        lineid = atoi(sourceid.c_str());
        cout << "line; " << select(lineid)->sid << endl;
        _julius_result* jrp = select(lineid);

        if (line.find("INPUTPARAM") != string::npos)
        {
            boost::regex_search(line, msec_match, msec_regex);
            msec += msec_match.str(1);
            cout << "msec: " << msec << ", atoi: " << atoi(msec.c_str()) << endl;
            jrp->duration = atoi(msec.c_str());
            jrp->direction = 3.0;
            cout << "debububug: " << jrp->duration << ", id: " << jrp->sid << ", jrp: " << jrp << endl;
//            jrp->duration = &duration;
//            cout << "debububug: " << jrp->duration << ", id: " << jrp->sid << ", jrp: " << jrp << endl;
        }
//        else if (line.find("CLASSID=\"2\"") != string::npos)
//        {
//            boost::regex_search(line, cm_match, cm_regex);
//            cm = cm_match.str(1);
//            cmscore = atof(cm.c_str());
//
//            boost::regex_search(line, word_match, word_regex);
//            word += word_match.str(1);
//        }
    }

}
