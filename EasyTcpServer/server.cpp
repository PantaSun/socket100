#include "EasyTcpServer.hpp"
//#pragma comment(lib, "ws2_32.lib")
using namespace std;




// �Կͻ���socket���д���

int main() {
	

	// 1 ����һ��socket
	EasyTcpServer server;
	server.InitSocket();
	// 2 �����ڽ��տͻ������ӵ�����˿�
	server.Bind("127.0.0.1", 4567);
	// 3 ��������˿�
	server.Listen(5);
	while (true)
	{
		server.OnRun();
		
		
		// select ����ݼ����е�socketʵ�������������Щ���ϣ�
		// ���ǽ��������޲�����socket���
		// ���һ������ΪNULL��ʾ����ģʽ��select��һֱ��ѯfdRead�е�socket
		// ֱ��������һ��socket�в���ʱ�ŷ���
		// int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, NULL);
		
		
	}
	
	// 8 �ر��׽���
	server.Close();
	// ���windows socket ����
	
	system("pause");
	return 0;
}