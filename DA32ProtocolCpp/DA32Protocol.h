/*
请将需要的预编译文件放在这个文件内。
*/
#pragma once
#pragma comment(lib,"lib_json.lib") 
#include<iostream>
#include "include/json/json.h"  
#include <fstream>  
#include <string>  
#include <cassert>
#include<time.h>
#include"include/md5.h"
#include"include/Client.h"
#include"include/Message.h"
#include"include/MyJson.h"
#include"include/Server.h"
#include<vector>
//TODO:添加你的头文件
//eg:#include "xx.h"
   
using namespace std; 
vector<Client>clientList;//需要维护的Client列表
Server *domainServer;//需要维护的Server
