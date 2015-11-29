#include "include/Server.h"
#include<iostream>
#include"include\Client.h"
#include"include\Message.h"
#include"include\MyJson.h"
#include<mutex>

extern DWORD WINAPI ReceiveThread(LPVOID lparam);
extern DWORD WINAPI ListenerThread(LPVOID lparam);
//线程之间传递的参数
//关闭server，发生在整个程序关闭时。
Server::~Server(void)
{
	//还没有和所有的链接说再见呢!这里不说了，因为Server不会退出！退出则是主程序退出，直接对整个clientList说再见就好，不需要再这个类的方法里写。
	closesocket(sockSrv);//关闭监听
	for(int i=0;i<hThread.size();i++) CloseHandle(hThread[i]);//停止所有链接的线程
	for(int i=0;i<sockAccept.size();i++) closesocket(sockAccept[i]);//关闭所有使用中的socket
	hThread.clear();
	sockAccept.clear();
	threadId.clear();
	WSACleanup();
}
//新建一个server
Server::Server()
{
}
//初始化，加载字库
bool Server::Init(int port_id)//加载字库等操作
{
	int err;
	///////////////////加载字库////////////////
	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
	{
		cout<<"faild when WSAStartup!"<<endl;
		system("pause");
		return false;
	}
	if ( LOBYTE( wsaData.wVersion ) != 1 ||HIBYTE( wsaData.wVersion ) != 1 )
	{ 
		WSACleanup( );
		cout<<"faild when WSAStartup!"<<endl;
		system("pause");
		return false;
	}
	max_binner=20;
	sockSrv=socket(AF_INET,SOCK_STREAM,0);//服务器，第二个参数是选择了流套接字，UDP选择SOCK_DGRAM数据报套接字
	addrSrv.sin_family=AF_INET;//定义类型
	addrSrv.sin_port=htons(port_id);//定义服务器的端口
	addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//定义服务器ip地址筛选,因为是服务器，操作时listen，所以参数是ANY，将自动选择默认的本地网卡。如果是客户，则操作时connet，需要将参数置为目标ip。 
	
	if(::bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(addrSrv)) == -1)
	{
		cout<<"bind() failed."<<endl;
		closesocket(sockSrv);//函数内打开，失败了，应该在函数内关闭
		//system("pause");
		this->~Server();
		return false;
	}
	if(listen(sockSrv, max_binner) == SOCKET_ERROR)//尝试开始监听,参数2是等待队列最大长度，也就是监听时最大的连接数
	{
		cout<<"Error listening on socket."<<endl;
		system("pause");
		return false;
	}
	cout<<"Server Created!"<<endl<<"Now,Listening``````"<<endl;
	return true;
}
//创建独立的线程，让监听socket开始accpet，需要外部锁
int Server::begin()
{
	DWORD *newId= new DWORD;//新的线程id
	HANDLE *newThread=new HANDLE;//新的线程变量

	*newThread=CreateThread(NULL, NULL, ListenerThread, (LPVOID)this, 0, newId);
	hThread.push_back(*newThread);
	threadId.push_back(*newId);
	return hThread.size();
}
//TODO：查询已有的Vector中所有的socket的add是否有和参数相同的
int Server::isSocketAlreadyInVector(SOCKADDR_IN addr)
{
	return 0;
}
//开始接受数据，此时socket会进入等待接收状态，直到接收结束
string Server::receiveData(SOCKET torcv)
{
	int bytesRecv=-1;
	char *recvbuf=new char[8];
	char rare[2];
	while(bytesRecv == SOCKET_ERROR || bytesRecv==0)
	{
		bytesRecv = recv(torcv, recvbuf, 2, 0);
		try
		{
			if (recvbuf[0]!='\x32' || recvbuf[1]!='\xA0') bytesRecv=-1;//server运行到此处会阻塞，直到接收信息或触发异常
		}
		catch(exception e)
		{
			//TODO:可能会有断线异常。
			break;
		}
	}
	bytesRecv=-1;
	while(bytesRecv == SOCKET_ERROR || bytesRecv==0)
	{
		bytesRecv = recv(torcv, recvbuf, 8, 0);
	}
	bytesRecv=-1;
	UINT64 length=0;
	for (int i=0;i<8;i++)
	{
		int x=pow(256,i);
		x=(unsigned char)(recvbuf[i])*x;
		length+=x;
	}
	recvbuf=new char[length];
	while(bytesRecv == SOCKET_ERROR || bytesRecv==0)
	{
		bytesRecv = recv(torcv, recvbuf, length, 0);
	}
	bytesRecv=-1;
	while(bytesRecv == SOCKET_ERROR || bytesRecv==0)
	{
		bytesRecv = recv(torcv, rare, 2, 0);
	}
	return recvbuf;
}
//关闭一个socket，并注销它的一切信息，需要外部锁
void Server::exitSocket(SOCKET &toclose)
{
	vector<SOCKET>::iterator itr = find(sockAccept.begin(),sockAccept.end(),toclose);
	closesocket(toclose);
	sockAccept.erase(itr);
}
//添加一个socket到列表中，这里添加的都是和一个Client链接的socket，需要外部锁
void Server::addSocket(SOCKET &toadd)
{
	sockAccept.push_back(toadd);
}