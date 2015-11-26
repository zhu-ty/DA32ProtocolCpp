#include"DA32Protocol.h"


vector<Client>clientList;
Server domainServer;




int main()
{
	string input;
	if(domainServer.Init(3237))
	{
		domainServer.begin();
		Client *newClient= new Client("127.0.0.1",3237);
		clientList.push_back(*newClient);
		while(1)
		{
			while(mtx_cout.try_lock()!=1);
			cout<<"请输入一个测试文本！输入exit退出"<<endl;
			mtx_cout.unlock();
			cin>>input;
			if(input=="exit") break;
			newClient->sendData(input);	
		}
	}
	system("pause");
	clientList.clear();	
	return 0;
}
