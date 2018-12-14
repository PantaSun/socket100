#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      2506
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>

#include"MessageHeader.hpp"


//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

#define _CellServer_THREAD_COUNT 4


class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ��...\n");
		}
		else {
			printf("����<socket=%d>�ɹ�...\n", (int)_sock);
		}
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = (int)bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("����,������˿�<%d>ʧ��...\n", port);
		}
		else {
			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>����,��������˿�ʧ��...\n", _sock);
		}
		else {
			printf("<socket=%d>��������˿ڳɹ�...\n", _sock);
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>����,���ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			userJoin.sock = (int)_cSock;
			SendDataToAll(&userJoin);
			// ���¿ͻ���socket���뵽ȫ��������
			g_clients.push_back(_cSock);
			printf("<socket=%d>�¿ͻ��˼��룺<socket=%d, IP=%s>\n", (int)_sock, (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock;
	}



	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32

			closesocket(_sock);

			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//------------
			//���Windows socket����
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			close(_sock);
#endif

		}
	}
	//����������Ϣ
	//int _nCount = 0;
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			//fd_set fdWrite;
			//fd_set fdExp;
			//��������
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;
			// ���Ѿ����ӵ�socket�ͻ��˶����뵽fdRead������
			for (int i = 0; i < g_clients.size(); i++)
			{
				FD_SET(g_clients[i], &fdRead);
				if (maxSock < g_clients[i])
					maxSock = g_clients[i];
			}

			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 0,0 };
			int ret = select(maxSock + 1, &fdRead, 0, 0, &t); //
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
			//�ж���������socket���Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (size_t i = 0; i < fdRead.fd_count; i++)
			{
				if (-1 == RecvData(fdRead.fd_array[i])) {
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}
			return true;
		}
		return false;
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	// ���տͻ��˷��͵�����
	int RecvData(SOCKET _cSock) {

		char recvBuf[4096] = {};
		int rcvBufLen;
		rcvBufLen = recv(_cSock, recvBuf, sizeof(DataHeader), 0);
		DataHeader *dh = (DataHeader*)recvBuf;
		if (rcvBufLen <= 0)
		{
			printf("�ͻ���<socket=%d>�Ѿ��Ͽ����ӣ���������\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, recvBuf + sizeof(DataHeader), dh->dataLen - sizeof(DataHeader), 0);
		OnNetMsg(dh, _cSock);
		return 0;
	}
	// ��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader *dh, SOCKET _cSock) {
		switch (dh->cmd)
		{
		case CMD_LOGIN:
		{
			Login *lir = (Login*)dh;
			printf("�յ�<socket=%d>��Ϣ CMD_LOGIN�� ��Ϣ����=%d, username=%s, passwd=%s\n", (int)_cSock,
				lir->dataLen, lir->username, lir->password);
			LoginResult lgir(0);
			SendData(_cSock, &lgir);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout *lor = (Logout *)dh;
			printf("�յ�<socket=%d>��Ϣ CMD_LOGOUT�� ��Ϣ����=%d, username=%s\n", (int)_cSock,
				lor->dataLen, lor->username);
			LogoutResult lgor(0);
			SendData(_cSock, &lgor);

		}
		break;

		default:
		{
			Error e;
			SendData(_cSock, &e);

		}
		break;
		}

	}
	//����ָ��Socket����
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLen, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(DataHeader* header)
	{

		for (int i = 0; i < g_clients.size(); i++)
			SendData(g_clients[i], header);

	}

};

#endif // !_EasyTcpServer_hpp_