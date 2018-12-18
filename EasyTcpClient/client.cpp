#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
//#pragma comment(lib, "ws2_32.lib")
#include "EasyTcpClient.hpp"
using namespace std;
bool g_bRun = true;
void cmdThread() {
	while (true)
	{
		// 3 输入请求命令
		char cmdBuf[128];
		/*cout << endl << "=============================== " << endl;
		cout << "请输入命令（输入‘q’退出）：" << endl;*/
		cin >> cmdBuf;
		// 4 处理请求
		if (strcmp(cmdBuf, "q") == 0)
		{
			g_bRun = false;
			cout << "cmdThread 线程已经退出！" << endl;
			break;
		}
		/*else  if (strcmp(cmdBuf, "login") == 0)
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
		
		}*/
		else
		{
			cout << "未知命令，请重试！" << endl;
			continue;
		}
	}
	
}
// 对服务端socket进行处理

int main() {
	const int maxConnects = 5;
	EasyTcpClient * clients[maxConnects];
	for (size_t i = 0; i < maxConnects; i++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		clients[i] = new EasyTcpClient();
		
	}
	for (size_t i = 0; i < maxConnects ; i++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		clients[i]->Connect("127.0.0.1", 4567);

	}
	// 1 建立一个socket
//	EasyTcpClient client;
	//client.InitSocket();
	//2 连接服务器
	//client.Connect("127.0.0.1", 4567); //172.27.35.1

	thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.username, "saber");
	strcpy(login.password, "wuwangsaigao!");
	   
	while (g_bRun)
	{
		//client.InProcess();
		////printf("发送登录请求\n");
		//client.SendData(&login);

		for (size_t i = 0; i < maxConnects; i++)
		{
			//clients[i]->InProcess();
			clients[i]->SendData(&login);
		}

	}
	
	 
	// 7 关闭套接字
	//client.Close();
	for (size_t i = 0; i < maxConnects; i++)
	{
		clients[i]->Close();
	}
	// 清除windows套接字环境
	cout << "已经退出" << endl;
	system("pause");
	return 0;
}