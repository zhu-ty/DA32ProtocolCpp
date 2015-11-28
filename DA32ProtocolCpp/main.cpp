#include"DA32Protocol.h"


vector<Client>clientList;
Server domainServer;

int main()
{
	string input;
	Client *newClient;
	if(domainServer.Init(3232))
	{
		domainServer.begin();
		cout<<"请输入对方的IP号："<<endl;
		cin>>input;
		try
		{
			newClient= new Client(input,3232);
		}
		catch(exception e)
		{

		}
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
