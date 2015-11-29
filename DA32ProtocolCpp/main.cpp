#include"DA32Protocol.h"

int main()
{
	string myName="New User";
	cout<<"请输入您的昵称：";
	cin>>myName;
	Client::USERNAME=new char[myName.length()];
	strcpy(Client::USERNAME,myName.c_str());
	if(domainServer.Init(3232))
	{
		domainServer.begin();
		mainThread();//开始主循环。
	}
	quitFunction();
	return 0;
}
