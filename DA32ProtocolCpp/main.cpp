#include<iostream>
#include "include/json/json.h"  
#include <fstream>  
#include <string>  
#include <cassert>
  
#pragma comment(lib,"lib_json_d.lib")  
using namespace std;  
int main()
{
    ifstream ifs;
    ifs.open("test/testjson.json");
    assert(ifs.is_open());
 
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root, false))
    {
        return -1;
    }
 
    std::string name = root["name"].asString();
    int age = root["age"].asInt();
 
    std::cout<<name<<std::endl;
    std::cout<<age<<std::endl;
	system("pause");
    return 0;
}