
#include<iostream>
#include<thread>

#include"EasyTcpClient.hpp"

bool g_bRun = true;
void cmdThread(EasyTcpClient *client)
{
	while (true)
	{
		
		char cmd[32];
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			std::cout << "退出" << std::endl;
			client->Close();
			g_bRun = false;
			break;
		}
		else if (0 == strcmp(cmd, "login"))
		{
			Login login;
			strcpy(login.userName, "liu");
			strcpy(login.passWord, "12345");
			client->SendData(&login);
			//csend(sock, (char *)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmd, "logout"))
		{
			LogOut logout;// = { "liu" };  
			strcpy(logout.userName, "liu");
			client->SendData(&logout);
			//send(sock, (char *)&logout, sizeof(LogOut), 0);
		}
		else {
			std::cout << "不支持的命令" << std::endl;
		}

	}
	
	
}
int main()
{
	EasyTcpClient c1;
	c1.Connect("192.168.3.85", 7856);
	
	std::thread t1(cmdThread,&c1);	
	t1.detach();
	Login log;
	strcpy(log.userName, "ccc");
	strcpy(log.passWord, "ddd");
	while (c1.isRun())
	{
		c1.OnRun();
		c1.SendData(&log);	
		Sleep(10);
		
	}
	c1.Close();
	
	return 0;
//	int fpid = fork();

}

