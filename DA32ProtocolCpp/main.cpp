#include"DA32Protocol.h"


vector<Client>clientList;
Server domainServer;


bool findClient(Client *tofind)
{
	vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),*tofind);
	if (itr!=clientList.end())
	{
		return true;
	}
	return false;
}

int main()
{
	string input,context;
	Client *newClient=new Client();
	int pass=0;

	cout<<"请输入您的昵称：";
	cin>>myName;

	if(domainServer.Init(3232))
	{
		domainServer.begin();
		while(1)
		{
			pass=0;//这个Client是否存在的flag
			cout<<"请输如“对方IP 内容(用空格分开)”："<<endl;
			cin>>input;//TODO:这里的输入机制决定了，你不能输入了空格之后又后悔输入···backspace回来改ip···
			cin>>context;
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
			if(pass==0)//不存在就新建之！
			{
				while(newClient->newClient(input)==false)
				{
					cout<<"链接失败！请输入正确的IP号："<<endl;
					cin>>input;
				}
				clientList.push_back(*newClient);
			}
			if(context=="exit") break;
			mtx.lock();
			if(findClient(newClient)) newClient->sendData(context);	
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
		clientList[i].exit();
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
