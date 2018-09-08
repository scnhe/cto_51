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
		//��ʼ�����绷��
		_sock = INVALID_SOCKET;
	
	}
	virtual ~EasyTcpClient()
	{
		if (INVALID_SOCKET != _sock)
		{
			Close();
		}
	
	}
	//��ʼ��socket
	int InitSocket()
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
			std::cout << "���󣬽���Socketʧ�ܡ�����" << std::endl;
			return -1;
		}
		else {
			std::cout << "����Socket�ɹ�������" << std::endl;
		}
		return 0;

	}
	//���ӷ�����
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
			std::cout << "�ͻ���" << _sock << "���ӷ������ɹ�" << std::endl;
		}
		return 0;

	}

	//�ر�

	void Close()
	{
		//�ر�Win sock 2.x����
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif // _WIN32
		_sock = INVALID_SOCKET;
	}
	//��������
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

	//��������,����ճ��
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			std::cout << "��������Ͽ�����" << std::endl;
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg( header);
		return 0;
	}
	//��Ӧ������Ϣ
	void OnNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *result = (LoginResult *)header;
			std::cout << _sock << " �յ�������������Ϣ:" << "Login--->" << result->result << std::endl;


		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogOutResult *result = (LogOutResult *)header;
			std::cout << _sock << " �յ�������������Ϣ:" << "LogOut---> " << result->result << std::endl;

		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *result = (NewUserJoin *)header;
			std::cout << _sock << " �յ�������������Ϣ:" << "�����û�����---> " << result->SocketId << std::endl;
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

	//��������
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
				std::cout << "�������" << std::endl;
				//	break;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					std::cout << "�������" << std::endl;
					//	break;
				}
			}
		//	std::cout << "����ʱ�䴦����������" << std::endl;
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