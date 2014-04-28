#ifndef WOODPECKER_REQHANDLER_HPP
#define WOODPECKER_REQHANDLER_HPP

#include <fstream>
#include "NLPIR.h"
#include "HuskyServer.hpp"

namespace segServer
{
    using namespace Husky;

    class ReqHandler: public IRequestHandler
    {
        public:
            ReqHandler(){}
            virtual ~ReqHandler(){
                NLPIR_Exit();
            }
            bool iniSeg(const int & encode){
                //初始化分词组件
                if(!NLPIR_Init("./",encode))//数据在当前路径下，默认为GBK编码的分词
                {
                    return false;
                }else{
                    return true;
                }
            }
            bool loadUserDict(const char * pfile){
                ifstream input(pfile);
                if(input.fail()){
                    return false;
                }
                string line;
                while(!input.eof()){
                    getline(input , line);
                    if(0 == line.size()){
                        continue;
                    }
                    NLPIR_AddUserWord(line.c_str());
                }
                return true;
            }
        public:
            virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd)
            {
                string sentence;
                int xiangxi = 1,keyword = 1;
                string val;
                if(!httpReq.GET("sentence", val)){
                    strSnd = "Please input sentence";
                    strSnd = new_UrlEncode(strSnd);
                    return true;
                }
                //URLDecode(val,sentence);
                sentence = new_UrlDecode(val);
                cout <<endl <<"Req:'" <<sentence <<"'" <<endl;
                if(!httpReq.GET("info", val)){
                }
                if(val != "1" && val != "0"){
                    val = "1";
                }
                xiangxi = atoi(val.c_str());
                if(!httpReq.GET("keyword", val)){
                }
                if(val != "1" && val != "0"){
                    val = "1";
                }
                keyword = atoi(val.c_str());
                //const char * sResult = NLPIR_ParagraphProcess(sentence.c_str() , xiangxi);
                CNLPIR *pProcessor=GetActiveInstance();
                while(pProcessor->IsAvailable()){
                    //忙等
                }
                const char * sResult = pProcessor->ParagraphProcess(sentence.c_str() , xiangxi);
                val.assign(sResult);
                if(keyword == 1){
                    sResult = pProcessor->GetKeyWords(sentence.c_str(), 20 ,xiangxi?true:false);
                }
                pProcessor->SetAvailable();

                trim(val);
                if(keyword == 1){
                    val = val + "\nkeyword: " + string(sResult);
                    trim(val);
                }
                cout <<"Rep:'"  <<val <<"'" <<endl;
                strSnd = new_UrlEncode(val);
                LogInfo("response '%s'", strSnd.c_str());
                return true;
            }
    };
}

#endif
