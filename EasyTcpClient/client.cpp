#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
//#pragma comment(lib, "ws2_32.lib")
#include "EasyTcpClient.hpp"
using namespace std;
bool g_bRun = true;

// �ͻ�����
const int maxConnects = 10000;
// �߳���
const int tCounts = 4;
EasyTcpClient * clients[maxConnects];

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
		else
		{
			cout << "δ֪��������ԣ�" << endl;
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
			printf("thread<%d>, Connect=%d ʧ�ܣ�����\n", id, i);
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
// �Է����socket���д���

int main() {
	
	// UI�߳�
	thread t1(cmdThread);
	t1.detach();
	
	// �����߳�
	for (int i = 0; i < tCounts; i++)
	{
		thread t(sendThread, i+1);
		t.detach();
	}
	
	while (g_bRun)
	{
		Sleep(100);
	}

	// 7 �ر��׽���
	//client.Close();
	
	// ���windows�׽��ֻ���
	cout << "�Ѿ��˳�" << endl;
	system("pause");
	return 0;
}