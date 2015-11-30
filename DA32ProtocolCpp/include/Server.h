#pragma once
#include <Winsock2.h>
#include<vector>
#include<thread>
#include<mutex>
#include"Client.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;

namespace allarea
{
	//TODO：定义变量
	///线程传递参数
	struct ThreadParament
	{
		function<Client* (SOCKADDR_IN)> checkClientList;
		function<void (Client*)> deleteClient;
		function<void (Client*)> respendClient;
		function<void (Client*)> elsefunction;
	};
}



class Server
{
private:
	SOCKET sockSrv;	
	SOCKADDR_IN addrSrv;
	WORD wVersionRequested;
	WSADATA wsaData;
	vector<SOCKET> sockAccept;//应答socket列表，实际上并没有用到
	std::vector<std::thread *> mThread;

private:	
	int max_binner;//允许的最大链接数,默认会是20
	function <Client* (SOCKADDR_IN)> checkClientList;
	function <void (Client*)> deleteClient;
	function<void (Client*)> respendClient;
	function<void (Client*)> elsefunction;
	std::mutex *mtx;//互斥锁，只在分线程、主线程中使用，请勿在类中使用

public:
	bool Init(int port_id=3232);
	Server(void);
	Server(allarea::ThreadParament tp);
	~Server(void);
	//Server(int port_id=3232);
	void set_max_binner(int n){max_binner=n;}//设置最大链接数
	int begin();//将返回开始监听后的线程，在hThread中的索引号
	vector<SOCKET> get_AcceptList(){return sockAccept;}
	SOCKET get_SockSrv(){return sockSrv;}
	vector<thread*> get_hThread(){return mThread;}
	int isSocketAlreadyInVector(SOCKADDR_IN addr);
	string receiveData(SOCKET torcv);
	void exitSocket(SOCKET &toclose);
	void addSocket(SOCKET &toadd);
	void ListenerThread();
	void ReceiveThread(LPVOID lparam);	
};