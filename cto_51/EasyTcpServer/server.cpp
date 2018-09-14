#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif // _WIN32
#include<iostream>
#include<vector>
#include<algorithm>
#include"EasyTcpServer.hpp"
#include<thread>
bool g_bRun = true;
void cmdThread()
{
	while (true)
	{

		char cmd[32];
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			g_bRun = false;
			std::cout << "ÍË³ö" << std::endl;			
			
			return;
		}		

	}


}

int main()
{

	EasyTcpServer s;
	s.InitSocket();
	s.Bind(nullptr,7856);
	s.Listen(10);
	std::thread t(cmdThread);
	t.detach();
	while (g_bRun)
	{
		s.OnRun();
	}
	s.Close();
	std::cout << "main end" << std::endl;
	return 0;
}
