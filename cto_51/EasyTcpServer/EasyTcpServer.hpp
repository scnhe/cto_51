#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#ifdef _WIN32

#ifndef FD_SETSIZE//��server.cpp����ǰ����windsock2.h ��������ĺ궨��ʧЧ
#define FD_SETSIZE 2506
#endif // !FD_SETSIZE

#include<WinSock2.h>
#include<Windows.h>
#pragma comment(lib,"ws2_32.lib")
#include"..\EasyTcpClient\MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include<atomic>
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
#include<thread>
#include<mutex>
#include<map>
class  ClientSocket 
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));//������Ǽ�������
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
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
	//����ָ��Socket����
	int SendData( DataHeader *header)
	{
		if ( header)
		{
			return send(_sockfd, (const char *)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
	
	SOCKET _sockfd;// fd_set file descr set
	char _szMsgBuf[RECV_BUFF_SIZE * 5];
	//��������,����ճ��
	//��Ϣ����������β��λ��
	int _lastPos;
};
class INetEvent
{
public:
	//�ͻ��˼���
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//�ͻ����뿪�¼�
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;


};


class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	//�ر�
	void setEventObj(INetEvent *event)
	{
		_pNetEvent = event;
	}
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}

			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			close(_sock);

#endif
			_clients.clear();
		}

		//	_sock = INVALID_SOCKET;
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//����������Ϣ
	fd_set _fdRead_bak;
	bool _clients_change;
	SOCKET _maxSock;
	bool OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			//�ӻ������ͻ�����ȡ���ͻ�����
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
		//			_clients.push_back(pClient);
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//û�пͻ�����Ҫ����
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			
			//�������׽��� BSD socket
			fd_set fd_read;
			//������
			FD_ZERO(&fd_read);
			if (_clients_change)
			{
				_clients_change = false;
				//����������socket�����뼯��
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter :_clients)
				{
					FD_SET(iter.second->sockfd(), &fd_read);//����64���޷����뵽set��
															//		std::cout << "fd_set num is " << fd_read.fd_count << "\n";
					if (_maxSock<iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}

				}
				memcpy(&_fdRead_bak, &fd_read, sizeof(fd_set));
			}
			else
			{
				memcpy(&fd_read,&_fdRead_bak,sizeof(fd_set));
			}
			
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = {0,0};
			int ret = select(_maxSock + 1, &fd_read, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				std::cout << "select��������� " << "\n";
				Close();
		//		
				return false;
			}
			else if (ret == 0)
			{
				continue;
			}
		//	std::cout << "select���� " << "\n";
#ifdef _WIN32
			for (int n = 0;n<fd_read.fd_count; n++)
			{
				auto iter = _clients.find(fd_read.fd_array[n]);
				if (-1 == RecvData(iter->second))
				{
					_clients_change = true;					
					if (_pNetEvent)
						_pNetEvent->OnNetLeave(iter->second);
					//	delete iter.second;
					_clients.erase(iter->first);
				}
			}

#else
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fd_read))
				{
					//		std::cout << "select����xxxxx " << "\n";
					//	ret = ;
					if (-1 == RecvData(iter.second))
					{
						_clients_change = true;
						temp.push_back(iter.second);
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter.second);
						//	delete iter.second;
						_clients.erase(iter.first);


					}
				}
			}
			for (auto iter : temp)
			{
				_clients.erase(iter->sockfd());
				delete iter;
			}
#endif // _WIN32
			std::vector<ClientSocket *>temp;
			
			
			//	std::cout << "����ʱ�䴦����������" << "\n";
			//return true;
		}

		return false;

	}
	char _szRecv[RECV_BUFF_SIZE] = {};//�ڶ���������˫����
	//��������,����ճ��
	int RecvData(ClientSocket  *pClient)
	{

		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		//
		if (nLen < 0)
		{
			//printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nLen);
		//	DataHeader *header = (DataHeader *)_szMsgBuf;
		pClient->setLastPos(pClient->getLast() + nLen);
//		int a = sizeof(DataHeader);
		while (pClient->getLast() >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ������ݳ���
			if (pClient->getLast() >= header->dataLength)
			{
			//	std::cout << "package ... " << pClient->sockfd() << "\n";
				//ʣ�����ݳ���
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient, header);
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
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		
		std::lock_guard<std::mutex>lock(_mutex);
	//	std::cout << "cellServer add ClientSocket..." << std::this_thread::get_id() << "\n";
		_clientsBuff.push_back(pClient);
	}
	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
	}
	size_t getClientCount()
	{
		return _clients.size() +_clientsBuff.size();
	}
private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::map<SOCKET,ClientSocket *>_clients;
	
	//����ͻ�����
	std::vector<ClientSocket *>_clientsBuff;
    std::mutex _mutex;
	std::thread _thread;
	//�����¼�����
	INetEvent* _pNetEvent;
};

#define _CellServer_THREAD_COUNT 4
class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	//��Ϣ��������ڲ��ᴴ���߳�
	std::vector<CellServer*> _cellServers;
	//ÿ����Ϣ��ʱ
	CELLTimestamp _tTime;
protected:
	//�յ���Ϣ����
	std::atomic_int _recvCount;
	//�ͻ��˼���
	std::atomic_int _clientCount;
public:
	EasyTcpServer()
	{
		//��ʼ�����绷��
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
	//	_recvCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
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
			std::cout << "Socket�رվ����ӡ�����" << "\n";
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			std::cout << "�����SOCKET��������" << "\n";
		//	return INVALID_SOCKET;
		}
		else {
			std::cout << "�����SOCKET�����ɹ�������" << "\n";
		}
		return _sock;

	}
	//��
	int Bind(const char *ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
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
		int ret = bind(_sock, (sockaddr *)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			std::cout << "�˿ڰ󶨴���	" << "\n";
		}
		else
		{
			std::cout << "�˿ڰ󶨳ɹ�" << port<<"\n";
		}
		
		return ret;

	}

	//����
	int Listen(int backlog)
	{

		int ret = listen(_sock, backlog);
		if (ret == SOCKET_ERROR)
		{
			std::cout << "��������" << "\n";
		}
		else {
			std::cout << "��������˿ڳɹ�" << "\n";
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
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
			std::cout << "Socket=<" << _clientSock << ">��Ч��Socket" << "\n";
			//send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else {
	//		std::cout << "��ӭ�¿ͻ��˼��룺" << inet_ntoa(_client.sin_addr) << " " << _clientSock << "\n";
	//		NewUserJoin userJoin;
	//		userJoin.SocketId = _clientSock;
	//		SendDataToAll(&userJoin);
	//		_clients.push_back(new ClientSocket(_clientSock));
			addClientToCellServer(new ClientSocket(_clientSock));
			
		}
		
		return _clientSock;

	}
	void addClientToCellServer(ClientSocket *pClient)
	{
		//���ҿͻ��������ٵ�CellServer��Ϣ�������
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
		
	}

	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//ע�������¼����ܶ���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
	}
	//�ر�

	void Close()
	{
		//�ر�Win sock 2.x����
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//�ر��׽���closesocket
			closesocket(_sock);

			WSACleanup();
#else
			//�ر��׽���closesocket
			close(_sock);

#endif
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			//������
			FD_ZERO(&fdRead);
			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 0,10};
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				std::cout << "select��������� " << "\n";
				Close();
				return false;
			}
			//        std::cout << "noblock" << "\n";
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			//	std::cout << "����ʱ�䴦����������" << "\n";
			return true;
		}

		return false;

	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//���㲢���ÿ���յ���������Ϣ
	void time4msg()
	{
	//	_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n", _cellServers.size(), t1, _sock,(int)_clientCount, (int)(_recvCount/ t1));
			_recvCount = 0;
			_tTime.update();
		}	

	}
	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_recvCount++;
	}
};

#endif // !_EasyTcpServer_hpp_
