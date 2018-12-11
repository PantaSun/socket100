#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
//#pragma comment(lib, "ws2_32.lib")
using namespace std;
enum CMD {
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
	DataHeader(unsigned int dl, int c) :dataLen(dl), cmd(c) {}
};

struct Login : public DataHeader
{
	Login() :DataHeader(sizeof(Login), CMD_LOGIN) {}
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
	LoginResult() {}
	LoginResult(int r) :DataHeader(sizeof(LoginResult),
		CMD_LOGOUT), result(r) {}
	int result;
};

struct Error : public DataHeader
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
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockSrv == INVALID_SOCKET )
	{
		cout << "SOCKET ����ʧ�ܣ�" << endl;
	}
	else
	{
		cout << "SOCKET �����ɹ���" << endl;
	}

	// 2 �����ڽ��տͻ������ӵ�����˿�
	sockaddr_in saIn;
	saIn.sin_family = AF_INET;
	saIn.sin_port = htons(4567);
	saIn.sin_addr.S_un.S_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	if (bind(sockSrv, (sockaddr*)&saIn, sizeof(saIn)) == SOCKET_ERROR)
	{
		cout << "������˿ڴ���" << endl;
	}
	else
	{
		cout << "������˿ڳɹ���" << endl;
	}
	// 3 ��������˿�
	if (listen(sockSrv, 5) == SOCKET_ERROR)
	{
		cout << "��������˿ڴ���" << endl;
	}
	else
	{
		cout << "������˿ڳɹ���" << endl;
	}
	// 4 �ȴ����ܿͻ�������
	sockaddr_in saClnt;
	int saClntLen = sizeof(saClnt);
	SOCKET sockClnt;
	if ((sockClnt = accept(sockSrv, (sockaddr*)&saClnt, &saClntLen)) == INVALID_SOCKET)
	{
		cout << "���յ���Ч�ͻ���SOCKET��" << endl;

	}
	else
	{
		cout << "�¿ͻ��˼��룬" << inet_ntoa(saClnt.sin_addr) << endl;

	}
	while (true)
	{
		// 5 ���տͻ��˷��͵�����
		int rcvBufLen;
		DataHeader dh;
		rcvBufLen = recv(sockClnt,(char *)&dh, sizeof(DataHeader), 0);
		if (rcvBufLen <= 0)
		{
			cout << "�ͻ����˳�����������" << endl;
			break;
		}
		// 6 ��������
		switch (dh.cmd)
		{
			case CMD_LOGIN:
			{
				Login login;
				recv(sockClnt, (char *)&login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
				// ���Զ��˺��������֤��ֻ�Ǽ򵥴�ӡ����
				cout << "�յ����� CMD_LOGIN�� ��Ϣ����="<< login.dataLen << " �û���="
					 << login.username << " ����=" << login.password << endl;
				cout << "���͵�¼��ִ��Ϣ��";
				LoginResult lgir(0);
				if (send(sockClnt, (const char*)&lgir, sizeof(LoginResult), 0) < 0)
				{
					cout << "����ʧ�ܣ�" << endl;
				}
				else
				{
					cout << "���ͳɹ���" << endl;
				}

			}
			break;
			case CMD_LOGOUT:
			{
				Logout logout;
				recv(sockClnt, (char *)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
				// ���Զ��˺��������֤��ֻ�Ǽ򵥴�ӡ����
				cout << "�յ����� CMD_LOGOUT�� ��Ϣ����="<< logout.dataLen << " �û���=" 
					 << logout.username  << endl;
				cout << "�����˳���ִ��Ϣ��";
				LogoutResult lgor(0);
				if (send(sockClnt, (const char*)&lgor, sizeof(LogoutResult), 0) < 0)
				{
					cout << "����ʧ�ܣ�" << endl;
				}
				else
				{
					cout << "���ͳɹ���" << endl;
				}

			}
			break;
		default:
			cout << "�յ�δ֪�����ͻ��˷��ʹ����ʶ��" << endl;
			Error e;
			if (send(sockClnt, (const char*)&e, sizeof(Error), 0) < 0)
			{
				cout << "����ʧ�ܣ�" << endl;
			}
			else
			{
				cout << "���ͳɹ���" << endl;
			}

			break;
		}
		
		
	}
	

	

	// 6 �ر��׽���
	closesocket(sockSrv);
	// ���windows socket ����
	WSACleanup();

	system("pause");
	return 0;
}