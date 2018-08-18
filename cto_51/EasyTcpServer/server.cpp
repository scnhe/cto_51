#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>
int main()
{
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	_sin.sin_port = htons(7856);

	if(SOCKET_ERROR == bind(sock, (sockaddr *)&_sin, sizeof(_sin)))
	{
		std::cout << "bind port 7856 error\n";
	}
	else {
		std::cout << "bind port 7856 success\n";
	}
	listen(sock, 5);
	sockaddr_in _client = {};
	int nAddrLen = sizeof(_client);// = 5;
	char msgBuf[] = "I'm server";
	SOCKET _clientSock = accept(sock, (sockaddr *)&_client, &nAddrLen);
	if (INVALID_SOCKET == _clientSock)
	{
		std::cout << "Client " << inet_ntoa(_client.sin_addr) << std::endl;
		send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
	}
	char recvBuf[128] = {};
	while (true)
	{
		int nLen = recv(_clientSock, recvBuf, 128, 0);
		if (nLen <= 0)
		{
			break;
		}
		//´¦ÀíÇëÇó
		if (0 == strcmp(recvBuf,"getName"))
		{
			char _buf[] = "Liu";
			send(_clientSock, _buf, strlen(_buf) + 1, 0);
		
		}
		else if (0 == strcmp(recvBuf, "getAge"))
		{
			char _buf[] = "30";
			send(_clientSock, _buf, strlen(_buf) + 1, 0);

		}
		else {
			char _buf[] = "???";
			send(_clientSock, _buf, strlen(_buf) + 1, 0);

		
		}
		

		
	}
	
	closesocket(sock);

	WSACleanup();

}