#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

#ifdef _WIN32
#define FD_SETSIZE 1024
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
		Close();
	}
	//��ʼ��socket
	void InitSocket()
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
		//	return -1;
		}
		else {
		//	std::cout << "����Socket�ɹ�������" << std::endl;
		}
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
		if (_sock != INVALID_SOCKET)
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
		
		
	}

	//��������
	int ncount = 0;
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
			
//			std::cout << " select ret = " << ret << " count = " << ncount++ << std::endl;
			if (ret < 0)
			{
				std::cout << "�������1" << std::endl;
				Close();
				return false;
				//	break;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					std::cout << "�������2" << std::endl;
					Close();
					return false;
					//	break;
				}
			}
			return true;
			//	std::cout << "����ʱ�䴦����������" << std::endl;
		}

		return false;

	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//���ջ�����

	char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ�����������Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//��������,����ճ��
	//��Ϣ����������β��λ��
	int _lastPos = 0;
	int RecvData(SOCKET _cSock)
	{
		
		int nLen = (int)recv(_cSock, _szRecv, RECV_BUFF_SIZE, 0);
//		std::cout << "Recv Data from srv is " << nLen << std::endl;
//		return 0;
		
		if (nLen <= 0)
		{
			std::cout << "��������Ͽ�����" << nLen<<std::endl;
		//	Close();
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(_szMsgBuf+_lastPos, _szRecv, nLen);
	//	DataHeader *header = (DataHeader *)_szMsgBuf;
		_lastPos +=nLen;
		while(_lastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)_szMsgBuf;
			//�ж���Ϣ�����������ݳ��ȴ������ݳ���
			if (_lastPos >= header->dataLength)
			{
				//ʣ�����ݳ���
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				//��Ϣ����������Ǩ��
				memcpy(_szMsgBuf, _szMsgBuf+header->dataLength, nSize);
				_lastPos = nSize;
			}
			else {//ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
//		recv(_cSock, _szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
//		OnNetMsg( header);
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
		//	std::cout << _sock << " �յ�������������Ϣ:" << "CMD_LOGIN_RESULT" << result->result << "���ݳ��ȣ�"<<result->dataLength<<std::endl;


		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogOutResult *result = (LogOutResult *)header;
		//	std::cout << _sock << " �յ�������������Ϣ:" << "CMD_LOGOUT_RESULT " << result->result << "���ݳ��ȣ�" << result->dataLength<<std::endl;

		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin *result = (NewUserJoin *)header;
		//	std::cout << _sock << " �յ�������������Ϣ:" << "�����û�����---> " << result->SocketId << result->dataLength << std::endl;
		}
		break;
		case CMD_ERROR:
		{
			std::cout << "�յ�������Ϣ " << _sock << " ���ݳ���Ϊ " << header->dataLength << std::endl;
		}
		break;
		default:
		{
			std::cout << "�յ�δ����������Ϣ,��Ϣ����Ϊ��" << header->dataLength<<std::endl;
		}
		break;
		}
	
	}

	
	//��������
	int SendData(DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char *)header, header->dataLength, 0);
		}
		else
		{
			return SOCKET_ERROR;
		}
	}
	

private:

};




#endif //_EASYTCPCLIENT_HPP