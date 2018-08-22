//V1.3
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>
#include<thread>
struct DataPackage
{
	int age;
	char name[32];

};
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN
};
//msg header
struct DataHeader
{
	short dataLength;
	short cmd;

};
struct Login :public DataHeader
{//也可以包含一个DataHeader对象
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];

};
struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;

};
//登出操作
struct LogOut :public DataHeader
{
	LogOut()
	{
		dataLength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}
	char userName[32];

};
//登出结果
struct LogOutResult :public DataHeader
{
	LogOutResult()
	{
		dataLength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
	}
	int result;

};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(LogOutResult);
		cmd = CMD_NEW_USER_JOIN;
		SocketId = 0;
	}
	int SocketId;

};

int process(SOCKET _clientSock)
{
	//接收客户端数据
	char szRecv[1024] = {};

	int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader *)szRecv;
	if (nLen < 0)
	{
		std::cout << "与服务器断开连接：" << _clientSock << std::endl;
		return -1;
	}

	std::cout << "SOCKET:" << _clientSock << " 接收到命令： " << header->cmd << " 数据长度：" << header->dataLength << std::endl;
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LoginResult *result = NULL;
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		result = (LoginResult*)szRecv;
		std::cout << _clientSock << " 收到服务器返回消息:" << "Login--->" << result->result << std::endl;
		

	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogOutResult *result = NULL;
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		result = (LogOutResult*)szRecv;
		std::cout << _clientSock << " 收到服务器返回消息:" << "LogOut---> " << result->result << std::endl;

	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin *result = NULL;
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		result = (NewUserJoin*)szRecv;
		std::cout << _clientSock << " 收到服务器返回消息:" << "有新用户加入---> " << result->SocketId << std::endl;
	}
	break;
	default:
	{
		header->cmd = CMD_ERROR;
		header->dataLength = 0;
		send(_clientSock, (char *)header, sizeof(DataHeader), 0);
	}
	break;
	}
	return 0;
}
bool g_bRun = true;
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char cmd[128] = {};
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			std::cout << "退出" << std::endl;
			g_bRun = false;
			break;
		}
		else if (0 == strcmp(cmd, "login"))
		{
			Login login;
			strcpy(login.userName, "liu");
			strcpy(login.passWord, "12345");
			send(sock, (char *)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmd, "logout"))
		{
			LogOut logout;// = { "liu" };  
			strcpy(logout.userName, "liu");
			send(sock, (char *)&logout, sizeof(LogOut), 0);
		}
		else {
			std::cout << "不支持的命令" << std::endl;
		}

	}
	
	
}
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
		std::cout << "客户端" << sock << "连接服务器成功" << std::endl;
	}
	std::thread t(cmdThread,sock);
//	t.join();
	t.detach();
	while (g_bRun)
	{

		fd_set fdReads;
		fd_set fdWrites;
		fd_set fdExcepts;

		FD_ZERO(&fdReads);
		FD_ZERO(&fdWrites);
		FD_ZERO(&fdExcepts);

		FD_SET(sock, &fdReads);
		timeval t = {0,0};
		int ret = select(sock + 1, &fdReads, &fdWrites, &fdExcepts, &t);
		if (ret < 0)
		{
			std::cout << "任务结束" << std::endl;
			break;
		}
		if(FD_ISSET(sock,&fdReads))
		{ 
			FD_CLR(sock, &fdReads);
			if (-1 == process(sock))
			{
				std::cout << "任务结束" << std::endl;
				break;
			}
		}
		std::cout << "空闲时间处理其他数据" << std::endl;
		
		Sleep(1000);
		/*char recvBuf[128] = {};
		recv(sock, recvBuf, 128, 0);
		DataPackage *pack = (DataPackage *)recvBuf;
		std::cout << "from server:" << pack->age <<" "<<pack->name<< std::endl;*/
	}
//	getchar();
	closesocket(sock);
	getchar();
	WSACleanup();
	return 0;

}

