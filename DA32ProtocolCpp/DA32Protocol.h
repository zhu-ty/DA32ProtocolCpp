/*
请将需要的预编译文件放在这个文件内。
*/
#pragma comment(lib,"lib_json.lib") 
#include<iostream>
#include "include/json/json.h"  
#include <fstream>  
#include <string>  
#include <cassert>
#include<time.h>
#include"include/md5.h"
#include"include/Client.h"
#include"include/Message.h"
#include"include/MyJson.h"
#include"include/Server.h"
#include<vector>
#include<mutex>
//TODO:添加你的头文件
//eg:#include "xx.h"
   
using namespace std; 

//TODO：定义变量
vector<Client>clientList;//需要维护的Client列表
Server domainServer;//需要维护的Server
mutex mtx;//互斥锁，只在分线程、主线程中使用，请勿在类中使用
///线程传递参数
struct ThreadParament
{
	SOCKET *rec_socket;
	Server *server;
};


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
///接收线程
DWORD WINAPI ReceiveThread(LPVOID lparam)
	{
		//应当能接受一段字节流，然后交给下层的Message处理。得到一个处理结果存到全局变量MyJson中。
		//参数lparam是接受用的ReceiveSocket。
		//还需要调用本地的Client去建立一个反向的TCP链接
		//需要有能力调用对应的Client去反向回复
		ThreadParament* pa=(ThreadParament*)lparam;
		SOCKET *ReceiveSocket=pa->rec_socket;//得到建立连接的那个socket
		Client *respond_client=NULL;//本机上的客户机，指向全局的一个Client，或者在此线程中创建一个全局的Client，并用这个指针操作。需要在线程结束时释放。
		Server *server_p=pa->server;//本机上的服务器
		SOCKADDR_IN dest_add;//和本机建立通信的Client的地址
		string rcv;//接收到的字节流。
		int x=-1;
		int nAddrLen = sizeof(dest_add);
		mtx.lock();
		server_p->addSocket(*ReceiveSocket);//维护acceptsocket
		mtx.unlock();
		if(::getpeername(*ReceiveSocket, (SOCKADDR*)&dest_add, &nAddrLen) != 0)
		{
			mtx.lock();
			std::cout<<"Get IP address by socket failed!"<<endl;
			server_p->exitSocket(*ReceiveSocket);//在server中删除这个socket
			mtx.unlock();
			return 0;
		}
		mtx.lock();
		cout<<"IP: "<<::inet_ntoa(dest_add.sin_addr)<<"  PORT: "<<ntohs(dest_add.sin_port)<<"开始向您通话！"<<endl;
		mtx.unlock();
		for(int i=0;i<clientList.size();i++)
		{
			if(inet_ntoa(clientList[i].getAddr().sin_addr)==inet_ntoa(dest_add.sin_addr))
			{
				respond_client=&clientList[i];//如果这个来源在本机的客户端链接表里
				break;
			}
		}
		if(respond_client==NULL)//如果不在，就新建一个客户端
		{
			respond_client=new Client();
			mtx.lock();
			while(respond_client->newClient(inet_ntoa(dest_add.sin_addr),3232)==false);
			clientList.push_back(*respond_client);//加入到List中
			mtx.unlock();
		}
		//主循环
		while(1)
		{
			try
			{
				Message mess=Message();
				MyJson infomation=MyJson();
				rcv=server_p->receiveData(*ReceiveSocket);//接收信息，在这里线程会等待到接收到为止
				infomation=mess.getContent(rcv);//这里完成了从字节层到信息层的解包
				if(infomation.type_s=="exit")
				{	
					mtx.lock();
					//注销这个客户端			
					vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),*respond_client);
					if (itr!=clientList.end())//如果找到了这个Client还活着，就杀了它
					{
						itr->exit();
						clientList.erase(itr);
					}
					server_p->exitSocket(*ReceiveSocket);//在acceptsocket中删除这个socket
					mtx.unlock();
					cout<<inet_ntoa(dest_add.sin_addr)<<"下线了！"<<endl;
					respond_client=NULL;
					return 0;
				}
				else
				{
					if(infomation.type_s=="text")
					{
						mtx.lock();
						respond_client->respend();
						infomation.showJson_in_console();
						cout<<endl;
						mtx.unlock();
					}
					else
					{
						//TODO:对其他的包类型进行处理
					}
				}
			}
			catch(exception e)//TODO：应当处理此类异常！
			{
				cout<<e.what();
				mtx.lock();
				server_p->exitSocket(*ReceiveSocket);
				mtx.unlock();
				delete respond_client;
				return -1;
			}
		}
		mtx.lock();
		server_p->exitSocket(*ReceiveSocket);
		mtx.unlock();
		delete respond_client;
		respond_client=NULL;
		return 1;
	}
///监听线程
DWORD WINAPI ListenerThread(LPVOID lparam)
	{
		SOCKET *AcceptSocket=new SOCKET();
		Server* server=(Server*)(LPVOID)lparam;
		SOCKET sr=server->get_SockSrv();
		while(1)
		{
			try
			{
				*AcceptSocket = accept(sr,0, 0);//会在这一步一直等到天荒地老
				mtx.lock();
				server->get_AcceptList().push_back(*AcceptSocket);
				mtx.unlock();
				DWORD *newId= new DWORD;//新的线程id
				HANDLE *newThread=new HANDLE;//新的线程变量
				ThreadParament pa;//线程间通信的参数
				pa.rec_socket=AcceptSocket;
				pa.server=server;
				mtx.lock();
				*newThread=CreateThread(NULL, NULL, ReceiveThread, (LPVOID)&pa, 0, newId);
				server->get_hThread().push_back(*newThread);
				server->get_threadId().push_back(*newId);
				mtx.unlock();
			}
			catch(exception e)
			{
				cout<<e.what()<<endl;
				closesocket(sr);
			}
		}
		return 1;
	}
///主线程，事实上不是一个线程，就是一个函数。
void mainThread()
{
	string input,context;
	Client *newClient=new Client();
	int pass=0;

	while(1)
		{
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
///安全退出所需要的操作
void quitFunction()
{
	mtx.lock();
	for(int i=clientList.size()-1;i>=0;i--)
	{
		clientList[i].exit();
		clientList.pop_back();
	}
	mtx.unlock();
	_sleep(1000);
}

