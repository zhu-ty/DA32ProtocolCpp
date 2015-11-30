#pragma once
#include"DA32Protocol.h"
extern string GBKToUTF8(const std::string& strGBK);
namespace usinginmain
{
	mutex c_mtx;
	//TODO：定义函数
	///返回是否可以从ClientList中找到一个Client
	bool findClient(Client *tofind)
	{
		vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),*tofind);
		if (itr!=clientList.end())
		{
			return true;
		}
		return false;
	}
	///主线程，事实上不是一个线程，就是一个函数。
	void mainThread()
	{
		string input,context;
		Client *newClient;
		int pass=0;
		while(1)
		{
			newClient=new Client();
			pass=0;//这个Client是否存在的flag
			cout<<"请输如“对方IP 内容(用空格分开)”："<<endl;
			cin>>input;//TODO:这里的输入机制决定了，你不能输入了空格之后又后悔输入·backspace回来改ip···
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
					if(pass==1) break;
				}
				if(pass==0) clientList.push_back(*newClient);
			}
			if(context=="exit") break;
			//context=context+"\n"+"有一回车！";
			c_mtx.lock();
			if(findClient(newClient)) newClient->sendData(context);	
			else
			{
				c_mtx.unlock();
				break;
			}
			c_mtx.unlock();
		}
	}
	///安全退出所需要的操作
	void quitFunction()
	{
		for(int i=clientList.size()-1;i>=0;i--)
		{
			clientList[i].exit();
			c_mtx.lock();
			clientList.pop_back();
			c_mtx.unlock();
		}
		_sleep(1000);
	}

	Client* checkClientList(SOCKADDR_IN add)
	{
		Client * respond_client=NULL;
		for(int i=0;i<clientList.size();i++)
		{
			if(inet_ntoa(clientList[i].getAddr().sin_addr)==inet_ntoa(add.sin_addr))
			{
				respond_client=&clientList[i];//如果这个来源在本机的客户端链接表里
				break;
			}
		}
		//此部分应由回调函数完成
		if(respond_client==NULL)//如果不在，就新建一个客户端
		{
			respond_client=new Client();
			while(respond_client->newClient(inet_ntoa(add.sin_addr),3232)==false);
			c_mtx.lock();
			clientList.push_back(*respond_client);//加入到List中
			c_mtx.unlock();
		}
		return respond_client;
	}

	void deleteClient(Client* c)
	{
		c_mtx.lock();
		vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),*c);
		if (itr!=clientList.end())//如果找到了这个Client还活着，就杀了它
		{
			itr->exit();
			clientList.erase(itr);
		}
		c_mtx.unlock();
	}

	void respendClient(Client* c)
	{
		c->respend();
	}

	void elsefunction(Client* c)
	{

	}

	void initServer()
	{
		allarea::ThreadParament tp;
		tp.checkClientList=(function<Client* (SOCKADDR_IN)>)(usinginmain::checkClientList);
		tp.deleteClient=(function<void (Client*)>)(usinginmain::deleteClient);
		tp.respendClient=(function<void (Client*)>)(usinginmain::respendClient);
		tp.elsefunction=(function<void (Client*)>)(usinginmain::elsefunction);
		domainServer=new Server(tp);
	}
}