#ifndef _MESSAGE_HEADER
#define _MESSAGE_HEADER

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
	DataHeader() {}
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
		CMD_LOGOUT_RESULT), result(r) {}
	int result;
};

struct LoginResult : public DataHeader
{
	LoginResult() {}
	LoginResult(int r) :DataHeader(sizeof(LoginResult),
		CMD_LOGIN_RESULT), result(r) {}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin() :DataHeader(sizeof(NewUserJoin),
		CMD_NEW_USER_JOIN), sock(0) {}
	int sock;
};

struct Error : public DataHeader
{
	Error() :DataHeader(sizeof(Error),
		CMD_LOGOUT), result(-1) {}
	int result;
};

#endif // !_MESSAGE_HEADER
