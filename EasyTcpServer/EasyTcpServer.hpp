#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      4096
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
#include "CELLTimeStamp.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 102400
#endif // !RECV_BUFF_SZIE
#define SERVER_THREAD_COUNT 4


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
	// 第二缓冲区
	char _rcvMsg[RECV_BUFF_SIZE * 10];
	// 消息缓冲区数据尾部位置
	unsigned int _lastPos;

};

class INetEvent {
public:
	// 纯虚函数
	virtual void OnLeave(ClientSocket * pClient) = 0;
};

class CellServer
{
private:
	SOCKET _sock;
	// 正式客户队列
	std::vector<ClientSocket*> _clients;
	// 缓冲客户队列
	std::vector<ClientSocket*> _clientsBuff;
	// 锁
	std::mutex _mutex;
	// 收发线程指针
	std::thread * _pThread;
	// 注册客户端离开事件
	INetEvent * _pNetEvent;
public:
	std::atomic_int _recvCount;
public:
	CellServer(SOCKET sock = INVALID_SOCKET):_sock(sock),_pThread(nullptr), 
											 _recvCount(0), _pNetEvent(nullptr){}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void setEventObj(INetEvent * pEvent)
	{
		_pNetEvent = pEvent;
	}

	//处理网络消息
	bool OnRun()
	{
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for ( auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();

			}

			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			fd_set fdWrite;
			fd_set fdExp;
			//清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//将描述符（socket）加入集合
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _clients[0]->socketfd();
			// 将已经链接的socket客户端都加入到fdRead集合中
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(_clients[i]->socketfd(), &fdRead);
				if (maxSock < _clients[i]->socketfd())
					maxSock = _clients[i]->socketfd();
			}

			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t); //
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}
		
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->socketfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[i]))
					{
						auto iter = _clients.begin() + i;
						if (iter != _clients.end())
						{
							if (_pNetEvent)
								_pNetEvent->OnLeave(_clients[i]);
							delete _clients[i];
							_clients.erase(iter);
						}

					}
				}
			}
		
		}
	}

	char _rcvBuff[RECV_BUFF_SIZE] = {};
	// 接收客户端发送的请求
	int RecvData(ClientSocket *pCSock) {


		int rcvBuffLen = recv(pCSock->socketfd(), _rcvBuff, RECV_BUFF_SIZE, 0);
		if (rcvBuffLen <= 0)
		{
			printf("客户端<socket=%d>已经断开连接，结束任务。\n", (int)pCSock->socketfd());
			return -1;
		}
		//printf("rcvBuffLen = %d\n", rcvBuffLen);
		// 将接收的数据拷贝到消息缓冲区
		memcpy(pCSock->rcvMsg() + pCSock->getLastPos(), _rcvBuff, rcvBuffLen);
		// 消息缓冲区的数据长度大于消息头DataHeader的长度
		pCSock->setLastPos(pCSock->getLastPos() + rcvBuffLen);
		// 判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pCSock->getLastPos() >= sizeof(DataHeader))
		{
			// 可以知道当前数据长度
			DataHeader *dh = (DataHeader*)pCSock->rcvMsg();
			if (pCSock->getLastPos() >= dh->dataLen)
			{
				// 消息缓冲区剩余未处理数据的长度
				unsigned int reDataLen = pCSock->getLastPos() - dh->dataLen;
				// 处理网络消息
				OnNetMsg(dh, pCSock->socketfd());
				// 将消息缓冲区中剩余未处理数据前移
				memcpy(pCSock->rcvMsg(), pCSock->rcvMsg() + dh->dataLen, reDataLen);
				// 将消息缓冲区数据尾部位置前移
				pCSock->setLastPos(reDataLen);

			}
			else
			{
				// 消息缓冲区剩余数据不够一条完整的消息
				break;
			}

		}
	
		return 0;
	}

	// 响应网络消息
	virtual void OnNetMsg(DataHeader *dh, SOCKET cSock) {
	
		_recvCount++;
		switch (dh->cmd)
		{
		case CMD_LOGIN:
		{
			Login *lir = (Login*)dh;
			/*printf("收到<socket=%d>消息 CMD_LOGIN： 信息长度=%d, username=%s, passwd=%s\n", (int)cSock,
				lir->dataLen, lir->username, lir->password);*/
				/*LoginResult lgir(0);
				SendData(cSock, &lgir);*/
		}
		break;
		case CMD_LOGOUT:
		{
			Logout *lor = (Logout *)dh;
			//printf("收到<socket=%d>消息 CMD_LOGOUT： 信息长度=%d, username=%s\n", (int)_cSock,
			//	lor->dataLen, lor->username);
			/*LogoutResult lgor(0);
			SendData(cSock, &lgor);
*/
		}
		break;

		default:
		{
			printf("收到未知消息");
			//Error e;
			//SendData(cSock, &e);

		}
		break;
		}

	}
	// 向缓冲队列添加新客户端
	void addClient(ClientSocket * pClient) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start() {
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	// 返回该线程中客户端数量，数量=缓冲队列+正式队列
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}


	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//关闭Socket
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
			// 8 关闭套节字closesocket
			closesocket(_sock);
			//------------
			//清除Windows socket环境
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->socketfd());
				delete _clients[n];
			}
			// 8 关闭套节字closesocket
			close(_sock);
#endif

		}
	}
};




class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	std::vector<CellServer*> _cellServers;
	// 计时器
	CELLTimeStamp _time;

public:
	EasyTcpServer():_sock(INVALID_SOCKET)
	{}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立socket失败...\n");
		}
		else {
			printf("建立<socket=%d>成功...\n", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 bind 绑定用于接受客户端连接的网络端口
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
			printf("错误,绑定网络端口<%d>失败...\n", port);
		}
		else {
			printf("绑定网络端口<%d>成功...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误,监听网络端口失败...\n", (int)_sock);
		}
		else {
			printf("<socket=%d>监听网络端口成功...\n", (int)_sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept()
	{
		// 4 accept 等待接受客户端连接
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
			printf("<socket=%d>错误,接受到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			/*NewUserJoin userJoin;
			userJoin.sock = (int)cSock;
			SendDataToAll(&userJoin);*/
			// 将新客户端socket加入到全局数组中
			addClientToCellServer(new ClientSocket(cSock));
			//_clients.push_back(new ClientSocket(cSock));
			//printf("<socket=%d>新客户端<%d>加入：<socket=%d, IP=%s>\n", (int)_sock, _clients.size(),
			//	(int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	void addClientToCellServer(ClientSocket * pClient) 
	{
		_clients.push_back(pClient);
		auto pMinServer = _cellServers[0] ;
		for (auto pServer : _cellServers)
		{
			if (pServer->getClientCount() < pMinServer->getClientCount())
			{
				pMinServer = pServer;
			}
		}
		pMinServer->addClient(pClient);
	}

	void Start() {
		for (int i = 0; i < SERVER_THREAD_COUNT; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
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
			fd_set fdWrite;
			fd_set fdExp;
			//清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//将描述符（socket）加入集合
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			

			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t); //
			//printf("select ret=%d count=%d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;

			}
			return true;
		}
		return false;
	}


	//关闭Socket
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
			// 8 关闭套节字closesocket
			closesocket(_sock);
			//------------
			//清除Windows socket环境
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->socketfd());
				delete _clients[n];
			}
			// 8 关闭套节字closesocket
			close(_sock);
#endif

		}
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}


	// 响应网络消息
	void time4msg() {
		
		auto t1 = _time.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			for (auto ser : _cellServers) 
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
			printf("thread<%d>, time<%lf>, socket<%d>, clients<%d>, recvCount<%d>\n", _cellServers.size(), t1, (int)_sock, _clients.size(), recvCount);
			_time.update();
		}

	}
	//发送指定Socket数据
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

		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->socketfd(), header);
		}

	}

	void OnLeave(ClientSocket * pClient)
	{
		for (int i = (int)_clients.size()-1; i >= 0; i--)
		{
			if (_clients[i] == pClient)
			{
				auto iter = _clients.begin() + i;
				if (iter != _clients.end())
				{
					_clients.erase(iter);
				}
			}
			
		}
	}
};

#endif // !_EasyTcpServer_hpp_
