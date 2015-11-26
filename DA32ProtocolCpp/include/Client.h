#pragma once

#include <Winsock2.h>
#include<iostream>
#include<vector>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

class Client
{
private:
	SOCKET sockClient;	
	SOCKADDR_IN addrClient;
	WORD wVersionRequested;
	WSADATA wsaData;
	void Init();

public:
	const static int HEAD_LENTH=2;
	const static int RARE_LENTH=2;
	const static int WIEDTH_LENTH=8;
	static char HEAD_CHAR[2];
	static char RARE_CHAR[2];

	Client(string dest_ip,int dest_port=3232);
	void exit();
	void respend();
	void sendData(string info);
	SOCKADDR_IN getAddr(){return addrClient;}
	SOCKET getSocket(){return sockClient;}
	bool operator==(Client a){return a.getSocket()==this->getSocket();}
	~Client(void);
};

