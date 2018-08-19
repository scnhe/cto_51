#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>

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
struct Login:public DataHeader
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
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	_sin.sin_port = htons(7856);

	if(SOCKET_ERROR == bind(sock, (sockaddr *)&_sin, sizeof(_sin)))
	{
		std::cout << "bind port 7856 error\n";
	}
	else {
		std::cout << "bind port 7856 success\n";
	}
	listen(sock, 5);
	sockaddr_in _client = {};
	int nAddrLen = sizeof(_client);// = 5;
	char msgBuf[] = "I'm server";
	SOCKET _clientSock = accept(sock, (sockaddr *)&_client, &nAddrLen);
	if (INVALID_SOCKET == _clientSock)
	{
		std::cout << "Client " << inet_ntoa(_client.sin_addr) << std::endl;
		send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
	}
	char recvBuf[128] = {};
	while (true)
	{
		//接收客户端数据
		char szRecv[1024] = {};
		
		int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			break;
		}
		std::cout << "接收到命令： " << header->cmd << " 数据长度：" << header->dataLength << std::endl;
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = NULL;
			recv(_clientSock, szRecv+sizeof(DataHeader), header->dataLength-sizeof(DataHeader), 0);
			login = (Login*)szRecv;
			std::cout << "UserName: " << login->userName << std::endl;
			//验证账号密码，此处忽略
			/*DataHeader header = {};
			header.dataLength = sizeof(LoginResult);
			header.cmd = CMD_LOGIN;*/
			LoginResult result = {};
			result.result = 0;
			/*send(_clientSock, (char *)&header, sizeof(DataHeader), 0);*/
			send(_clientSock, (char *)&result, sizeof(LoginResult), 0);
			/*send(_clientSock,)*/
		
		}
		break;
		case CMD_LOGOUT:
		{
			LogOut *out = NULL;
			recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			out = (LogOut*)szRecv;
			//验证账号密码，此处忽略
			/*DataHeader header = {};
			header.dataLength = sizeof(LoginResult);
			header.cmd = CMD_LOGIN;*/
			LogOutResult result = {};
			result.result = 0;
			/*send(_clientSock, (char *)&header, sizeof(DataHeader), 0);*/
			send(_clientSock, (char *)&result, sizeof(LogOutResult), 0);
			/*send(_clientSock,)*/
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
		////处理请求
		//if (0 == strcmp(recvBuf,"getName"))
		//{
		//	char _buf[] = "Liu";
		//	send(_clientSock, _buf, strlen(_buf) + 1, 0);
		//
		//}
		//else if (0 == strcmp(recvBuf, "getAge"))
		//{
		//	char _buf[] = "30";
		//	send(_clientSock, _buf, strlen(_buf) + 1, 0);

		//}
		//else if (0 == strcmp(recvBuf, "getInfo"))
		//{
		//	DataPackage pack = { 35,"Yang" };			
		//	send(_clientSock, (char *)&pack, sizeof(DataPackage), 0);

		//}
		//else {
		//	char _buf[] = "???";
		//	send(_clientSock, _buf, strlen(_buf) + 1, 0);

		//
		//}
		

		
	}
	
	closesocket(sock);

	WSACleanup();

}