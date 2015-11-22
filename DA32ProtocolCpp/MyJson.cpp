#include "MyJson.h"
tm convert_string_to_time_t(const std::string & time_string)  
{  
    struct tm tm1;  
    time_t time1;  
    int i = sscanf(time_string.c_str(), "%d.%d.%d %d:%d" , &(tm1.tm_year), &(tm1.tm_mon), &(tm1.tm_mday),  &(tm1.tm_hour),&(tm1.tm_min));              
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
    if (!reader.parse(charflow, root, false))
    {
        return ;
    }
 
    name = root["name"].asString();
	id=root["id"].asInt();
	type_s=root["type"].asString();
	time_s=root["data"]["time"].asString();
	text=root["data"]["text"].asCString();
	_else=root["else"];
	md5_s=root["md5"].asString();
	time=convert_string_to_time_t(time_s);
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