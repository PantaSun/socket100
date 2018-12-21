#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
//#pragma comment(lib, "ws2_32.lib")
#include "EasyTcpClient.hpp"
using namespace std;
bool g_bRun = true;

// 客户端数
const int maxConnects = 10000;
// 线程数
const int tCounts = 4;
EasyTcpClient * clients[maxConnects];

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
		else
		{
			cout << "未知命令，请重试！" << endl;
			continue;
		}
	}
	
}

void sendThread(int id) {

	int step	= maxConnects / tCounts;
	int begin	= step * (id - 1);
	int end		= step * id;

	for (int  i = begin; i < end; i++)
	{
		clients[i] = new EasyTcpClient();
	}

	for (int i = begin; i < end; i++)
	{
		if( -1 == clients[i]->Connect("127.0.0.1", 4567))
			printf("thread<%d>, Connect=%d 失败！！！\n", id, i);
		else
			printf("thread<%d>, Connect=%d\n", id, i);
	}
	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);
	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].username, "saber");
		strcpy(login[i].password, "wuwangsaigao!");

	}
	const int nLen = sizeof(login);

	while (g_bRun)
	{	
		for (int i = begin; i < end; i++)
		{
			clients[i]->SendData(login, nLen);
		}

	}

	for (int i = begin; i < end; i++)
	{
		clients[i]->Close();
	}

}
// 对服务端socket进行处理

int main() {
	
	// UI线程
	thread t1(cmdThread);
	t1.detach();
	
	// 发送线程
	for (int i = 0; i < tCounts; i++)
	{
		thread t(sendThread, i+1);
		t.detach();
	}
	
	while (g_bRun)
	{
		Sleep(100);
	}

	// 7 关闭套接字
	//client.Close();
	
	// 清除windows套接字环境
	cout << "已经退出" << endl;
	system("pause");
	return 0;
}