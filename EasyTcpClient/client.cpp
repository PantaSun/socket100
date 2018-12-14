#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
//#pragma comment(lib, "ws2_32.lib")
#include "EasyTcpClient.hpp"
using namespace std;
bool g_exit = false;
void cmdThread(EasyTcpClient *client) {
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
			client->Close();
			return;
		}
		else  if (strcmp(cmdBuf, "login") == 0)
		{
			Login login;
			strcpy(login.username, "Saber");
			strcpy(login.password, "wuwangsaigao!");
			// 5 向服务器发送请求
			if (client->SendData(&login) < 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}

		}
		else if (strcmp(cmdBuf, "logout") == 0)
		{
			Logout logout;
			strcpy(logout.username, "Saber");
			// 5 向服务器发送请求
			if (client->SendData(&logout)< 0)
			{
				cout << "发送失败！" << endl;

			}
			else
			{
				cout << "发送成功！" << endl;

			}
		
		}
		else
		{
			cout << "未知命令，请重试！" << endl;
			continue;
		}
	}
	
}
// 对服务端socket进行处理

int main() {
	
	// 1 建立一个socket
	EasyTcpClient client;
	client.InitSocket();
	//2 连接服务器
	client.Connect("127.0.0.1", 4567);
	// 启动线程
	thread t1(cmdThread, &client);

	// 将子线程与主线程分离
	t1.detach();
	   
	while (client.isRun())
	{
		client.InProcess();
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
	
		

		//cout << "客户端空闲时间处理其他事物。。。" << endl;
		
		Sleep(1000);

	}
	
	 
	// 7 关闭套接字
	client.Close();
	// 清除windows套接字环境
	
	system("pause");
	return 0;
}