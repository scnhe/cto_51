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
			std::cout << "退出" << "\n";
			g_bRun = false;
			break;
		}
		else {
			std::cout << "不支持的命令" << std::endl;
		}

	}
	
	
}
const int cCount = 8;
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
		printf("thread<%d>,Connect=%d\n", id,i);
	}
	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);

	Login log[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(log[n].userName, "ccc");
		strcpy(log[n].passWord, "ddd");
	}
	
	const int nLen = sizeof(log);
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
		//	std::cout << "Sending ..." << std::endl;
			c1[i]->SendData(log,nLen);
		//	_sleep(2000);
			c1[i]->OnRun();

		}
	}
	for (int n = begin; n < end; n++)
	{
		c1[n]->Close();
        delete c1[n];
	}
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
		Sleep(100);

	printf("已退出。\n");
	return 0;
//	int fpid = fork();

}

