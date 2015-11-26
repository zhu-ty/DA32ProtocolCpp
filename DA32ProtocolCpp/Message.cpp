#include "include/Message.h"
#include<comutil.h>

string GBKToUTF8(const std::string& strGBK)  
{  
    string strOutUTF8 = "";  
    WCHAR * str1;  
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);  
    str1 = new WCHAR[n];  
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);  
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
    char * str2 = new char[n];  
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
    strOutUTF8 = str2;  
    delete[]str1;  
    str1 = NULL;  
    delete[]str2;  
    str2 = NULL;  
    return strOutUTF8;  
}  
string UTF8ToGBK(const std::string& strUTF8)  
{  
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);  
    unsigned short * wszGBK = new unsigned short[len + 1];  
    memset(wszGBK, 0, len * 2 + 2);  
    MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);  
  
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)wszGBK, -1, NULL, 0, NULL, NULL);  
    char *szGBK = new char[len + 1];  
    memset(szGBK, 0, len + 1);  
    WideCharToMultiByte(CP_ACP,0, (LPCWCH)wszGBK, -1, szGBK, len, NULL, NULL);  
    //strUTF8 = szGBK;  
    std::string strTemp(szGBK);  
    delete[]szGBK;  
    delete[]wszGBK;  
    return strTemp;  
}  

Message::Message(void)
{
}


Message::~Message(void)
{
}

//解包，获得Json
MyJson Message::getContent(string dataflow)
{
	MyJson info;
	dataflow=UTF8ToGBK(dataflow);
	info.getJson(dataflow);
	return info;
}
//HELP！！！字段长度是一个八位的正数，直接用数值表示。这样就不能用char、string来传递参数了！！！因为0x00会自动认为其截止！
char* Message::getWrap(char* tosend)
{
	string data=GBKToUTF8(tosend);
	/**/
	char *pointer=new char[data.length()];
	for(int i=0;i<data.length();i++)pointer[i]=data[i];
	return pointer;
}