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
#include <thread>
#include"MessageHeader.hpp"
#include "CELLTimeStamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 102400
#endif // !RECV_BUFF_SZIE




class ClientSocket {
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET) :_socketfd(sock), _lastPos(0)
	{
		memset(_rcvMsg, 0, sizeof(_rcvMsg));
	}
	SOCKET socketfd() 
	{
		return _socketfd;
	}
	int getLastPos() 
	{
		return _lastPos;
	}
	void setLastPos(int pos) 
	{
		_lastPos = pos;
	}
	char * rcvMsg() 
	{
		return _rcvMsg;
	}
private:
	SOCKET _socketfd;
	// �ڶ�������
	char _rcvMsg[RECV_BUFF_SIZE * 10];
	// ��Ϣ����������β��λ��
	unsigned int _lastPos;
	
};

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	// ��ʱ��
	CELLTimeStamp _time;
	// ������
	int _packsCount;
public:
	EasyTcpServer():_sock(INVALID_SOCKET), _packsCount(0)
	{}
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
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>����,���ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			userJoin.sock = (int)cSock;
			SendDataToAll(&userJoin);
			// ���¿ͻ���socket���뵽ȫ��������
			_clients.push_back(new ClientSocket(cSock));
			printf("<socket=%d>�¿ͻ���<%d>���룺<socket=%d, IP=%s>\n", (int)_sock, _clients.size(),
				(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}



	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->socketfd());
				delete _clients[n];
			}	
			// 8 �ر��׽���closesocket
			closesocket(_sock);
			//------------
			//���Windows socket����
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->socketfd());
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
			fd_set fdWrite;
			fd_set fdExp;
			//������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;
			// ���Ѿ����ӵ�socket�ͻ��˶����뵽fdRead������
			for (int i = (int)_clients.size()-1; i >=0; i--)
			{
				FD_SET(_clients[i]->socketfd(), &fdRead);
				if (maxSock < _clients[i]->socketfd())
					maxSock = _clients[i]->socketfd();
			}

			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); //
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
			/*for (size_t i = 0; i < fdRead.fd_count; i++)
			{
				if (-1 == RecvData(fdRead.fd_array[i])) {
					auto iter = find(_clients.begin(), _clients.end(), fdRead.fd_array[i]);
					if (iter != _clients.end())
					{ 
						delete *iter;
						_clients.erase(iter);
					}
				}
			}*/
			for (int i = (int)_clients.size()-1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->socketfd(),&fdRead))
				{
					if (-1 == RecvData(_clients[i]))
					{
						auto iter = _clients.begin() + i;
						if (iter != _clients.end())
						{
							delete _clients[i];
							_clients.erase(iter);
						}

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

	char _rcvBuff[RECV_BUFF_SIZE] = {};
	// ���տͻ��˷��͵�����
	int RecvData(ClientSocket *pCSock) {

		
		int rcvBuffLen = recv(pCSock->socketfd(), _rcvBuff, RECV_BUFF_SIZE, 0);
		if (rcvBuffLen <= 0)
		{
			printf("�ͻ���<socket=%d>�Ѿ��Ͽ����ӣ���������\n", (int)pCSock->socketfd());
			return -1;
		}
		//printf("rcvBuffLen = %d\n", rcvBuffLen);
		// �����յ����ݿ�������Ϣ������
		memcpy(pCSock->rcvMsg() + pCSock->getLastPos(), _rcvBuff, rcvBuffLen);
		// ��Ϣ�����������ݳ��ȴ�����ϢͷDataHeader�ĳ���
		pCSock->setLastPos(pCSock->getLastPos() + rcvBuffLen);
		// �ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (pCSock->getLastPos() >= sizeof(DataHeader))
		{
			// ����֪����ǰ���ݳ���
			DataHeader *dh = (DataHeader*)pCSock->rcvMsg();
			if (pCSock->getLastPos() >= dh->dataLen)
			{
				// ��Ϣ������ʣ��δ�������ݵĳ���
				unsigned int reDataLen = pCSock->getLastPos() - dh->dataLen;
				// ����������Ϣ
				OnNetMsg(dh, pCSock->socketfd());
				// ����Ϣ��������ʣ��δ��������ǰ��
				memcpy(pCSock->rcvMsg(), pCSock->rcvMsg() + dh->dataLen, reDataLen);
				// ����Ϣ����������β��λ��ǰ��
				pCSock->setLastPos(reDataLen);

			}
			else
			{
				// ��Ϣ������ʣ�����ݲ���һ����������Ϣ
				break;
			}

		}
		/*DataHeader *dh = (DataHeader*)recvBuf;
		
		recv(cSock, recvBuf + sizeof(DataHeader), dh->dataLen - sizeof(DataHeader), 0);
		OnNetMsg(dh, cSock);
		*/
		return 0;
	}
	// ��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader *dh, SOCKET cSock) {
		_packsCount++;
		
		auto t1 = _time.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>, socket<%d>, packsCount<%d>\n", t1, _sock, _packsCount);
			_time.update();
			_packsCount = 0;
		}

		switch (dh->cmd)
		{
		case CMD_LOGIN:
		{
			Login *lir = (Login*)dh;
			/*printf("�յ�<socket=%d>��Ϣ CMD_LOGIN�� ��Ϣ����=%d, username=%s, passwd=%s\n", (int)cSock,
				lir->dataLen, lir->username, lir->password);*/
			/*LoginResult lgir(0);
			SendData(cSock, &lgir);*/
		}
		break;
		case CMD_LOGOUT:
		{
			Logout *lor = (Logout *)dh;
			//printf("�յ�<socket=%d>��Ϣ CMD_LOGOUT�� ��Ϣ����=%d, username=%s\n", (int)_cSock,
			//	lor->dataLen, lor->username);
			/*LogoutResult lgor(0);
			SendData(cSock, &lgor);
*/
		}
		break;

		default:
		{
			printf("�յ�δ֪��Ϣ");
			Error e;
			SendData(cSock, &e);

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

		for (int i = 0; i < _clients.size(); i++)
			SendData(_clients[i]->socketfd(), header);

	}

};

#endif // !_EasyTcpServer_hpp_
