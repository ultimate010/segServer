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
        public:
            virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd)
            {
                string sentence;
                int xiangxi = 1;
                string val;
                if(!httpReq.GET("sentence", val)){
                    return false;
                }
                //URLDecode(val,sentence);
                sentence = new_UrlDecode(val);
                cout <<"'" <<sentence <<"'" <<endl;
                if(!httpReq.GET("info", val)){
                    return false;
                }
                if(val != "1" && val != "0"){
                    strSnd = "Bad info";
                    return true;
                }
                xiangxi = atoi(val.c_str());
/*
                const char * sResult = NLPIR_ParagraphProcess(sentence.c_str() , xiangxi);
*/
                CNLPIR *pProcessor=GetActiveInstance();
                while(pProcessor->IsAvailable()){
                    //忙等
                }
                const char * sResult = pProcessor->ParagraphProcess(sentence.c_str() , xiangxi);
                pProcessor->SetAvailable();

                val.assign(sResult);
                trim(val);
                cout <<val <<"|" <<endl;
                strSnd = new_UrlEncode(val);
                LogInfo("response '%s'", strSnd.c_str());
                return true;
            }
    };
}

#endif
