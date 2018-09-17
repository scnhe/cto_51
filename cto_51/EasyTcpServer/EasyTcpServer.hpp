#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#ifdef _WIN32

#ifndef FD_SETSIZE//在server.cpp中提前引入windsock2.h 导致这里的宏定义失效
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
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));//这个忘记加括号了
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
	//发送指定Socket数据
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
	//接收数据,处理粘包
	//消息缓冲区数据尾部位置
	int _lastPos;
};
class INetEvent
{
public:
	//客户端加入
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	//客户端消息事件
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

	//关闭
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
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//处理网络消息
	fd_set _fdRead_bak;
	bool _clients_change;
	SOCKET _maxSock;
	bool OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			//从缓冲区客户队列取出客户数据
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
			//没有客户端需要处理
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			
			//伯克利套接字 BSD socket
			fd_set fd_read;
			//清理集合
			FD_ZERO(&fd_read);
			if (_clients_change)
			{
				_clients_change = false;
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter :_clients)
				{
					FD_SET(iter.second->sockfd(), &fd_read);//超过64个无法加入到set中
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
			
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = {0,0};
			int ret = select(_maxSock + 1, &fd_read, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				std::cout << "select任务结束！ " << "\n";
				Close();
		//		
				return false;
			}
			else if (ret == 0)
			{
				continue;
			}
		//	std::cout << "select任务！ " << "\n";
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
					//		std::cout << "select任务！xxxxx " << "\n";
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
			
			
			//	std::cout << "空闲时间处理其他数据" << "\n";
			//return true;
		}

		return false;

	}
	char _szRecv[RECV_BUFF_SIZE] = {};//第二缓冲区，双缓冲
	//接收数据,处理粘包
	int RecvData(ClientSocket  *pClient)
	{

		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		//
		if (nLen < 0)
		{
			//printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLast(), _szRecv, nLen);
		//	DataHeader *header = (DataHeader *)_szMsgBuf;
		pClient->setLastPos(pClient->getLast() + nLen);
//		int a = sizeof(DataHeader);
		while (pClient->getLast() >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader *)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于数据长度
			if (pClient->getLast() >= header->dataLength)
			{
			//	std::cout << "package ... " << pClient->sockfd() << "\n";
				//剩余数据长度
				int nSize = pClient->getLast() - header->dataLength;
				OnNetMsg(pClient, header);
				//消息缓冲区数据迁移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);// = nSize;
			}
			else {//剩余数据不足一个完整消息
				break;
			}
		}
		return 0;
	}
	//响应网络消息
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
	//正式客户队列
	std::map<SOCKET,ClientSocket *>_clients;
	
	//缓冲客户队列
	std::vector<ClientSocket *>_clientsBuff;
    std::mutex _mutex;
	std::thread _thread;
	//网络事件对象
	INetEvent* _pNetEvent;
};

#define _CellServer_THREAD_COUNT 4
class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	std::vector<CellServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
protected:
	//收到消息计数
	std::atomic_int _recvCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer()
	{
		//初始化网络环境
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
	//	_recvCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化socket
	SOCKET InitSocket()
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
			std::cout << "服务端SOCKET建立出错" << "\n";
		//	return INVALID_SOCKET;
		}
		else {
			std::cout << "服务端SOCKET建立成功。。。" << "\n";
		}
		return _sock;

	}
	//绑定
	int Bind(const char *ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind 绑定用于接受客户端连接的网络端口
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
			std::cout << "端口绑定错误	" << "\n";
		}
		else
		{
			std::cout << "端口绑定成功" << port<<"\n";
		}
		
		return ret;

	}

	//监听
	int Listen(int backlog)
	{

		int ret = listen(_sock, backlog);
		if (ret == SOCKET_ERROR)
		{
			std::cout << "监听错误" << "\n";
		}
		else {
			std::cout << "监听网络端口成功" << "\n";
		}
		return ret;
	}

	//接受客户端连接
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
			std::cout << "Socket=<" << _clientSock << ">无效的Socket" << "\n";
			//send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else {
	//		std::cout << "欢迎新客户端加入：" << inet_ntoa(_client.sin_addr) << " " << _clientSock << "\n";
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
		//查找客户数量最少的CellServer消息处理对象
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
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
	}
	//关闭

	void Close()
	{
		//关闭Win sock 2.x环境
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//关闭套节字closesocket
			closesocket(_sock);

			WSACleanup();
#else
			//关闭套节字closesocket
			close(_sock);

#endif
		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			//清理集合
			FD_ZERO(&fdRead);
			//将描述符（socket）加入集合
			FD_SET(_sock, &fdRead);
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0,10};
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				std::cout << "select任务结束！ " << "\n";
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
			//	std::cout << "空闲时间处理其他数据" << "\n";
			return true;
		}

		return false;

	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//计算并输出每秒收到的网络消息
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
	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_recvCount++;
	}
};

#endif // !_EasyTcpServer_hpp_
