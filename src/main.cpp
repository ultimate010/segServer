
#include "NLPIR.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "Daemon.hpp"
#include "ReqHandler.hpp"
#include "Config.hpp"

using namespace Husky;
using namespace segServer;
using namespace Limonp;


/*
void SplitGBK(const char *sInput);
void SplitUTF8();

void testKeyWord()
{

    const char * sKeyword= NLPIR_GetKeyWords("������˹����X3Q רҵ�Խ���",10,true);
    printf("%s\n",sKeyword);
    sKeyword= NLPIR_GetKeyWords("MOTOROLA/Ħ������ Ħ�������Խ��� MAG ONE A8�Խ��� רҵ��ˤ������̨ ����/�Ƶ�/��λ/��ҵ",10,true);
    printf("%s\n",sKeyword);

}
void testSplit()
{

    NLPIR_SetPOSmap(PKU_POS_MAP_SECOND);//���ô��Ա�ע���ϵ����ͣ�Ĭ��Ϊ������������ע��ϵ

    char sSentence[2048] = {0};

    strcpy(sSentence,"������Ƶ��ݸ��������");
    const char * sResult = NLPIR_ParagraphProcess(sSentence, 1);
    printf("%s\n", sResult);
    printf("��̬����û��ʺ�\n");
    NLPIR_AddUserWord("����   ag");
    sResult = NLPIR_ParagraphProcess(sSentence, 1);
    printf("%s\n", sResult);

}



bool myiniSeg(int encode){
    //��ʼ���ִ����
    if(!NLPIR_Init("./",encode))//�����ڵ�ǰ·���£�Ĭ��ΪGBK����ķִ�
    {
        return false;
    }else{
        return true;
    }
}

bool releaseSeg(){
    NLPIR_Exit();
    return true;
}
*/

bool server(const Config& conf, const char * key)
{
    uint port = 0;
    uint threadNum = 0;
    int encode = GBK_CODE;
    string pidFile;
    string val;
    string userDict = "";
    if(!conf.get("port", val))
    {
        LogFatal("conf get port failed.");
        return false;
    }
    port = atoi(val.c_str());
    if(!conf.get("encode", val))
    {
        LogFatal("conf get encode failed.");
        return false;
    }
    encode = atoi(val.c_str());
    if(!conf.get("thread_num", val))
    {
        LogFatal("conf get thread_num failed.");
        return false;
    }
    threadNum = atoi(val.c_str());
    if(!conf.get("pid_file", pidFile))
    {
        LogFatal("conf get pid_file failed.");
        return false;
    }
    
    if(!conf.get("dict", userDict)){}


    ReqHandler reqHandler;
    if(!reqHandler.iniSeg(encode)){
        perror("ICTCLAS INIT FAILED!\n");
        exit(-1);
    }
    if(userDict != ""){
        reqHandler.loadUserDict(userDict.c_str()); //load user dict 
    }
    HuskyServer sf(port, threadNum, &reqHandler);
    //sf.init();
    //sf.run();
    Daemon daemon(&sf, pidFile.c_str());
    if(!strcmp("start", key))
    {
        return daemon.start();
    }
    else if(!strcmp("stop", key))
    {
        return daemon.stop();
    }
    return false;
}

int main(int argc,char *argv[])
{
    /*
    if(!iniSeg(GBK_CODE)){
        perror("ICTCLAS INIT FAILED!\n");
        exit(-1);
    }

    testKeyWord();
    testSplit();

    releaseSeg();
    return 0;
    */
    if(argc < 3)
    {
        printf("usage:\n\t%s  <config_file> <start|stop>\n", argv[0]);
        return EXIT_FAILURE;
    }
    Config conf(argv[1]);
    return !server(conf,argv[2]);
}


