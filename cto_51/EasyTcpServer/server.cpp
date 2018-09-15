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
