#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#ifdef _WIN32
#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include"..\EasyTcpClient\MessageHeader.hpp"
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include"../EasyTcpClient/MessageHeader.hpp"
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif // _WIN32

#include<iostream>
#include<vector>
class EasyTcpServer
{
	SOCKET _sock;
	std::vector<SOCKET>g_clients;
public:
	EasyTcpServer()
	{
		//��ʼ�����绷��
		_sock = INVALID_SOCKET;

	}
	virtual ~EasyTcpServer()
	{
		//if (INVALID_SOCKET != _sock)
		//{
			Close();
	//	}

	}
	//��ʼ��socket
	SOCKET InitSocket()
	{
		//����Win socket 2.x ����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32
		if (INVALID_SOCKET != _sock)
		{
			std::cout << "Socket�رվ����ӡ�����" << std::endl;
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			std::cout << "�����SOCKET��������" << std::endl;
		//	return INVALID_SOCKET;
		}
		else {
			std::cout << "�����SOCKET�����ɹ�������" << std::endl;
		}
		return _sock;

	}
	//��
	int Bind(const char *ip, unsigned short port)
	{
		int ret = 0;
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
#ifdef _WIN32
		if (ip)
		{ 
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {

			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}

#else
		
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {

			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif // _WIN32
		_sin.sin_port = htons(port);

		if (SOCKET_ERROR == bind(_sock, (sockaddr *)&_sin, sizeof(_sin)))
		{
			std::cout << "�˿ڰ󶨴���	" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "�˿ڰ󶨳ɹ�" << port<<std::endl;
		}
		
		return 0;

	}

	//����
	int Listen(int backlog)
	{

		int ret = listen(_sock, backlog);
		if (ret == SOCKET_ERROR)
		{
			std::cout << "��������" << std::endl;
		}
		else {
			std::cout << "��������˿ڳɹ�" << std::endl;
		}
		return ret;
	}

	//��������
	int Accept()
	{
		sockaddr_in _client = {};
		int nAddrLen = sizeof(sockaddr_in);// = 5;
		char msgBuf[] = "I'm server";
		SOCKET _clientSock = INVALID_SOCKET;
#ifdef _WIN32
		_clientSock = accept(_sock, (sockaddr *)&_client, &nAddrLen);
#else
		_clientSock = accept(_sock, (sockaddr *)&_client, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == _clientSock)
		{
			std::cout << "Socket=<" << _clientSock << ">��Ч��Socket" << std::endl;
			//send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else {
			std::cout << "��ӭ�¿ͻ��˼��룺" << inet_ntoa(_client.sin_addr) << " " << _clientSock << std::endl;
			NewUserJoin userJoin;
			userJoin.SocketId = _clientSock;
			SendDataToAll(&userJoin);
			g_clients.push_back(_clientSock);
		}
		
		return _clientSock;

	}

	//�ر�

	void Close()
	{
		//�ر�Win sock 2.x����
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (size_t n = g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}

			closesocket(_sock);

			WSACleanup();
#else
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);

#endif
		}

	//	_sock = INVALID_SOCKET;
	}
	//��������
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_except;
			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_except);

			FD_SET(_sock, &fd_read);
			FD_SET(_sock, &fd_write);
			FD_SET(_sock, &fd_except);
			SOCKET maxSock = _sock;
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fd_read);//����64���޷����뵽set��
											   //		std::cout << "fd_set num is " << fd_read.fd_count << std::endl;
				if (maxSock<g_clients[n])
				{
					maxSock = g_clients[n];
				}

			}
			timeval t = { 1,0 };
			//    SOCKET_ERROR

			int ret = select(maxSock + 1, &fd_read, &fd_write, &fd_except, &t);
			if (ret < 0)
			{
				std::cout << "select��������� " << std::endl;
				Close();
				return false;
			}
			//        std::cout << "noblock" << std::endl;
			if (FD_ISSET(_sock, &fd_read))
			{
				FD_CLR(_sock, &fd_read);
				Accept();
			}


			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fd_read))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			//	std::cout << "����ʱ�䴦����������" << std::endl;
			return true;
		}

		return false;

	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//��������,����ճ��
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			std::cout << "��ͻ���" << _cSock << "�Ͽ�����" << std::endl;
			//	Close();
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = NULL;
			login = (Login*)header;
			std::cout << _cSock << " ���յ��ͻ������� CMD_LOGIN " << "UserName: " << login->userName << " Password:" << login->passWord << " ���ݳ��ȣ�" << login->dataLength << std::endl;

			LoginResult result = {};
			result.result = 1;
			send(_cSock, (char *)&result, sizeof(LoginResult), 0);

		}
		break;
		case CMD_LOGOUT:
		{
			LogOut *out = NULL;
			out = (LogOut*)header;
			std::cout << _cSock << " ���յ��ͻ����˳�����:" << "UserName: " << out->userName << std::endl;
			LogOutResult result = {};
			result.result = 2;
			send(_cSock, (char *)&result, sizeof(LogOutResult), 0);
		}
		break;
		default:
		{
			header->cmd = CMD_ERROR;
			header->dataLength = 0;
			send(_cSock, (char *)header, sizeof(DataHeader), 0);
		}
		break;
		}

	}

	//����ָ��Socket����
	int SendData(SOCKET _cSock,DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char *)header, header->dataLength, 0);
		}
		else
		{
			return SOCKET_ERROR;
		}
	}
	//����ָ��Socket����
	int SendDataToAll(DataHeader *header)
	{
		if (isRun() && header)
		{
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				SendData(g_clients[n], header);
			}
            return 0;
		}
		else
		{
			return SOCKET_ERROR;
		}
	}

	

	

private:

};




#endif //_EASYTCPSERVER_HPP
