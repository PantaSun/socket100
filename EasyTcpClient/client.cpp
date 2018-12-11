#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum CMD{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};
struct DataHeader
{
	short dataLen;
	short cmd;
	DataHeader(){}
	DataHeader(unsigned int dl, int c):dataLen(dl), cmd(c){}
};

struct Login: public DataHeader
{
	Login():DataHeader(sizeof(Login), CMD_LOGIN){}
	char username[32];
	char password[32];
};

struct Logout : public DataHeader
{
	Logout() :DataHeader(sizeof(Logout), CMD_LOGOUT) {}
	char username[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult() {}
	LogoutResult(int r) :DataHeader(sizeof(LogoutResult), 
						CMD_LOGOUT), result(r) {}
	int result;
};

struct LoginResult : public DataHeader
{
	LoginResult(){}
	LoginResult(int r) :DataHeader(sizeof(LoginResult),
						CMD_LOGOUT), result(r) {}
	int result;
};

struct Error: public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};



int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
	
	// 1 ����һ��socket
	SOCKET sockClnt = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClnt == INVALID_SOCKET)
	{
		cout << "SOCKET ����ʧ�ܣ�" << endl;
	}
	else
	{
		cout << "SOCKET �����ɹ���" << endl;
	}
	//2 ���ӷ�����
	sockaddr_in saSrv;
	saSrv.sin_family = AF_INET;
	saSrv.sin_port = htons(4567);
	saSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sockClnt, (sockaddr*)&saSrv, sizeof(saSrv)) == SOCKET_ERROR)
	{
		cout << "���ӷ�����ʧ�ܣ�" << endl;
	}
	else
	{
		cout << "���ӷ������ɹ���" << endl;

	}

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
		}
	

		

	}
	
	 
	// 7 �ر��׽���
	closesocket(sockClnt);

	// ���windows�׽��ֻ���
	WSACleanup();

	system("pause");
	return 0;
}