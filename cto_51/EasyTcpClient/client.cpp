
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>
int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	_sin.sin_port = htons(7856);

	if (SOCKET_ERROR == connect(sock, (sockaddr *)&_sin, sizeof(_sin)))
	{
		std::cout << "Connect error	" << std::endl;
	}
	else
	{
		char buf[128];
		int len=128;
		int key = recv(sock, buf, len, 0);
		std::cout << "recv = " << buf << " " << key << std::endl;
	}
	getchar();
	closesocket(sock);
	getchar();
	WSACleanup();
	return 0;

}