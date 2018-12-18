#include "EasyTcpServer.hpp"
//#pragma comment(lib, "ws2_32.lib")
using namespace std;



bool g_bRun = true;
void cmdThread() {
	while (true)
	{
		// 3 输入请求命令
		char cmdBuf[128];
		/*printf("=============================== \n");
		printf("请输入命令（输入‘q’退出）：\n");*/
		scanf("%s",cmdBuf);
		// 4 处理请求
		if (strcmp(cmdBuf, "q") == 0)
		{
			g_bRun = false;
			printf("cmdThread 线程已经退出！\n");
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
			printf("未知命令，请重试！\n");
			continue;
		}
	}

}

// 对客户端socket进行处理

int main() {
	

	// 1 建立一个socket
	EasyTcpServer server;
	server.InitSocket();
	// 2 绑定用于接收客户端连接的网络端口
	server.Bind(INADDR_ANY, 4567);
	// 3 监听网络端口
	server.Listen(5);
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		
		
		// select 会根据集合中的socket实际情况来重置这些集合，
		// 就是将集合中无操作的socket清除
		// 最后一个参数为NULL表示阻塞模式，select会一直查询fdRead中的socket
		// 直到有至少一个socket有操作时才返回
		// int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, NULL);
		
		
	}
	
	// 8 关闭套接字
	server.Close();
	// 清除windows socket 环境
	
	system("pause");
	return 0;
}