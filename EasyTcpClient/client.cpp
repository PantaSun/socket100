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
		// 3 ������������
		char cmdBuf[128];
		/*cout << endl << "=============================== " << endl;
		cout << "������������롮q���˳�����" << endl;*/
		cin >> cmdBuf;
		// 4 ��������
		if (strcmp(cmdBuf, "q") == 0)
		{
			g_bRun = false;
			cout << "cmdThread �߳��Ѿ��˳���" << endl;
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
			cout << "δ֪��������ԣ�" << endl;
			continue;
		}
	}
	
}
// �Է����socket���д���

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
	// 1 ����һ��socket
//	EasyTcpClient client;
	//client.InitSocket();
	//2 ���ӷ�����
	//client.Connect("127.0.0.1", 4567); //172.27.35.1

	thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.username, "saber");
	strcpy(login.password, "wuwangsaigao!");
	   
	while (g_bRun)
	{
		//client.InProcess();
		////printf("���͵�¼����\n");
		//client.SendData(&login);

		for (size_t i = 0; i < maxConnects; i++)
		{
			//clients[i]->InProcess();
			clients[i]->SendData(&login);
		}

	}
	
	 
	// 7 �ر��׽���
	//client.Close();
	for (size_t i = 0; i < maxConnects; i++)
	{
		clients[i]->Close();
	}
	// ���windows�׽��ֻ���
	cout << "�Ѿ��˳�" << endl;
	system("pause");
	return 0;
}