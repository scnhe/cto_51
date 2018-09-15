
#include<iostream>
#include<thread>

#include"EasyTcpClient.hpp"

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		
		char cmd[32];
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			std::cout << "退出" << std::endl;
		//	client->Close();
			g_bRun = false;
			break;
		}
		else if (0 == strcmp(cmd, "login"))
		{
			Login login;
			strcpy(login.userName, "liu");
			strcpy(login.passWord, "12345");
		//	client->SendData(&login);
			//csend(sock, (char *)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmd, "logout"))
		{
			LogOut logout;// = { "liu" };  
			strcpy(logout.userName, "liu");
		//	client->SendData(&logout);
			//send(sock, (char *)&logout, sizeof(LogOut), 0);
		}
		else {
			std::cout << "不支持的命令" << std::endl;
		}

	}
	
	
}
int main()
{
	
	const int cCount = 2000;
	EasyTcpClient *c1[cCount];
	for (int i = 0; i < cCount; i++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		c1[i] = new EasyTcpClient;
	}
	for(int i = 0;i<cCount;i++)
	{ 
		if (!g_bRun)
		{
			return 0;
		}
			c1[i]->Connect("192.168.3.3", 7856);
			std::cout << " 当前连接数为:" << i << std::endl;
	}
	
	int a = sizeof(EasyTcpClient);
	std::thread t1(cmdThread);
	t1.detach();
	Login log;
	strcpy(log.userName, "ccc");
	strcpy(log.passWord, "ddd");
	int b = sizeof(Login);
	while (g_bRun)
	{
		for (int i = 0; i < cCount; i++)
		{
			c1[i]->SendData(&log);
			c1[i]->OnRun();
			
		}
			
	//	Sleep(1000);
		
	}
	for (int i = 0; i<cCount; i++)
	c1[i]->Close();
	
	std::cout << "clients exit finished" << std::endl;
	return 0;
//	int fpid = fork();

}

