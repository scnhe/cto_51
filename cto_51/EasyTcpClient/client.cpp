
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>
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
	CMD_ERROR
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
		
	}
	
	while (true)
	{
		char cmd[128] = {};
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			break;
		}
		else if (0 == strcmp(cmd, "login")) {
			Login login;
			strcpy(login.userName, "liu");
			strcpy(login.passWord, "12345");
			/*DataHeader header;
			header.cmd = CMD_LOGIN;
			header.dataLength = sizeof(Login);
			send(sock, (char *)&header, sizeof(DataHeader), 0);*/
			send(sock, (char *)&login, sizeof(Login), 0);
			/*send(sock, cmd, strlen(cmd) + 1, 0);*/
			/*recv(sock, (char *)&header, sizeof(DataHeader), 0);*/
			LoginResult result = {};
			recv(sock, (char *)&result, sizeof(LoginResult), 0);
			std::cout << "登陆结果为：" << result.result << std::endl;
		}
		else if (0 == strcmp(cmd, "logout")) {
			/*send(sock, cmd, strlen(cmd) + 1, 0);*/
			LogOut logout;// = { "liu" };
			strcpy(logout.userName, "liu");
			/*DataHeader header = { sizeof(LogOut),CMD_LOGOUT};
			send(sock, (char *)&header, sizeof(DataHeader), 0);*/
			send(sock, (char *)&logout, sizeof(LogOut), 0);
			/*send(sock, cmd, strlen(cmd) + 1, 0);*/
			/*recv(sock, (char *)&header, sizeof(DataHeader), 0);*/
			LogOutResult result = {};
			recv(sock, (char *)&result, sizeof(LogOutResult), 0);
			std::cout << "登出结果为：" << result.result << std::endl;

		}
		else {
			std::cout << "不支持的命令" << std::endl;
		}
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