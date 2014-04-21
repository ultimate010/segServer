#ifndef HUSKY_HTTP_REQINFO_H
#define HUSKY_HTTP_REQINFO_H

#include <iostream>
#include <string>
#include "logger.hpp"
#include "Config.hpp"

namespace Husky
{
    using namespace Limonp;
    static const char* const KEY_METHOD = "METHOD";
    static const char* const KEY_PATH = "PATH";
    static const char* const KEY_PROTOCOL = "PROTOCOL";

    inline std::string new_UrlEncode(const std::string& szToEncode)
    {
        std::string src = szToEncode;
        char hex[] = "0123456789ABCDEF";
        string dst;

        for (size_t i = 0; i < src.size(); ++i)
        {
            unsigned char cc = src[i];
            if (isascii(cc))
            {
                if (cc == ' ')
                {
                    dst += "%20";
                }
                else
                    dst += cc;
            }
            else
            {
                unsigned char c = static_cast<unsigned char>(src[i]);
                dst += '%';
                dst += hex[c / 16];
                dst += hex[c % 16];
            }
        }
        return dst;
    }


    inline std::string new_UrlDecode(const std::string& szToDecode)
    {
        std::string result;
        int hex = 0;
        for (size_t i = 0; i < szToDecode.length(); ++i)
        {
            switch (szToDecode[i])
            {
                case '+':
                    result += ' ';
                    break;
                case '%':
                    if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
                    {
                        std::string hexStr = szToDecode.substr(i + 1, 2);
                        hex = strtol(hexStr.c_str(), 0, 16);
                        //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
                        //可以不经过编码直接用于URL
                        if (!((hex >= 48 && hex <= 57) ||   //0-9
                                    (hex >=97 && hex <= 122) || //a-z
                                    (hex >=65 && hex <= 90) ||  //A-Z
                                    //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@]
                                    hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
                                    || hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
                                    || hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
                             ))
                        {
                            result += char(hex);
                            i += 2;
                        }
                        else result += '%';
                    }else {
                        result += '%';
                    }
                    break;
                default:
                    result += szToDecode[i];
                    break;
            }
        }
        return result;
    }


    typedef unsigned char BYTE;

    inline BYTE toHex(BYTE x)
    {
        return x > 9 ? x -10 + 'A': x + '0';
    }

    inline BYTE fromHex(BYTE x)
    {
        return isdigit(x) ? x-'0' : x-'A'+10;
    }

    inline void URLEncode(const string &sIn, string& sOut)
    {
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {      
            BYTE buf[4];
            memset( buf, 0, 4 );
            if( isalnum( (BYTE)sIn[ix] ) )
            {      
                buf[0] = sIn[ix];
            }
            //else if ( isspace( (BYTE)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
            //{
            //    buf[0] = '+';
            //}
            else
            {
                buf[0] = '%';
                buf[1] = toHex( (BYTE)sIn[ix] >> 4 );
                buf[2] = toHex( (BYTE)sIn[ix] % 16);
            }
            sOut += (char *)buf;
        }
    };

    inline void URLDecode(const string &sIn, string& sOut)
    {
        for( size_t ix = 0; ix < sIn.size(); ix++ )
        {
            BYTE ch = 0;
            if(sIn[ix]=='%')
            {
                ch = (fromHex(sIn[ix+1])<<4);
                ch |= fromHex(sIn[ix+2]);
                ix += 2;
            }
            else if(sIn[ix] == '+')
            {
                ch = ' ';
            }
            else
            {
                ch = sIn[ix];
            }
            sOut += (char)ch;
        }
    }

    class HttpReqInfo
    {
        public:
            bool load(const string& headerStr)
            {
                size_t lpos = 0, rpos = 0;
                vector<string> buf;
                rpos = headerStr.find("\n", lpos);

                if(string::npos == rpos)
                {
                    LogFatal("headerStr illegal.");
                    return false;
                }
                string firstline(headerStr, lpos, rpos - lpos);
                trim(firstline);
                if(!split(firstline, buf, " ") || 3 != buf.size())
                {
                    LogFatal("parse header first line failed.");
                    return false;
                }
                _headerMap[KEY_METHOD] = trim(buf[0]); 
                _headerMap[KEY_PATH] = trim(buf[1]); 
                _headerMap[KEY_PROTOCOL] = trim(buf[2]); 
                //first request line end
                //parse path to _methodGetMap
                if("GET" == _headerMap[KEY_METHOD])
                {
                    _parseUrl(firstline, _methodGetMap);
                }


                lpos = rpos + 1;
                if(lpos >= headerStr.size())
                {
                    LogFatal("headerStr illegal");
                    return false;
                }
                //message header begin
                while(lpos < headerStr.size() && string::npos != (rpos = headerStr.find('\n', lpos)) && rpos > lpos)
                {
                    string s(headerStr, lpos, rpos - lpos);
                    size_t p = s.find(':');
                    if(string::npos == p)
                    {
                        break;//encounter empty line
                    }
                    string k(s, 0, p);
                    string v(s, p+1);
                    trim(k);
                    trim(v);
                    if(k.empty()||v.empty())
                    {
                        LogFatal("headerStr illegal.");
                        return false;
                    }
                    upper(k);
                    _headerMap[k] = v;
                    lpos = rpos + 1;
                }
                //message header end

                //body begin

                return true;
            }
        public:
            string& operator[] (const string& key)
            {
                return _headerMap[key];
            }
            bool find(const string& key, string& res)const
            {
                return _find(_headerMap, key, res);
            }
            bool GET(const string& argKey, string& res)const
            {
                return _find(_methodGetMap, argKey, res);
            }
            bool POST(const string& argKey, string& res)const
            {
                return _find(_methodPostMap, argKey, res);
            }
        private:
            HashMap<string, string> _headerMap;
            HashMap<string, string> _methodGetMap;
            HashMap<string, string> _methodPostMap;
            //public:
            friend ostream& operator<<(ostream& os, const HttpReqInfo& obj);
        private:
            bool _find(const HashMap<string, string>& mp, const string& key, string& res)const
            {
                HashMap<string, string>::const_iterator it = mp.find(key);
                if(it == mp.end())
                {
                    return false;
                }
                res = it->second;
                return true;
            }
        private:
            bool _parseUrl(const string& url, HashMap<string, string>& mp)
            {
                if(url.empty())
                {
                    return false;
                }

                uint pos = url.find('?');
                if(string::npos == pos)
                {
                    return false;
                }
                uint kleft = 0, kright = 0;
                uint vleft = 0, vright = 0;
                for(uint i = pos + 1; i < url.size();)
                {
                    kleft = i;
                    while(i < url.size() && url[i] != '=')
                    {
                        i++;
                    }
                    if(i >= url.size())
                    {
                        break;
                    }
                    kright = i;
                    i++;
                    vleft = i;
                    while(i < url.size() && url[i] != '&' && url[i] != ' ')
                    {
                        i++;
                    }
                    vright = i;
                    mp[url.substr(kleft, kright - kleft)] = url.substr(vleft, vright - vleft);
                    i++;
                }

                return true;
            }
    };

    inline std::ostream& operator << (std::ostream& os, const Husky::HttpReqInfo& obj)
    {
        return os << obj._headerMap << obj._methodGetMap << obj._methodPostMap;
    }

}

#endif
