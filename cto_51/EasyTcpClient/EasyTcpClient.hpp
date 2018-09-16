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
	bool _isConnect;
public:
	EasyTcpClient()
	{
		//初始化网络环境
		_sock = INVALID_SOCKET;
		_isConnect = false;
	}
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void InitSocket()
	{
		//启动Win socket 2.x 环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		if (INVALID_SOCKET != _sock)
		{
			std::cout << "Socket关闭旧连接。。。" << "\n";
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			std::cout << "错误，建立Socket失败。。。" << "\n";
		//	return -1;
		}
		else {
		//	std::cout << "建立Socket成功。。。" << "\n";
		}
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
         ret = connect(_sock, (sockaddr *)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret )
		{
			std::cout << "Connect error	" << "\n";
			return -1;
		}
		else
		{
		    _isConnect = true;
		//	std::cout << "客户端" << _sock << "连接服务器成功" << "\n";
		}
		return ret;

	}

	//关闭

	void Close()
	{
		if (_sock != INVALID_SOCKET)
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
		_isConnect = false;
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t); 
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				std::cout << "任务结束1" << "\n";
				Close();
				return false;
				//	break;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					std::cout << "任务结束2" << "\n";
					Close();
					return false;
					//	break;
				}
			}
			return true;
			//	std::cout << "空闲时间处理其他数据" << "\n";
		}

		return false;

	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET && _isConnect;
	}
	//接收缓冲区

	char _szRecv[RECV_BUFF_SIZE] = {};
	//第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 5] = {};
	//接收数据,处理粘包
	//消息缓冲区数据尾部位置
	int _lastPos = 0;
	int RecvData(SOCKET _cSock)
	{
		
		int nLen = (int)recv(_cSock, _szRecv, RECV_BUFF_SIZE, 0);
//		std::cout << "Recv Data from srv is " << nLen << "\n";
//		return 0;
		
		if (nLen <= 0)
		{
			std::cout << "与服务器断开连接" << nLen<<"\n";
		//	Close();
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf+_lastPos, _szRecv, nLen);
	//	DataHeader *header = (DataHeader *)_szMsgBuf;
		_lastPos +=nLen;
		while(_lastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)_szMsgBuf;
			//判断消息缓冲区的数据长度大于数据长度
			if (_lastPos >= header->dataLength)
			{
				//剩余数据长度
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				//消息缓冲区数据迁移
				memcpy(_szMsgBuf, _szMsgBuf+header->dataLength, nSize);
				_lastPos = nSize;
			}
			else {//剩余数据不足一个完整消息
				break;
			}
		}
//		recv(_cSock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
//		OnNetMsg( header);
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
		//	std::cout << _sock << " 收到服务器返回消息:" << "CMD_LOGIN_RESULT" << result->result << "数据长度："<<result->dataLength<<"\n";


		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogOutResult *result = (LogOutResult *)header;
		//	std::cout << _sock << " 收到服务器返回消息:" << "CMD_LOGOUT_RESULT " << result->result << "数据长度：" << result->dataLength<<"\n";

		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *result = (NewUserJoin *)header;
		//	std::cout << _sock << " 收到服务器返回消息:" << "有新用户加入---> " << result->SocketId << result->dataLength << "\n";
		}
		break;
		case CMD_ERROR:
		{
			std::cout << "收到错误消息 " << _sock << " 数据长度为 " << header->dataLength << "\n";
		}
		break;
		default:
		{
			std::cout << "收到未定义类型消息,消息长度为：" << header->dataLength<<"\n";
		}
	
		}
	
	}

	
	//发送数据
	int SendData(DataHeader *header,int nLen)
	{
	    int ret = SOCKET_ERROR;
		if (isRun() && header)
		{
			ret =  send(_sock, (const char *)header, nLen, 0);
			if(SOCKET_ERROR ==ret)
			{
				Close();
			}
		}
		return ret;
	}
	

private:

};




#endif //_EASYTCPCLIENT_HPP