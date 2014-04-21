
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

    const char * sKeyword= NLPIR_GetKeyWords("雷曼克斯骁龙X3Q 专业对讲机",10,true);
    printf("%s\n",sKeyword);
    sKeyword= NLPIR_GetKeyWords("MOTOROLA/摩托罗拉 摩托罗拉对讲机 MAG ONE A8对讲机 专业耐摔大功率手台 工地/酒店/单位/物业",10,true);
    printf("%s\n",sKeyword);

}
void testSplit()
{

    NLPIR_SetPOSmap(PKU_POS_MAP_SECOND);//设置词性标注集合的类型，默认为计算所二级标注体系

    char sSentence[2048] = {0};

    strcpy(sSentence,"不雅视频东莞聚众淫乱");
    const char * sResult = NLPIR_ParagraphProcess(sSentence, 1);
    printf("%s\n", sResult);
    printf("动态添加用户词后：\n");
    NLPIR_AddUserWord("不雅   ag");
    sResult = NLPIR_ParagraphProcess(sSentence, 1);
    printf("%s\n", sResult);

}



bool myiniSeg(int encode){
    //初始化分词组件
    if(!NLPIR_Init("./",encode))//数据在当前路径下，默认为GBK编码的分词
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


