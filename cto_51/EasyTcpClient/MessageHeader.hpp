#ifndef _MessageHeader_HPP
#define _MessageHeader_HPP
struct DataPackage
{
	int age;
	char name[32];

};
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN
};
//msg header
struct DataHeader
{
	short dataLength;
	short cmd;

};
struct Login :public DataHeader
{//也可以包含一个DataHeader对象
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];

};
struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;

};
//登出操作
struct LogOut :public DataHeader
{
	LogOut()
	{
		dataLength = sizeof(LogOut);
		cmd = CMD_LOGOUT;
	}
	char userName[32];

};
//登出结果
struct LogOutResult :public DataHeader
{
	LogOutResult()
	{
		dataLength = sizeof(LogOutResult);
		cmd = CMD_LOGOUT_RESULT;
	}
	int result;

};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(LogOutResult);
		cmd = CMD_NEW_USER_JOIN;
		SocketId = 0;
	}
	int SocketId;

};

#endif // !_MessageHeader_HPP
