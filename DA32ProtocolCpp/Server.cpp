#include "include\Server.h"
#include<iostream>
#include"include\Client.h"
#include"include\Message.h"
#include"include\MyJson.h"
#include<mutex>

Server::~Server(void)
{
	//还没有和所有的链接说再见呢!这里不说了，因为Server不会退出！退出则是主程序退出，直接对整个clientList说再见就好，不需要再这个类的方法里写。
	closesocket(sockSrv);//关闭监听
	for(int i=0;i<mThread.size();i++) mThread[i]->~thread();//停止所有链接的线程
	for(int i=0;i<sockAccept.size();i++) closesocket(sockAccept[i]);//关闭所有使用中的socket
	mThread.clear();
	sockAccept.clear();
	WSACleanup();
	delete mtx;
}
//新建一个server
Server::Server()
{
	mtx=new mutex();
}

Server::Server(allarea::ThreadParament tp)
{
	checkClientList= tp.checkClientList;
	deleteClient=tp.deleteClient;
	elsefunction=tp.elsefunction;
	respendClient=tp.respendClient;
	mtx=new mutex();
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
	//thread newThread(mem_fun(&Server::ListenerThread),this);//开始线程
	thread *newThread=new thread(&Server::ListenerThread,this);
	newThread->detach();
	mtx->lock();
	mThread.push_back(newThread);
	int x=mThread.size();
	mtx->unlock();
	return x;
	//return 1;
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
		bytesRecv = recv(torcv, recvbuf, Client::HEAD_LENTH, 0);
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
		bytesRecv = recv(torcv, recvbuf, Client::WIEDTH_LENTH, 0);
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
		bytesRecv = recv(torcv, rare, Client::RARE_LENTH, 0);
	}
	return recvbuf;
}
//关闭一个socket，并注销它的一切信息，需要外部锁
void Server::exitSocket(SOCKET &toclose)
{
	vector<SOCKET>::iterator itr = find(sockAccept.begin(),sockAccept.end(),toclose);
	closesocket(toclose);
	if(itr!=sockAccept.end())sockAccept.erase(itr);
}
//添加一个socket到列表中，这里添加的都是和一个Client链接的socket，需要外部锁
void Server::addSocket(SOCKET &toadd)
{
	sockAccept.push_back(toadd);
}

void Server::ReceiveThread(LPVOID lparam)
{
	//应当能接受一段字节流，然后交给下层的Message处理。得到一个处理结果存到全局变量MyJson中。
	//参数lparam是接受用的ReceiveSocket。
	//还需要调用本地的Client去建立一个反向的TCP链接
	//需要有能力调用对应的Client去反向回复
	SOCKET *ReceiveSocket=(SOCKET*)lparam;//得到建立连接的那个socket
	Client *respond_client=NULL;//本机上的客户机，指向全局的一个Client，或者在此线程中创建一个全局的Client，并用这个指针操作。需要在线程结束时释放。
	SOCKADDR_IN dest_add;//和本机建立通信的Client的地址
	string rcv;//接收到的字节流。
	int x=-1;
	int nAddrLen = sizeof(dest_add);
	//通过RCVSocket获得对方的地址
	if(::getpeername(*ReceiveSocket, (SOCKADDR*)&dest_add, &nAddrLen) != 0)
	{
		mtx->lock();
		std::cout<<"Get IP address by socket failed!"<<endl;
		this->exitSocket(*ReceiveSocket);//在server中删除这个socket
		mtx->unlock();
		return;
	}

	mtx->lock();
	cout<<"IP: "<<::inet_ntoa(dest_add.sin_addr)<<"  PORT: "<<ntohs(dest_add.sin_port)<<"开始向您通话！"<<endl;
	mtx->unlock();

	mtx->lock();
	respond_client=checkClientList(dest_add);//调用回调函数，checkClientList，并完成恢复Client的赋值
	mtx->unlock();
	//主循环

	while(1)
	{
		try
		{
			Message mess=Message();
			MyJson infomation=MyJson();
			rcv=this->receiveData(*ReceiveSocket);//接收信息，在这里线程会等待到接收到为止
			infomation=mess.getContent(rcv);//这里完成了从字节层到信息层的解包
			if(infomation.type_s=="exit")
			{	
				mtx->lock();
				//注销这个客户端，应当由回调函数完成
				this->exitSocket(*ReceiveSocket);//在acceptsocket中删除这个socket
				deleteClient(respond_client);
				mtx->unlock();
				cout<<inet_ntoa(dest_add.sin_addr)<<"下线了！"<<endl;
				respond_client=NULL;//它只是一个无辜的指针，告诉你内容在哪里，但请不要再本函数内操作它的内容。
				return ;
			}
			else
			{
				if(infomation.type_s=="text")
				{
					mtx->lock();
					respendClient(respond_client);
					infomation.showJson_in_console();
					cout<<endl;
					mtx->unlock();
				}
				else
				{
					//TODO:对其他的包类型进行处理，如果需要操作Client，依旧请使用回调函数。
					elsefunction(respond_client);
				}
			}
		}
		catch(exception e)//TODO：应当处理此类异常！
		{
			cout<<e.what();
			mtx->lock();
			this->exitSocket(*ReceiveSocket);
			mtx->unlock();
			deleteClient(respond_client);
			respond_client=NULL;
			return;
		}
	}
	mtx->lock();
	this->exitSocket(*ReceiveSocket);
	mtx->unlock();
	delete respond_client;
	respond_client=NULL;
	return ;
}
	///监听线程
void Server::ListenerThread()
{
	SOCKET *AcceptSocket=new SOCKET();
	SOCKET sr=this->get_SockSrv();
	while(1)
	{
		try
		{
			*AcceptSocket = accept(sr,0, 0);//会在这一步一直等到天荒地老
			mtx->lock();
			this->get_AcceptList().push_back(*AcceptSocket);//将接收到信息的socket加入列表				
			std::thread *RcvThread=new std::thread(&Server::ReceiveThread,this,AcceptSocket);
			RcvThread->detach();
			this->get_hThread().push_back(RcvThread);
			mtx->unlock();
		}
		catch(exception e)
		{
			cout<<e.what()<<endl;
			closesocket(sr);
		}
	}
}