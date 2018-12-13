#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
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
	DataHeader() {}
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
	NewUserJoin() :DataHeader(sizeof(NewUserJoin),
		CMD_NEW_USER_JOIN), sock(0) {}
	int sock;
};

struct Error : public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};
bool g_exit = false;
void cmdThread(SOCKET sockClnt) {
	while (true)
	{
		// 3 输入请求命令
		char cmdBuf[128];
		cout << endl << "=============================== " << endl;
		cout << "请输入命令（输入‘q’退出）：" << endl;
		cin >> cmdBuf;
		// 4 处理请求
		if (strcmp(cmdBuf, "q") == 0)
		{
			g_exit = true;
			return;
		}
		else  if (strcmp(cmdBuf, "login") == 0)
		{
			Login login;
			strcpy(login.username, "Saber");
			strcpy(login.password, "wuwangsaigao!");
			// 5 向服务器发送请求
			if (send(sockClnt, (const char*)&login, sizeof(Login), 0) < 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}

			// 6接受服务器返回信息
			LoginResult lgir;
			recv(sockClnt, (char*)&lgir, sizeof(LoginResult), 0);
			cout << "登录结果（0表示成功，-1表示失败）：" << lgir.result << endl;
		}
		else if (strcmp(cmdBuf, "logout") == 0)
		{
			Logout logout;
			strcpy(logout.username, "Saber");
			// 5 向服务器发送请求
			if (send(sockClnt, (const char*)&logout, sizeof(Logout), 0) < 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}
			LoginResult lgor;
			recv(sockClnt, (char*)&lgor, sizeof(LogoutResult), 0);
			cout << "退出结果（0表示成功，-1表示失败）：" << lgor.result << endl;
		}
		else
		{
			cout << "未知命令，请重试！" << endl;
			continue;
		}
	}
	
}
// 对服务端socket进行处理
int processer(SOCKET sockClnt) {

	// 5 接收客户端发送的请求
	int rcvBufLen;
	DataHeader dh;
	rcvBufLen = recv(sockClnt, (char *)&dh, sizeof(DataHeader), 0);
	if (rcvBufLen <= 0)
	{
		cout << "与服务器断开连接，结束任务。" << endl;
		return -1;
	}

	// 6 处理请求
	switch (dh.cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		LoginResult lir;
		recv(sockClnt, (char *)&lir + sizeof(DataHeader), sizeof(LoginResult) - sizeof(DataHeader), 0);
		cout << "收到服务器消息 CMD_LOGIN_RESULT： 信息长度=" << lir.dataLen << endl;

	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult lor;
		recv(sockClnt, (char *)&lor + sizeof(DataHeader), sizeof(LogoutResult) - sizeof(DataHeader), 0);
		cout << "收到服务器消息 CMD_LOGOUT_RESULT： 信息长度=" << lor.dataLen << endl;

	}
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin nuserin;
		recv(sockClnt, (char *)&nuserin + sizeof(DataHeader), sizeof(NewUserJoin) - sizeof(DataHeader), 0);
		cout << "收到服务器消息 NEW_USER_Join： socket=" << nuserin.sock << endl;

	}
	break;
	default:
		break;
	}


	return 0;
}

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
	
	// 1 建立一个socket
	SOCKET sockClnt = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClnt == INVALID_SOCKET)
	{
		cout << "SOCKET 建立失败！" << endl;
	}
	else
	{
		cout << "SOCKET 建立成功！" << endl;
	}
	//2 连接服务器
	sockaddr_in saSrv;
	saSrv.sin_family = AF_INET;
	saSrv.sin_port = htons(4567);
	saSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sockClnt, (sockaddr*)&saSrv, sizeof(saSrv)) == SOCKET_ERROR)
	{
		cout << "连接服务器失败！" << endl;
	}
	else
	{
		cout << "连接服务器成功！" << endl;

	}
	// 启动线程
	thread t1(cmdThread, sockClnt);

	// 将子线程与主线程分离
	t1.detach();

	while (!g_exit)
	{
		/* 不再接受命令行输入
		// 3 输入请求命令
		char cmdBuf[128];
		cout << endl << "=============================== " << endl;
		cout << "请输入命令（输入‘q’退出）：" << endl;
		cin >> cmdBuf;
		// 4 处理请求
		if (strcmp(cmdBuf, "q") == 0)
		{
			break;
		}
		else  if(strcmp(cmdBuf, "login") == 0)
		{
			Login login;
			strcpy(login.username, "Saber");
			strcpy(login.password, "wuwangsaigao!");
			// 5 向服务器发送请求
			if (send(sockClnt, (const char*)&login, sizeof(Login), 0) < 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}

			// 6接受服务器返回信息
			LoginResult lgir;
			recv(sockClnt, (char*)&lgir, sizeof(LoginResult), 0);
			cout << "登录结果（0表示成功，-1表示失败）：" << lgir.result << endl;
		}
		else if (strcmp(cmdBuf, "logout") == 0)
		{
			Logout logout;
			strcpy(logout.username, "Saber");
			// 5 向服务器发送请求
			if (send(sockClnt, (const char*)&logout, sizeof(Logout), 0) < 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}
			LoginResult lgor;
			recv(sockClnt, (char*)&lgor, sizeof(LogoutResult), 0);
			cout << "退出结果（0表示成功，-1表示失败）：" << lgor.result << endl;
		}
		else
		{
			cout << "未知命令，请重试！" << endl;
			continue;
		}*/
	
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(sockClnt, &fdRead);

		timeval t = { 0, 0 };
		int ret_slt = select(sockClnt + 1, &fdRead, NULL, NULL, &t);

		if (ret_slt < 0)
		{
			cout << "select任务结束，退出" << endl;
			break;
		}
		
		if (FD_ISSET(sockClnt, &fdRead))
		{
			FD_CLR(sockClnt, &fdRead);
			if (-1 == processer(sockClnt))
			{
				cout << "processor结束" << endl;
				break;
			}
		}

		//cout << "客户端空闲时间处理其他事物。。。" << endl;
		
		Sleep(1000);

	}
	
	 
	// 7 关闭套接字
	closesocket(sockClnt);

	// 清除windows套接字环境
	WSACleanup();

	system("pause");
	return 0;
}