// 
// RSC Socket Class (RSCSocket.h) 
// By Jae H. Park 
//
// Created: 02.01.2005
//
// RSCSocket class handles the network communication via WinSock
// Note: only IPv4 is supported
// 

#pragma once

#define NO_FLAGS_SET		0
//#define PORT (u_short)	44000
//#define MAXBUFLEN			4096
//#define MAXCLIENT			256

class RSCSocket{
public:
	static bool RSCSocketInit_W();
	static void RSCSocketUninst_W();
	static bool m_socket_w_status;

public:
	RSCSocket();
	RSCSocket(SOCKET sock);
	~RSCSocket();

	bool RSCSend(char *buf, const unsigned int buflen);
	bool RSCReceive(char *buf, const unsigned int buflen);

	bool RSCCreateStream(bool isNagleOn); // equivalent to create function
	SOCKET RSCAccept(void);
	bool RSCConnect(u_short ori_port, char *specific_ip);
	bool RSCBind(u_short ori_port, char *specific_ip);
	bool RSCListen(int n);
	
	bool RSCClose(void);
	bool RSCShutdown(int how);
	bool RSCReceiveSetTimeOut(int t);
	bool RSCSendSetTimeOut(int t);
	bool RSCNagleOption(bool on);

protected:
	SOCKET m_socket;
	int m_status;

};