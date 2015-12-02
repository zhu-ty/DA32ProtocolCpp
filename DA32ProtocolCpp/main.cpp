#include"DA32Protocol.h"
#include"usinginmain.h"
int main()
{
	usinginmain::initServer();
	string myName="New User";
	cout<<"请输入您的昵称：";
	cin>>myName;
	Client::USERNAME=new char[myName.size()];
	strcpy(Client::USERNAME,myName.c_str());
	if(domainServer->Init(3232))
	{
		domainServer->begin();
		usinginmain::mainThread();//开始主循环。
	}
	usinginmain::quitFunction();
	return 0;
}
