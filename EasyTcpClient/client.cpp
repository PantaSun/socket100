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
		// 3 ������������
		char cmdBuf[128];
		cout << endl << "=============================== " << endl;
		cout << "������������롮q���˳�����" << endl;
		cin >> cmdBuf;
		// 4 ��������
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
		
		}
		else
		{
			cout << "δ֪��������ԣ�" << endl;
			continue;
		}
	}
	
}
// �Է����socket���д���

int main() {
	
	// 1 ����һ��socket
	EasyTcpClient client;
	client.InitSocket();
	//2 ���ӷ�����
	client.Connect("127.0.0.1", 4567);
	// �����߳�
	thread t1(cmdThread, &client);

	// �����߳������̷߳���
	t1.detach();
	   
	while (client.isRun())
	{
		client.InProcess();
		/* ���ٽ�������������
		// 3 ������������
		char cmdBuf[128];
		cout << endl << "=============================== " << endl;
		cout << "������������롮q���˳�����" << endl;
		cin >> cmdBuf;
		// 4 ��������
		if (strcmp(cmdBuf, "q") == 0)
		{
			break;
		}
		else  if(strcmp(cmdBuf, "login") == 0)
		{
			Login login;
			strcpy(login.username, "Saber");
			strcpy(login.password, "wuwangsaigao!");
			// 5 ���������������
			if (send(sockClnt, (const char*)&login, sizeof(Login), 0) < 0)
			{
				cout << "����ʧ�ܣ�" << endl;

			}
			else
			{
				cout << "���ͳɹ���" << endl;

			}

			// 6���ܷ�����������Ϣ
			LoginResult lgir;
			recv(sockClnt, (char*)&lgir, sizeof(LoginResult), 0);
			cout << "��¼�����0��ʾ�ɹ���-1��ʾʧ�ܣ���" << lgir.result << endl;
		}
		else if (strcmp(cmdBuf, "logout") == 0)
		{
			Logout logout;
			strcpy(logout.username, "Saber");
			// 5 ���������������
			if (send(sockClnt, (const char*)&logout, sizeof(Logout), 0) < 0)
			{
				cout << "����ʧ�ܣ�" << endl;

			}
			else
			{
				cout << "���ͳɹ���" << endl;

			}
			LoginResult lgor;
			recv(sockClnt, (char*)&lgor, sizeof(LogoutResult), 0);
			cout << "�˳������0��ʾ�ɹ���-1��ʾʧ�ܣ���" << lgor.result << endl;
		}
		else
		{
			cout << "δ֪��������ԣ�" << endl;
			continue;
		}*/
	
		

		//cout << "�ͻ��˿���ʱ�䴦�������������" << endl;
		
		Sleep(1000);

	}
	
	 
	// 7 �ر��׽���
	client.Close();
	// ���windows�׽��ֻ���
	
	system("pause");
	return 0;
}