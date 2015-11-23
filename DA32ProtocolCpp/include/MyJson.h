#pragma once
#include <string>
#include<fstream>
#include "include/json/json.h"  
using namespace std;
class MyJson
{
private:
	std::string name,type_s,md5_s,time_s,text;
	tm time; 
	int id;
	Json::Value _else;

public:
	MyJson(void);
	~MyJson(void);
	void showJson_in_console();
	void getJson(std::string charflow);
	void getJson(istream& charflow);
};

