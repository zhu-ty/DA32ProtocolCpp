#include "include/Server.h"
#include<iostream>
#include"include\Client.h"
#include"include\Message.h"
#include"include\MyJson.h"
#include<mutex>

//线程之间传递的参数
mutex mtx_server_acceptlist;//用来保护acceptlist的互斥锁
mutex mtx_cout;//用来保护cout的互斥锁
mutex mtx_clientlist;
mutex mtx_handle;

extern vector<Client>clientList;//本机客户端组
//线程通信参数
struct ThreadParament
{
	SOCKET *rec_socket;
	Server *server;
};
//接收线程
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
	while (mtx_server_acceptlist.try_lock()!=1);
	server_p->addSocket(*ReceiveSocket);
	mtx_server_acceptlist.unlock();
	if(::getpeername(*ReceiveSocket, (SOCKADDR*)&dest_add, &nAddrLen) != 0)
    {
		while(mtx_cout.try_lock()!=1);
		std::cout<<"Get IP address by socket failed!n"<<endl;
		system("pause");
		mtx_cout.unlock();
		while (mtx_server_acceptlist.try_lock()!=1);
		server_p->exitSocket(*ReceiveSocket);//在server中删除这个socket
		mtx_server_acceptlist.unlock();
        return 0;
    }
	while(mtx_cout.try_lock()!=1);
	cout<<"IP: "<<::inet_ntoa(dest_add.sin_addr)<<"  PORT: "<<ntohs(dest_add.sin_port)<<endl;
	mtx_cout.unlock();
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
		respond_client=new Client(inet_ntoa(dest_add.sin_addr),3232);
		while(!mtx_clientlist.try_lock())
		{
			clientList.push_back(*respond_client);//加入到List中
			mtx_clientlist.unlock();
		}
	}
	//主循环
	while(1)
	{
		Message mess=Message();
		MyJson infomation=MyJson();
		rcv=server_p->receiveData(*ReceiveSocket);//接收信息，在这里线程会等待到接收到为止
		infomation=mess.getContent(rcv);//这里完成了从字节层到信息层的解包
		if(infomation.type_s=="exit")
		{
			infomation.showJson_in_console();
			respond_client->~Client();
			//注销这个客户端			
			while(mtx_clientlist.try_lock()!=1);
			vector<Client>::iterator itr = find(clientList.begin(),clientList.end(),*respond_client);
			clientList.erase(itr);
			mtx_clientlist.unlock();
			//从客户端list中删除它
			while(mtx_server_acceptlist.try_lock()!=1);
			server_p->exitSocket(*ReceiveSocket);//在server中删除这个socket
			mtx_server_acceptlist.unlock();
			delete respond_client;//内存回收
			respond_client=NULL;
			return 0;
		}
		else
		{
			if(infomation.type_s=="text")
			{
				respond_client->respend();
				while(mtx_cout.try_lock()!=1);
				cout<<"Server Receive:"<<endl;
				infomation.showJson_in_console();
				cout<<endl;
				mtx_cout.unlock();
			}
			else
			{
				//TODO:对其他的包类型进行处理
			}
		}
	}
	while(mtx_server_acceptlist.try_lock()!=1);
	server_p->exitSocket(*ReceiveSocket);
	mtx_server_acceptlist.unlock();
	delete respond_client;
	respond_client=NULL;
	return 1;
}
//监听线程
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
			while(mtx_cout.try_lock()!=1);
			cout<<"A Client connetted!"<<endl;//监听到结果,建立了一个连接
			mtx_cout.unlock();
			while(mtx_server_acceptlist.try_lock()!=1);
			server->get_AcceptList().push_back(*AcceptSocket);
			mtx_server_acceptlist.unlock();
			DWORD *newId= new DWORD;//新的线程id
			HANDLE *newThread=new HANDLE;//新的线程变量
			ThreadParament pa;//线程间通信的参数
			pa.rec_socket=AcceptSocket;
			pa.server=server;
			while(mtx_handle.try_lock()!=1);
			*newThread=CreateThread(NULL, NULL, ReceiveThread, (LPVOID)&pa, 0, newId);
			server->get_hThread().push_back(*newThread);
			server->get_threadId().push_back(*newId);
			mtx_handle.unlock();
		}
		catch(exception e)
		{
			cout<<e.what()<<endl;
			closesocket(sr);
		}
	}
	return 1;
}
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
	//Init();
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
//创建独立的线程，让监听socket开始accpet
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
		if (recvbuf[0]!='\x32' || recvbuf[1]!='\xA0') bytesRecv=-1;//改为||
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
//关闭一个socket，并注销它的一切信息
void Server::exitSocket(SOCKET &toclose)
{
	vector<SOCKET>::iterator itr = find(sockAccept.begin(),sockAccept.end(),toclose);
	closesocket(toclose);
	sockAccept.erase(itr);
}
//添加一个socket到列表中，这里添加的都是和一个Client链接的socket
void Server::addSocket(SOCKET &toadd)
{
	sockAccept.push_back(toadd);
}