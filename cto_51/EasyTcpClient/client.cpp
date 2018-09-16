
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
const int cCount = 1000;
const int tCount = 4;
EasyTcpClient *c1[cCount];
void sendThread(int id)//14
{
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;
	for (int i = begin; i < end; i++)
	{
	
		c1[i] = new EasyTcpClient;
	}
	for (int i = begin; i<end; i++)
	{		
		c1[i]->Connect("192.168.3.3", 7856);
		std::cout <<"Thread<"<<id<< ">  当前连接数为:" << i << "\n";
	}

	int a = sizeof(EasyTcpClient);
	Login log;
	strcpy(log.userName, "ccc");
	strcpy(log.passWord, "ddd");
	int b = sizeof(Login);
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			c1[i]->SendData(&log);
		//	c1[i]->OnRun();

		}
	}
	for (int i = begin; i<end; i++)
		c1[i]->Close();
}
int main()
{
	//ui线程
	std::thread t1(cmdThread);
	t1.detach();

	
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();
	}
	while (g_bRun)
	{
		Sleep(100);
	}
	std::cout << "clients exit finished" << std::endl;
	return 0;
//	int fpid = fork();

}

