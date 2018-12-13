#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <vector>
//#pragma comment(lib, "ws2_32.lib")
using namespace std;
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
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

struct NewUserJoin : public DataHeader
{
	NewUserJoin():DataHeader(sizeof(NewUserJoin), 
		CMD_NEW_USER_JOIN), sock(0){}
	int sock;
};

struct Error : public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};


// ����ȫ��socket���飬����������ӳɹ��Ŀͻ���socket
std::vector<SOCKET> g_clients;
// �Կͻ���socket���д���
int processer(SOCKET sockClnt) {

	// 5 ���տͻ��˷��͵�����
	int rcvBufLen;
	DataHeader dh;
	rcvBufLen = recv(sockClnt, (char *)&dh, sizeof(DataHeader), 0);
	if (rcvBufLen <= 0)
	{
		cout << "�ͻ���socket=<"<< sockClnt << ">�˳�����������" << endl;
		return -1;
	}

	// 6 ��������
	switch (dh.cmd)
	{
	case CMD_LOGIN:
	{
		Login login;
		recv(sockClnt, (char *)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
		// ���Զ��˺��������֤��ֻ�Ǽ򵥴�ӡ����
		cout << "�յ����� CMD_LOGIN�� socket = <" << sockClnt << "> ��Ϣ����=" << login.dataLen << " �û���="
			<< login.username << " ����=" << login.password << endl;
		cout << "���͵�¼��ִ��Ϣ��";
		LoginResult lgir(0);
		if (send(sockClnt, (const char*)&lgir, sizeof(LoginResult), 0) < 0)
		{
			cout << "����ʧ�ܣ�" << endl;
			return -1;
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
		cout << "�յ����� CMD_LOGOUT�� socket = <"<< sockClnt <<"> ��Ϣ����=" << logout.dataLen << " �û���="
			<< logout.username << endl;
		cout << "�����˳���ִ��Ϣ��";
		LogoutResult lgor(0);
		if (send(sockClnt, (const char*)&lgor, sizeof(LogoutResult), 0) < 0)
		{
			cout << "����ʧ�ܣ�" << endl;
			return -1;
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
			return -1;
		}
		else
		{
			cout << "���ͳɹ���" << endl;
		}

		break;
	}


	return 0;
}
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
		cout << "��������˿ڳɹ���" << endl;
	}
	
	while (true)
	{
		// ����FD_SET����socket�ļ���
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		// ����ռ���
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// ��socket���뼯��
		FD_SET(sockSrv, &fdRead);
		FD_SET(sockSrv, &fdWrite);
		FD_SET(sockSrv, &fdExp);

		// ���Ѿ����ӵ�socket�ͻ��˶����뵽fdRead������
		for (int i = 0; i < g_clients.size(); i++)
		{
			FD_SET(g_clients[i], &fdRead);

		}
		// select ����ݼ����е�socketʵ�������������Щ���ϣ�
		// ���ǽ��������޲�����socket���
		// ���һ������ΪNULL��ʾ����ģʽ��select��һֱ��ѯfdRead�е�socket
		// ֱ��������һ��socket�в���ʱ�ŷ���
		// int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, NULL);
		
		timeval t = { 0,0 };
		// ���ｫ���һ�������޸�Ϊt����ʱ���Ƿ�����ģʽ�ˣ�select��ÿ��socket��ѯһ�ξͷ���
		int ret_slt = select(sockSrv + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret_slt < 0)
		{
			cout << "�˳�����������" << endl;
			break;

		}


		if (FD_ISSET(sockSrv, &fdRead))
		{
			FD_CLR(sockSrv, &fdRead);
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

				// �������ͻ��˹㲥���û��ļ���
				cout << "�������ͻ��˹㲥���û��ļ��룺" << endl;
				for (int i = 0; i < g_clients.size(); i++)
				{
					NewUserJoin nuserin;
					nuserin.sock = sockClnt;
					if (send(g_clients[i], (const char*)&nuserin, sizeof(NewUserJoin), 0) < 0)
					{
						cout << "����ʧ�ܣ�" << i+1 << endl;
						return -1;
					}
					else
					{
						cout << "���ͳɹ���" << i+1 << endl;
					}


				}
				// ���¿ͻ���socket���뵽ȫ��������
				g_clients.push_back(sockClnt);
			}

		}

		// ѭ���Ĵ����в����Ķ���ͻ���
		for (size_t i = 0; i < fdRead.fd_count; i++)
		{
			if (-1 == processer(fdRead.fd_array[i])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		//cout << "����������ʱ�䴦�������������" << endl;

	}
	for (int i = 0; i < g_clients.size(); i++)
	{
		closesocket(g_clients[i]);
	}
	// 8 �ر��׽���
	closesocket(sockSrv);
	// ���windows socket ����
	WSACleanup();

	system("pause");
	return 0;
}