#include "EasyTcpServer.hpp"
//#pragma comment(lib, "ws2_32.lib")
using namespace std;



bool g_bRun = true;
void cmdThread() {
	while (true)
	{
		// 3 ������������
		char cmdBuf[128];
		/*printf("=============================== \n");
		printf("������������롮q���˳�����\n");*/
		scanf("%s",cmdBuf);
		// 4 ��������
		if (strcmp(cmdBuf, "q") == 0)
		{
			g_bRun = false;
			printf("cmdThread �߳��Ѿ��˳���\n");
			break;
		}
		/*else  if (strcmp(cmdBuf, "login") == 0)
		{
			Login login;
			strcpy(login.username, "Saber");
			strcpy(login.password, "wuwangsaigao!");
			// 5 ���������������
			if (client->SendData(&login) < 0)
			{
				cout << "����ʧ�ܣ�" << endl;

			}
			else
			{
				cout << "���ͳɹ���" << endl;

			}

		}
		else if (strcmp(cmdBuf, "logout") == 0)
		{
			Logout logout;
			strcpy(logout.username, "Saber");
			// 5 ���������������
			if (client->SendData(&logout)< 0)
			{
				cout << "����ʧ�ܣ�" << endl;

			}
			else
			{
				cout << "���ͳɹ���" << endl;

			}

		}*/
		else
		{
			printf("δ֪��������ԣ�\n");
			continue;
		}
	}

}

// �Կͻ���socket���д���

int main() {
	

	// 1 ����һ��socket
	EasyTcpServer server;
	server.InitSocket();
	// 2 �����ڽ��տͻ������ӵ�����˿�
	server.Bind(INADDR_ANY, 4567);
	// 3 ��������˿�
	server.Listen(5);
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
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