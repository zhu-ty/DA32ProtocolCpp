#include "include/MyJson.h"
#include"include/md5.h"
//辅助函数，将固定格式的字符串解析成时间结构tm
tm convert_string_to_time_t(const std::string & time_string)  
{  
    struct tm tm1;  
    time_t time1;  
	int i = sscanf(time_string.c_str(), "%d.%d.%d %d:%d:%d" , &(tm1.tm_year), &(tm1.tm_mon), &(tm1.tm_mday),  &(tm1.tm_hour),&(tm1.tm_min),&(tm1.tm_sec));              
    return tm1;  
}  

MyJson::MyJson(void)
{
}

MyJson::~MyJson(void)
{
}

void MyJson::getJson(std::string charflow)
{
	Json::Reader reader;
    Json::Value root;
	std::string md5_check;
	char *p;
    if (!reader.parse(charflow, root, false))
    {
        return ;
    }
 
    name = root["data"]["name"].asString();
	id=root["id"].asInt();
	type_s=root["type"].asString();
	time_s=root["time"].asString();
	text=root["data"]["text"].asString();
	_else=root["else"];
	md5_s=root["md5"].asString();
	time=convert_string_to_time_t(time_s);
	p=(char*)&id;
	md5_check=*p;
	md5_check=md5_check+*(p+1)+*(p+2)+*(p+3);
	md5_check=md5_check+type_s+time_s+name+text;
	MD5 md5(md5_check);
	string result = md5.md5();
	if (result!=md5_s)
	{
		//校验出错
		cout<<"md5校验失败！！"<<endl;
	}
}

void MyJson::getJson(istream& charflow)
{
	Json::Reader reader;
    Json::Value root;
    if (!reader.parse(charflow, root, false))
    {
        return ;
    }
 
    name = root["data"]["name"].asString();
	id=root["id"].asInt();
	type_s=root["type"].asString();
	time_s=root["time"].asString();
	text=root["data"]["text"].asString();
	_else=root["else"];
	md5_s=root["md5"].asString();
	time=convert_string_to_time_t(time_s);
}

void MyJson::showJson_in_console()
{
	std::cout<<name<<std::endl;
    std::cout<<id<<std::endl;
	std::cout<<time_s<<std::endl;
	std::cout<<text<<std::endl;

	system("pause");
}