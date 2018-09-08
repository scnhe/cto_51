#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

#ifdef _WIN32
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
#include"MessageHeader.hpp"
class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		//初始化网络环境
		_sock = INVALID_SOCKET;
	
	}
	virtual ~EasyTcpClient()
	{
		if (INVALID_SOCKET != _sock)
		{
			Close();
		}
	
	}
	//初始化socket
	int InitSocket()
	{
		//启动Win socket 2.x 环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		if (INVALID_SOCKET != _sock)
		{
			std::cout << "Socket关闭旧连接。。。" << std::endl;
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			std::cout << "错误，建立Socket失败。。。" << std::endl;
			return -1;
		}
		else {
			std::cout << "建立Socket成功。。。" << std::endl;
		}
		return 0;

	}
	//连接服务器
	int Connect(const char *ip,unsigned short port)
	{
		int ret = 0;
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		_sin.sin_port = htons(port);

		if (SOCKET_ERROR == connect(_sock, (sockaddr *)&_sin, sizeof(_sin)))
		{
			std::cout << "Connect error	" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "客户端" << _sock << "连接服务器成功" << std::endl;
		}
		return 0;

	}

	//关闭

	void Close()
	{
		//关闭Win sock 2.x环境
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif // _WIN32
		_sock = INVALID_SOCKET;
	}
	//发送数据
	int SendData(DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char *)header, header->dataLength - sizeof(DataHeader), 0);
		}
		else
		{
			return SOCKET_ERROR;
		}		
	}

	//接收数据,处理粘包
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			std::cout << "与服务器断开连接" << std::endl;
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg( header);
		return 0;
	}
	//响应网络消息
	void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *result = (LoginResult *)header;
			std::cout << _sock << " 收到服务器返回消息:" << "Login--->" << result->result << std::endl;


		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogOutResult *result = (LogOutResult *)header;
			std::cout << _sock << " 收到服务器返回消息:" << "LogOut---> " << result->result << std::endl;

		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *result = (NewUserJoin *)header;
			std::cout << _sock << " 收到服务器返回消息:" << "有新用户加入---> " << result->SocketId << std::endl;
		}
		break;
		default:
		{
			header->cmd = CMD_ERROR;
			header->dataLength = 0;
			send(_sock, (char *)header, sizeof(DataHeader), 0);
		}
		break;
		}
	
	}

	//处理数据
	bool OnRun() 
	{
		if (isRun())
		{
			fd_set fdReads;
			fd_set fdWrites;
			fd_set fdExcepts;

			FD_ZERO(&fdReads);
			FD_ZERO(&fdWrites);
			FD_ZERO(&fdExcepts);

			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, &fdWrites, &fdExcepts, &t);
			if (ret < 0)
			{
				std::cout << "任务结束" << std::endl;
				//	break;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					std::cout << "任务结束" << std::endl;
					//	break;
				}
			}
		//	std::cout << "空闲时间处理其他数据" << std::endl;
		}
		
		return 0;
	
	}
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

private:

};




#endif //_EASYTCPCLIENT_HPP