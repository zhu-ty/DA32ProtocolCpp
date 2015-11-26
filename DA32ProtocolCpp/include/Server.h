#pragma once

#include <Winsock2.h>
#include<vector>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
class Server
{
private:
	SOCKET sockSrv;	
	SOCKADDR_IN addrSrv;
	WORD wVersionRequested;
	WSADATA wsaData;

	vector<DWORD> threadId;//线程id
	vector<HANDLE> hThread;//线程
	vector<SOCKET> sockAccept;//应答socket列表，实际上并没有用到

	
	int max_binner;//允许的最大链接数,默认会是20
public:
	bool Init(int port_id=3232);
	Server(void);//不适用默认的构造
	~Server(void);
	//Server(int port_id=3232);
	void set_max_binner(int n){max_binner=n;}//设置最大链接数
	int begin();//将返回开始监听后的线程，在hThread中的索引号
	vector<SOCKET> get_AcceptList(){return sockAccept;}
	SOCKET get_SockSrv(){return sockSrv;}
	vector<HANDLE> get_hThread(){return hThread;}
	vector<DWORD> get_threadId(){return threadId;}
	int isSocketAlreadyInVector(SOCKADDR_IN addr);
	string receiveData(SOCKET torcv);
	void exitSocket(SOCKET &toclose);
	void addSocket(SOCKET &toadd);
};

