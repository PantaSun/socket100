#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")
using namespace std;
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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

struct Error : public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};
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
		cout << "监网络端口成功！" << endl;
	}
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

	}
	while (true)
	{
		// 5 接收客户端发送的请求
		int rcvBufLen;
		DataHeader dh;
		rcvBufLen = recv(sockClnt,(char *)&dh, sizeof(DataHeader), 0);
		if (rcvBufLen <= 0)
		{
			cout << "客户端退出，结束任务。" << endl;
			break;
		}
		// 6 处理请求
		switch (dh.cmd)
		{
			case CMD_LOGIN:
			{
				Login login;
				recv(sockClnt, (char *)&login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
				// 忽略对账号密码的验证，只是简单打印出来
				cout << "收到命令 CMD_LOGIN： 信息长度="<< login.dataLen << " 用户名="
					 << login.username << " 密码=" << login.password << endl;
				cout << "发送登录回执信息，";
				LoginResult lgir(0);
				if (send(sockClnt, (const char*)&lgir, sizeof(LoginResult), 0) < 0)
				{
					cout << "发送失败！" << endl;
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
				cout << "收到命令 CMD_LOGOUT： 信息长度="<< logout.dataLen << " 用户名=" 
					 << logout.username  << endl;
				cout << "发送退出回执信息，";
				LogoutResult lgor(0);
				if (send(sockClnt, (const char*)&lgor, sizeof(LogoutResult), 0) < 0)
				{
					cout << "发送失败！" << endl;
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
			}
			else
			{
				cout << "发送成功！" << endl;
			}

			break;
		}
		
		
	}
	

	

	// 6 关闭套接字
	closesocket(sockSrv);
	// 清除windows socket 环境
	WSACleanup();

	system("pause");
	return 0;
}