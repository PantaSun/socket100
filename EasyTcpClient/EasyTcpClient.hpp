#ifndef _EASY_TCP_CLIENT
#define _EASY_TCP_CLIENT

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET) (~0)
#define SOCKET_ERROR			(-1)
#endif

#include <iostream>
#include <thread>
#include "MessageHeader.hpp"
  
class EasyTcpClient
{
public:
	EasyTcpClient():_sock(INVALID_SOCKET) 
	{

	}

	virtual ~EasyTcpClient() 
	{
		Close();
	}
	/*
	1 ����һ��socket
	*/
	int InitSocket() 
	{
	#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
	#endif // _WIN32
		
		if (_sock != INVALID_SOCKET)
		{
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET)
		{
			std::cout << "SOCKET ����ʧ�ܣ�" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "SOCKET �����ɹ���" << std::endl;
			return 0;
		}
	}

	/*
	2 ���ӷ�����
	*/
	int Connect(const char *ip, unsigned short port) 
	{
		if (_sock == INVALID_SOCKET)
		{
			InitSocket();
		}
		sockaddr_in saSrv;
		saSrv.sin_family = AF_INET;
		saSrv.sin_port = htons(port);
#ifdef _WIN32
		saSrv.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		saSrv.sin_addr.s_Addr = inet_addr(ip);
#endif // _WIN32

		int  ret = connect(_sock, (sockaddr*)&saSrv, sizeof(saSrv));
		if (ret == SOCKET_ERROR)
		{
			std::cout << "<socket="<< _sock <<">���ӷ�����<"<< ip << ":" << port <<">ʧ�ܣ�" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "<socket=" << _sock << ">���ӷ�����<" << ip << ":" << port << ">�ɹ���" << std::endl;
			return ret;
		}
	}

	/*
	 �ر��׽���
	*/
	void Close() 
	{
		if (_sock != INVALID_SOCKET)
		{

			#ifdef _WIN32
				
				closesocket(_sock);
				// ���windows�׽��ֻ���
				WSACleanup();
			#else
				close(_sock);
			#endif // _WIN32

			_sock = INVALID_SOCKET;
		}

	}

	/*
	 ��ѯ������Ϣ
	*/
	bool InProcess() 
	{
		if (isRun()) 
		{

			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);

			timeval t = { 0, 0 };
			int ret_slt = select(_sock + 1, &fdRead, NULL, NULL, &t);

			if (ret_slt < 0)
			{
				std::cout << "socket=<" << _sock << "> select����������˳�" << std::endl;
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					std::cout << "processor����" << std::endl;
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// �鿴�Ƿ��ڹ�����
	bool isRun() 
	{
		return _sock != INVALID_SOCKET;
	}

	// ����������Ϣ
	int RecvData(SOCKET sockClnt) 
	{
		char recvBuf[4096] = {};
		// 5 ���տͻ��˷��͵�����
		int rcvBufLen;
		rcvBufLen = recv(sockClnt, recvBuf, sizeof(DataHeader), 0);
		DataHeader *dh = (DataHeader*)recvBuf;
		if (rcvBufLen <= 0)
		{
			std::cout << "��������Ͽ����ӣ���������" << std::endl;
			return -1;
		}
		recv(sockClnt, recvBuf + sizeof(DataHeader), dh->dataLen - sizeof(DataHeader), 0);
		MsgProcessor(dh);
		return 0;
	}

	// ����������Ϣ
	void MsgProcessor(DataHeader *dh) {
		// 6 ��������
		switch (dh->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				LoginResult *lir = (LoginResult*)dh;
				std::cout << "�յ���������Ϣ CMD_LOGIN_RESULT�� ��Ϣ����=" << lir->dataLen << std::endl;

			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult *lor = (LogoutResult *)dh;
				std::cout << "�յ���������Ϣ CMD_LOGOUT_RESULT�� ��Ϣ����=" << lor->dataLen << std::endl;

			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *nuserin = (NewUserJoin*)dh;
				std::cout << "�յ���������Ϣ NEW_USER_Join�� socket=" << nuserin->sock << std::endl;

			}
			break;
			default:
				break;
		}

	}

	// ����������Ϣ
	int SendData(DataHeader *dh) {
		if (isRun())
		{
			return send(_sock, (const char*)dh, dh->dataLen, 0);
		}
		return SOCKET_ERROR;
	}
private:
	SOCKET _sock;
};







#endif // _EASY_TCP_CLIENT

