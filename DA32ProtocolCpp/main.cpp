#include"DA32Protocol.h"
int main()
{
	MyJson test;
	ifstream ifs;
	ifs.open("test/testjson.json");
	assert(ifs.is_open());
	test.getJson(ifs);
	test.showJson_in_console();
    return 0;
}