#ifndef _EASY_TCP_CLIENT
#define _EASY_TCP_CLIENT

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define FD_SETSIZE      2506
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
	1 建立一个socket
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
			std::cout << "SOCKET 建立失败！" << std::endl;
			return -1;
		}
		else
		{
			//std::cout << "SOCKET 建立成功！" << std::endl;
			return 0;
		}
	}

	/*
	2 连接服务器
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
			//std::cout << "<socket="<< _sock <<">连接服务器<"<< ip << ":" << port <<">失败！" << std::endl;
			return -1;
		}
		else
		{
			//std::cout << "<socket=" << _sock << ">连接服务器<" << ip << ":" << port << ">成功！" << std::endl;
			return ret;
		}
	}

	/*
	 关闭套接字
	*/
	void Close() 
	{
		if (_sock != INVALID_SOCKET)
		{

			#ifdef _WIN32
				
				closesocket(_sock);
				// 清除windows套接字环境
				WSACleanup();
			#else
				close(_sock);
			#endif // _WIN32

			_sock = INVALID_SOCKET;
		}

	}

	/*
	 查询网络消息
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
				std::cout << "socket=<" << _sock << "> select任务结束，退出" << std::endl;
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					std::cout << "processor结束" << std::endl;
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// 查看是否在工作中
	bool isRun() 
	{
		return _sock != INVALID_SOCKET;
	}
#define RECV_BUFF_SIZE 10240
	// 接受缓冲区
	char _rcvBuff[RECV_BUFF_SIZE] = {};
	// 第二缓冲区
	char _rcvMsg[RECV_BUFF_SIZE * 10] = {};
	// 消息缓冲区数据尾部位置
	unsigned int _lastPos = 0;

	// 接受网络消息，可以处理少包、粘包问题
	int RecvData(SOCKET sockClnt) 
	{
		int rcvBuffLen = recv(sockClnt, _rcvBuff, RECV_BUFF_SIZE, 0);
		
		if (rcvBuffLen <= 0)
		{
			printf("客户端<socket=%d>已经断开连接，结束任务。\n", (int)sockClnt);
			return -1;
		}
		//printf("rcvBuffLen = %d\n", rcvBuffLen);
		
		// 将接收的数据拷贝到消息缓冲区
		memcpy(_rcvMsg + _lastPos, _rcvBuff, rcvBuffLen);
		// 消息缓冲区的数据长度大于消息头DataHeader的长度
		_lastPos += rcvBuffLen;
		// 判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (_lastPos >= sizeof(DataHeader))
		{
			// 可以知道当前数据长度
			DataHeader *dh = (DataHeader*)_rcvMsg;
			if (_lastPos >= dh->dataLen)
			{
				// 消息缓冲区剩余未处理数据的长度
				unsigned int reDataLen = _lastPos - dh->dataLen;
				// 处理网络消息
				MsgProcessor(dh);
				// 将消息缓冲区中剩余未处理数据前移
				memcpy(_rcvMsg, _rcvMsg + dh->dataLen, reDataLen);
				// 将消息缓冲区数据尾部位置前移
				_lastPos = reDataLen;

			}
			else
			{
				// 消息缓冲区剩余数据不够一条完整的消息
				break;
			}

		 }
		return 0;
	}

	// 处理网络消息
	void MsgProcessor(DataHeader *dh) {
		// 6 处理请求
		switch (dh->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				LoginResult *lir = (LoginResult*)dh;
				//std::cout << "收到服务器消息 CMD_LOGIN_RESULT： 信息长度=" << lir->dataLen << std::endl;

			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult *lor = (LogoutResult *)dh;
				//std::cout << "收到服务器消息 CMD_LOGOUT_RESULT： 信息长度=" << lor->dataLen << std::endl;

			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *nuserin = (NewUserJoin*)dh;
				//std::cout << "收到服务器消息 NEW_USER_Join： socket=" << nuserin->sock << std::endl;

			}
			break;
			default:
			{
				Error *er = (Error*)dh;
				//std::cout << "收到服务器消息 ERROR： 信息长度=" << er->dataLen << std::endl;

			}
				break;
		}

	}

	// 发送网络消息
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

