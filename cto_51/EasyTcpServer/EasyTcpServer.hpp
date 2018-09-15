#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#ifdef _WIN32

#ifndef FD_SETSIZE//��server.cpp����ǰ����windsock2.h ��������ĺ궨��ʧЧ
#define FD_SETSIZE 10024
#endif // !FD_SETSIZE

#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include"..\EasyTcpClient\MessageHeader.hpp"
#include"CELLTimestamp.hpp"
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

class  ClientSocket 
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));//������Ǽ�������
		_lastPos = 0;
	}
	char *msgBuf()
	{
		return _szMsgBuf;
	}
	int getLast()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
private:
	
	SOCKET _sockfd;// fd_set file descr set
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��������,����ճ��
	//��Ϣ����������β��λ��
	int _lastPos;
};
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket *>_clients;
	CELLTimestamp _tTime;
	int _recvCount;
public:
	EasyTcpServer()
	{
		//��ʼ�����绷��
		_sock = INVALID_SOCKET;
		_recvCount = 0;
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
	//		std::cout << "��ӭ�¿ͻ��˼��룺" << inet_ntoa(_client.sin_addr) << " " << _clientSock << std::endl;
	//		NewUserJoin userJoin;
	//		userJoin.SocketId = _clientSock;
	//		SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(_clientSock));
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
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}

			closesocket(_sock);

			WSACleanup();
#else
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);

#endif
			_clients.clear();
		}

	//	_sock = INVALID_SOCKET;
	}
	//��������
	int ncount = 0;
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
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fd_read);//����64���޷����뵽set��
											   //		std::cout << "fd_set num is " << fd_read.fd_count << std::endl;
				if (maxSock<_clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
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
				return true;
			}


			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fd_read))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
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

	char _szRecv[RECV_BUFF_SIZE] = {};//�ڶ���������˫����
	//��������,����ճ��
	int RecvData(ClientSocket  *pClient)
	{
		
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		//
		if (nLen < 0)
		{
			std::cout << "�ͻ����˳����� " << pClient->sockfd() << std::endl;
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nLen);
		//	DataHeader *header = (DataHeader *)_szMsgBuf;
		pClient->setLastPos(pClient->getLast() + nLen);
		while (pClient->getLast() >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ������ݳ���
			if (pClient->getLast() >= header->dataLength)
			{
				//ʣ�����ݳ���
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient->sockfd(),header);
				//��Ϣ����������Ǩ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);// = nSize;
			}
			else {//ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader *header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n", t1, _sock,_clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = NULL;
			login = (Login*)header;
		//	std::cout << _cSock << " ���յ��ͻ������� CMD_LOGIN " << "UserName: " << login->userName << " Password:" << login->passWord << " ���ݳ��ȣ�" << login->dataLength << std::endl;

			LoginResult result = {};
			result.result = 1;
			SendData(_cSock,&result);
		//	send(_cSock, (char *)&result, sizeof(LoginResult), 0);

		}
		break;
		case CMD_LOGOUT:
		{
		//	LogOut *out = NULL;
		//	out = (LogOut*)header;
		//	std::cout << _cSock << " ���յ��ͻ����˳�����:" << "UserName: " << out->userName << std::endl;
		//	LogOutResult result = {};
		//	result.result = 2;
		//	SendData(_cSock, &result);
		}
		break;		
		default:
		{
			std::cout << _cSock << "������յ��ͻ���δ������Ϣ:" << header->dataLength << std::endl;
			DataHeader h;
		//	send(_cSock, (char *)&h, sizeof(DataHeader), 0);
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
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				SendData(_clients[n]->sockfd(), header);
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
