
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
	while (true)
	{
		SOCKET _clientSock = accept(sock, (sockaddr *)&_client, &nAddrLen);
		if (INVALID_SOCKET != _clientSock)
		{
			std::cout <<"Client "<<inet_ntoa(_client.sin_addr)<<std::endl;
			send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}

		
	}
	
	closesocket(sock);

	WSACleanup();

}