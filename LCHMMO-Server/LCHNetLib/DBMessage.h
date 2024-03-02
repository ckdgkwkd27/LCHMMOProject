#pragma once

class Session;

class DBMessage
{
public:
	DBMessage(SessionPtr owner);
	
	void OnResult();
	bool SqlExecute();
	
	virtual bool OnSqlExecute() = 0;
	virtual void OnSuccess() {}
	virtual void OnFail() {}

	SessionPtr session;
	bool isSuccess;
};

