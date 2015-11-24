#include"DA32Protocol.h"
int main()
{
	MyJson test;
	Json::Value _el;
	ifstream ifs;
	string input,packed;
	//一些初始化操作
	test.name="Cwx";
	test.type_s="text";
	test._else=_el;
	test.id=1;

	cout<<"输入要聊天信息：";
	cin>>input;
	test.text=input;
	packed=test.PackJson(input);
	cout<<packed;//打包好的packed
	/*ifs.open("test/testjson.json");
	assert(ifs.is_open());
	if(!test.getJson(ifs)) return -1;
	cout<<"from filestream"<<endl;
	test.showJson_in_console();*/
	cout<<"from charstream"<<endl;
	test.getJson(packed);
	test.showJson_in_console();
    return 0;
}