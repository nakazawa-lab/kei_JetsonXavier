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
#include <time.h>

/// 20190911 for speech recognition
#include <boost/regex.hpp>
#include <boost/asio.hpp>

#include"julius_struct.h"

using namespace std;

string line, sourceid, azimuth, cm, word, msec, sec, usec;
float cmscore, direction;
int sid, duration, lineid;
bool recogout = false;

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
boost::regex sec_regex("SEC=\"([^\"]+)\"");
boost::smatch sec_match;
boost::regex usec_regex("USEC=\"([^\"]+)\"");
boost::smatch usec_match;

JuliusResults::JuliusResults()
{
    _julius_result* error_handling;
    results_map[-1] = error_handling;
}

_julius_result* JuliusResults::select(int id)
{
    return results_map.at(id);
}

/// jaddはresults_mapに新しい要素を追加する関数
void JuliusResults::jadd(_julius_result* jr)
{
    results_map[jr->sid] = jr;
}

/******************************** An example of text data sent from Julius ********************************
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
***********************************************************************************************************/

/// jmerge_dataは上記のようなデータが送られてくるので、それを適切にデータに格納する関数
int JuliusResults::jmerge_data(string line)
{
    /// はじめに"SOURCEINFO"を見つけたら_julius_resultオブジェクトを生成
    if (line.find("SOURCEINFO") != string::npos)
    {
        _julius_result* tmp = new _julius_result;

        boost::regex_search(line, sourceid_match, sourceid_regex);
        sourceid = sourceid_match.str(1);
        tmp->sid = atoi(sourceid.c_str());     /// string -> int

        boost::regex_search(line, azimuth_match, azimuth_regex);
        azimuth = azimuth_match.str(1);
        tmp->direction = atof(azimuth.c_str());   /// string -> float

        jadd(tmp);
        return -1;
    }
    /// SOURCEINFOと書かれていないが、SOURCEIDと書かれた行の場合
    /// 例えば、"<INPUTPARAM SOURCEID="83" FRAMES="572" MSEC="5720"/>"
    else if (line.find("SOURCEID") != string::npos)
    {
        boost::regex_search(line, sourceid_match, sourceid_regex);
        sourceid = sourceid_match.str(1);
        lineid = atoi(sourceid.c_str());
        if (line.find("INPUTPARAM") != string::npos)
        {
            boost::regex_search(line, msec_match, msec_regex);
            msec = msec_match.str(1);
            select(lineid)->duration = atoi(msec.c_str());
            return -1;
        }
        else if (line.find("<RECOGOUT") != string::npos)
        {
            recogout = true;
            return -1;
        }
        else if (line.find("RECOGEND") != string::npos)
        {
            boost::regex_search(line, sec_match, sec_regex);
            sec = sec_match.str(1);
            boost::regex_search(line, usec_match, usec_regex);
            usec = usec_match.str(1);
            select(lineid)->endtime_stamp = atof(sec.c_str()) + 0.001 * 0.001 * atof(usec.c_str());

            recogout = false;
            return lineid;
        }
    }
    else if (recogout==true && line.find("CLASSID=\"2\"") != string::npos)
    {
        boost::regex_search(line, cm_match, cm_regex);
        cm = cm_match.str(1);
        select(lineid)->cmscore = atof(cm.c_str());

        boost::regex_search(line, word_match, word_regex);
        select(lineid)->word += word_match.str(1);
        return -1;
    }
    else if (line.find("</RECOGOUT>") != string::npos)
    {
//        recogout = false;
//        return lineid;
        return -1;
    }
    else
    {
        return -2;
    }
}

bool JuliusResults::jinit(string line)
{
    id = jmerge_data(line);
    if (id>=0 && select(id)->word=="MICTEST")
    {
        init_id = id;

        return true;
    }
    else return false;
}

