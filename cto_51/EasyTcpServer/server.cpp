#include<iostream>
#include<vector>
#include<algorithm>
#include"EasyTcpServer.hpp"
#include<thread>
bool g_bRun = true;
void cmdThread()
{
	while (true)
	{

		char cmd[32];
		scanf("%s", cmd);
		if (0 == strcmp(cmd, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			printf("��֧�ֵ����\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	//	printf("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
	//	_clientCount--;
		EasyTcpServer::OnNetLeave(pClient);
		printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pClient, header);
		
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
	//		LoginResult ret;
	//		pClient->SendData(&ret);
			LoginResult* ret = new LoginResult();
			pCellServer->addSendTask(pClient, ret);
		}//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
		break;
		case CMD_LOGOUT:
		{
			
			LogOut* logout = (LogOut*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->dataLength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}
private:


};

int main()
{

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 7856);
	server.Listen(5);
	server.Start(4);

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		//printf("����ʱ�䴦������ҵ��..\n");
	}
	server.Close();
	printf("���˳���\n");
	getchar();
	return 0;
}
