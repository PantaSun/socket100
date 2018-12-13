#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <vector>
//#pragma comment(lib, "ws2_32.lib")
using namespace std;
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLen;
	short cmd;
	DataHeader(){}
	DataHeader(unsigned int dl, int c) :dataLen(dl), cmd(c) {}
};

struct Login : public DataHeader
{
	Login() :DataHeader(sizeof(Login), CMD_LOGIN) {}
	char username[32];
	char password[32];
};

struct Logout : public DataHeader
{
	Logout() :DataHeader(sizeof(Logout), CMD_LOGOUT) {}
	char username[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult() {}
	LogoutResult(int r) :DataHeader(sizeof(LogoutResult),
		CMD_LOGOUT), result(r) {}
	int result;
};

struct LoginResult : public DataHeader
{
	LoginResult() {}
	LoginResult(int r) :DataHeader(sizeof(LoginResult),
		CMD_LOGOUT), result(r) {}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin():DataHeader(sizeof(NewUserJoin), 
		CMD_NEW_USER_JOIN), sock(0){}
	int sock;
};

struct Error : public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};


// 创建全局socket数组，用来存放链接成功的客户端socket
std::vector<SOCKET> g_clients;
// 对客户端socket进行处理
int processer(SOCKET sockClnt) {

	// 5 接收客户端发送的请求
	int rcvBufLen;
	DataHeader dh;
	rcvBufLen = recv(sockClnt, (char *)&dh, sizeof(DataHeader), 0);
	if (rcvBufLen <= 0)
	{
		cout << "客户端socket=<"<< sockClnt << ">退出，结束任务。" << endl;
		return -1;
	}

	// 6 处理请求
	switch (dh.cmd)
	{
	case CMD_LOGIN:
	{
		Login login;
		recv(sockClnt, (char *)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
		// 忽略对账号密码的验证，只是简单打印出来
		cout << "收到命令 CMD_LOGIN： socket = <" << sockClnt << "> 信息长度=" << login.dataLen << " 用户名="
			<< login.username << " 密码=" << login.password << endl;
		cout << "发送登录回执信息，";
		LoginResult lgir(0);
		if (send(sockClnt, (const char*)&lgir, sizeof(LoginResult), 0) < 0)
		{
			cout << "发送失败！" << endl;
			return -1;
		}
		else
		{
			cout << "发送成功！" << endl;
		}

	}
	break;
	case CMD_LOGOUT:
	{
		Logout logout;
		recv(sockClnt, (char *)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
		// 忽略对账号密码的验证，只是简单打印出来
		cout << "收到命令 CMD_LOGOUT： socket = <"<< sockClnt <<"> 信息长度=" << logout.dataLen << " 用户名="
			<< logout.username << endl;
		cout << "发送退出回执信息，";
		LogoutResult lgor(0);
		if (send(sockClnt, (const char*)&lgor, sizeof(LogoutResult), 0) < 0)
		{
			cout << "发送失败！" << endl;
			return -1;
		}
		else
		{
			cout << "发送成功！" << endl;
		}

	}
	break;
	default:
		cout << "收到未知命令，向客户端发送错误标识。" << endl;
		Error e;
		if (send(sockClnt, (const char*)&e, sizeof(Error), 0) < 0)
		{
			cout << "发送失败！" << endl;
			return -1;
		}
		else
		{
			cout << "发送成功！" << endl;
		}

		break;
	}


	return 0;
}
int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	// 1 建立一个socket
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockSrv == INVALID_SOCKET )
	{
		cout << "SOCKET 建立失败！" << endl;
	}
	else
	{
		cout << "SOCKET 建立成功！" << endl;
	}

	// 2 绑定用于接收客户端连接的网络端口
	sockaddr_in saIn;
	saIn.sin_family = AF_INET;
	saIn.sin_port = htons(4567);
	saIn.sin_addr.S_un.S_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	if (bind(sockSrv, (sockaddr*)&saIn, sizeof(saIn)) == SOCKET_ERROR)
	{
		cout << "绑定网络端口错误！" << endl;
	}
	else
	{
		cout << "绑定网络端口成功！" << endl;
	}
	// 3 监听网络端口
	if (listen(sockSrv, 5) == SOCKET_ERROR)
	{
		cout << "监听网络端口错误！" << endl;
	}
	else
	{
		cout << "监听网络端口成功！" << endl;
	}
	
	while (true)
	{
		// 创建FD_SET，是socket的集合
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		// 先清空集合
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// 将socket放入集合
		FD_SET(sockSrv, &fdRead);
		FD_SET(sockSrv, &fdWrite);
		FD_SET(sockSrv, &fdExp);

		// 将已经链接的socket客户端都加入到fdRead集合中
		for (int i = 0; i < g_clients.size(); i++)
		{
			FD_SET(g_clients[i], &fdRead);

		}
		// select 会根据集合中的socket实际情况来重置这些集合，
		// 就是将集合中无操作的socket清除
		// 最后一个参数为NULL表示阻塞模式，select会一直查询fdRead中的socket
		// 直到有至少一个socket有操作时才返回
		// int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, NULL);
		
		timeval t = { 0,0 };
		// 这里将最后一个参数修改为t，这时就是非阻塞模式了，select对每个socket查询一次就返回
		int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret_slt < 0)
		{
			cout << "退出，结束任务。" << endl;
			break;

		}


		if (FD_ISSET(sockSrv, &fdRead))
		{
			FD_CLR(sockSrv, &fdRead);
			// 4 等待接受客户端连接
			sockaddr_in saClnt;
			int saClntLen = sizeof(saClnt);
			SOCKET sockClnt;
			if ((sockClnt = accept(sockSrv, (sockaddr*)&saClnt, &saClntLen)) == INVALID_SOCKET)
			{
				cout << "接收到无效客户端SOCKET！" << endl;

			}
			else
			{
				cout << "新客户端加入，" << inet_ntoa(saClnt.sin_addr) << endl;

				// 向其他客户端广播新用户的加入
				cout << "向其他客户端广播新用户的加入：" << endl;
				for (int i = 0; i < g_clients.size(); i++)
				{
					NewUserJoin nuserin;
					nuserin.sock = sockClnt;
					if (send(g_clients[i], (const char*)&nuserin, sizeof(NewUserJoin), 0) < 0)
					{
						cout << "发送失败！" << i+1 << endl;
						return -1;
					}
					else
					{
						cout << "发送成功！" << i+1 << endl;
					}


				}
				// 将新客户端socket加入到全局数组中
				g_clients.push_back(sockClnt);
			}

		}

		// 循环的处理有操作的多个客户端
		for (size_t i = 0; i < fdRead.fd_count; i++)
		{
			if (-1 == processer(fdRead.fd_array[i])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		//cout << "服务器空闲时间处理其他事物。。。" << endl;

	}
	for (int i = 0; i < g_clients.size(); i++)
	{
		closesocket(g_clients[i]);
	}
	// 8 关闭套接字
	closesocket(sockSrv);
	// 清除windows socket 环境
	WSACleanup();

	system("pause");
	return 0;
}