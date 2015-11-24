#pragma once
#include <string>
#include<fstream>
#include "include/json/json.h"  

using namespace std;
//信息层类MyJson，包含了解Json包获取其中信息的操作，共有成员变量为信息层的数据。
class MyJson
{
public:
	std::string name,type_s,md5_s,time_s,text;
	tm time_tm; 
	int id;
	Json::Value _else;

public:
	MyJson(void);
	~MyJson(void);
	//将该Json信息在命令行中显示，可以便于调试
	void showJson_in_console();
	//解码Json信息的方式一，从一个字符串中获取
	bool getJson(std::string charflow);
	//解码Json信息的方式二，从文件输入流中获取
	bool getJson(istream& charflow);

	//编码Json信息到一个Json包中。input是文本内容，需要预置type_s、_else其余信息都不需要设置，返回的char*是用UTF-8编码JSON包
	char* PackJson(std::string input);
};

