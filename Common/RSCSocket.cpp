// 
// RSC Socket Class (RSCSocket.cpp) 
// By Jae H. Park 
//
// Created: 02.01.2005
//
// RSCSocket class handles the network communication via WinSock
//
// 

#include "stdafx.h"

// System-wide function call (only once per RSCSocket class init)
// To avoid the dumb linker problem, the static variable must declare in .cpp file
bool RSCSocket::m_socket_w_status;

bool RSCSocket::RSCSocketInit_W()
{
	WSADATA wsadata;
	int status;

	// winsock version requested: major 2, minor 0
	status = WSAStartup(MAKEWORD(2, 0), &wsadata);
	if(status != 0 )
	{
		// winsock version does not match. Error returned.
		m_socket_w_status = false;
	}

	m_socket_w_status = true;

	return m_socket_w_status;
}

void RSCSocket::RSCSocketUninst_W()
{
	if (m_socket_w_status)
	{
		WSACleanup();
	}
}

RSCSocket::RSCSocket()
{
	m_socket = -1;
}

RSCSocket::RSCSocket(SOCKET sock)
{
	m_socket = sock;
}

RSCSocket::~RSCSocket()
{

}

bool RSCSocket::RSCSend(char *buf, const unsigned int buflen)
{
	int numsnt;

	numsnt = send(m_socket, buf, buflen, NO_FLAGS_SET);
	
	if(numsnt != buflen){
		return false;
	}

	return true;
}

bool RSCSocket::RSCReceive(char *buf, const unsigned int buflen)
{
	int numrcv;
	unsigned int partlen;
	
	partlen = buflen;
	while (partlen > 0)
	{
		numrcv = recv(m_socket, buf, partlen, NO_FLAGS_SET);
		if((numrcv == 0) || (numrcv == SOCKET_ERROR) || (numrcv < 0))
		{
			return false;
		}
		
		buf += numrcv;
		partlen -= numrcv;
	}

	return true;
}

bool RSCSocket::RSCCreateStream(bool isNagleOn) // equivalent to create function
{

	// Create the socket
	// Check that the old socket was closed
	if (m_socket >= 0)
		RSCClose();

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket == INVALID_SOCKET)
    {
		m_status = WSACleanup();
		return false;
    }

	// If bopt is non-zero value, it indicates that the option is set in
	// setsockeopt() function - fourth parameter.
	const int bopt = 1;

	// Let the system reuse the address even if the port is in use (TIME_WAIT state)
	// In this way, the application server can re-bind to the same address even though
	// the server is restarted (right after it has been shutdown).
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&bopt, sizeof(bopt)))
    {
      return false;
    }

	if(isNagleOn)
	if(!isNagleOn){
		// Disable Nagle's algorithm for interactive network session. (client/server env.)
		if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&bopt, sizeof(bopt)))
		{
			return false;
		}
	}

	return true;
}

SOCKET RSCSocket::RSCAccept(void)
{
	SOCKET accepted_socket;
	SOCKADDR_IN accepted_addr;
	
	int socket_addr_len = sizeof(SOCKADDR_IN);

	accepted_socket = accept(m_socket, (LPSOCKADDR) &accepted_addr, &socket_addr_len);
	
	if(accepted_socket == INVALID_SOCKET)
		return -1;

	return accepted_socket;
}

bool RSCSocket::RSCConnect(u_short ori_port, char *specific_ip)
{
	SOCKADDR_IN client_addr;
	unsigned long specific_addr;
	int status;
	
	specific_addr = inet_addr(specific_ip);

	if(specific_addr == INADDR_NONE){
		LPHOSTENT host;
		host = gethostbyname(specific_ip);

		if(host == NULL){
			// Error while resolving the host name
			return false;
		}
		specific_addr = ((LPIN_ADDR) host->h_addr)->s_addr;
	}

	memcpy(&client_addr.sin_addr, &specific_addr, sizeof(specific_addr));

	client_addr.sin_port = htons(ori_port);
	client_addr.sin_family = AF_INET;

	status=connect(m_socket, (LPSOCKADDR) &client_addr, sizeof(client_addr));
	if (status == SOCKET_ERROR)
	{
		RSCClose();
		return false;
	}

	return true;
}

bool RSCSocket::RSCBind(u_short ori_port, char *specific_ip)
// If you want to allow the server to bind any ip address,
// make sure to assign NULL value to the 2nd parameter.
{
	SOCKADDR_IN server_addr;
	unsigned long specific_addr;
	int status;

	if(specific_ip == NULL){
        memset(&server_addr, 0 , sizeof(server_addr));
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}else{
		// inet_addr does not support IPv6 - specific_ip format is IPv4 only
		specific_addr = inet_addr(specific_ip);
		if(specific_addr == INADDR_NONE)
			return false;
		memcpy(&server_addr.sin_addr, &specific_addr, sizeof(specific_addr));
	}
	server_addr.sin_port = htons(ori_port);
	server_addr.sin_family = AF_INET;

	// bind function call
	status = bind(m_socket, (LPSOCKADDR) &server_addr, sizeof(server_addr));
	
	if(status == SOCKET_ERROR)
		return false;

	return true;
}

bool RSCSocket::RSCListen(int n)
// n is a maximum length of the queue of pending connections.
// You can specify SOMAXCONN to set a reasonable maximum value.
{
	int status;

	status = listen(m_socket, n);
	if(status == SOCKET_ERROR)
		return false;

	return true;}
	
bool RSCSocket::RSCClose(void)
{
	int status;

	RSCShutdown(SD_BOTH);

	status = closesocket(m_socket);
	if(status == SOCKET_ERROR){
		// Do Not Cleanup in Multithread environment
		//
		//status = WSACleanup();
		return false;
	}

	return true;
}

bool RSCSocket::RSCShutdown(int how)
{
	int status;

	status = shutdown(m_socket, how);
	if(status == SOCKET_ERROR)
		return false;

	return true;
}

bool RSCSocket::RSCReceiveSetTimeOut(int t)
//If a send or receive operation times out on a socket, the socket state is indeterminate, 
//and should not be used; TCP sockets in this state have a potential for data loss, 
//since the operation could be canceled at the same moment the operation was to be completed.
{
	int status;

	// t = 1000 --> 1 sec.
	status = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &t, sizeof(t));

	if(status == SOCKET_ERROR)
			return false;

	return true;
}

bool RSCSocket::RSCSendSetTimeOut(int t)
{
	int status;

	// t = 1000 --> 1 sec.
	status = setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char *) &t, sizeof(t));

	if(status == SOCKET_ERROR)
			return false;

	return true;
}

bool RSCSocket::RSCNagleOption(bool on)
{
	int status;
	int bopt = 0; // 1: true - 0: false

	// If Nagle is on (true), bopt = 0;
	if(on)	bopt = 0;
	else bopt = 1;

	// If bopt = 1, disable the Nagle algorithm
	status = setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&bopt, sizeof(bopt));

	if(status == SOCKET_ERROR)
			return false;

	return true;
}