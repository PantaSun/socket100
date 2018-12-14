#include "EasyTcpServer.hpp"
//#pragma comment(lib, "ws2_32.lib")
using namespace std;




// 对客户端socket进行处理

int main() {
	

	// 1 建立一个socket
	EasyTcpServer server;
	server.InitSocket();
	// 2 绑定用于接收客户端连接的网络端口
	server.Bind("127.0.0.1", 4567);
	// 3 监听网络端口
	server.Listen(5);
	while (true)
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