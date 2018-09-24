#include<iostream>
#include<thread>

#include"EasyTcpClient.hpp"
#include"..\EasyTcpServer\CELLTimestamp.hpp"
#include<atomic>
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
const int cCount = 5000;
const int tCount = 4;
EasyTcpClient *c1[cCount];
//int n = sizeof(EasyTcpClient);
std::atomic_int	secondCount = 0;
std::atomic_int	readCount = 0;
void sendThread(int id)//14
{
	printf("thread<%d>,start\n", id);
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
	}
	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);
	readCount++;
	while (readCount < tCount)
	{//等待其他线程准备结束
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);

	}
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
			
			if (SOCKET_ERROR != c1[i]->SendData(log, nLen))
			{
				secondCount++;
			}
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
	CELLTimestamp tTimer;
	while (g_bRun)
	{
		auto t = tTimer.getElapsedSecond();
		if (t >= 1.0)
		{
		//	printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n",tCount,cCount,t,secondCount);
			std::cout << "thread<" << tCount << ">,clients<" << cCount << ">,time<" << t << ">,send<" << secondCount << ">\n";
			tTimer.update();
			secondCount = 0;
		}
		Sleep(1);
	}
		
	

	printf("已退出。\n");
	return 0;
//	int fpid = fork();

}

