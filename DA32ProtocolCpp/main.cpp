#include"DA32Protocol.h"


vector<Client>clientList;
Server domainServer;


bool findClient(Client tofind)
{
	vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),tofind);
	if (itr!=clientList.end())
	{
		return true;
	}
	return false;
}

int main()
{
	string input;
	Client *newClient=new Client();
	int pass=0;

	cout<<"请输入您的昵称：";
	cin>>myName;

	if(domainServer.Init(3232))
	{
		domainServer.begin();
		cout<<"请输入对方的IP号："<<endl;
		cin>>input;
		for(int i=0;i<clientList.size();i++)
		{
			string temp=inet_ntoa(clientList[i].getAddr().sin_addr);
			if(temp==input)
			{
				pass=1;
				newClient=&clientList[i];
				break;
			}
		}

		if(pass==0)
		{
			while(newClient->newClient(input)==false)
			{
				cout<<"链接失败！请输入对方的IP号："<<endl;
				cin>>input;
			}
			clientList.push_back(*newClient);
		}
		while(1)
		{
			mtx.lock();
			cout<<"现在可以输入您的聊天（输入exit退出）"<<endl;
			mtx.unlock();
			cin>>input;
			if(input=="exit") break;
			mtx.lock();
			if(findClient(*newClient)) newClient->sendData(input);	
			else
			{
				mtx.unlock();
				break;
			}
			mtx.unlock();
		}
	}
	mtx.lock();
	for(int i=clientList.size()-1;i>=0;i--)
	{
		//clientList[i].~Client();
		clientList.pop_back();
	}
	mtx.unlock();
	_sleep(1000);
	//system("pause");
	/*mtx.lock();
	domainServer.~Server();
	mtx.unlock();*/
	newClient=NULL;
	return 0;
}
