#pragma once
#include<string>
#include"MyJson.h"
using namespace std;
class Message
{
public:
	Message(void);
	~Message(void);
	MyJson getContent(string dataflow);
	string getWrap(string tosend);
};

