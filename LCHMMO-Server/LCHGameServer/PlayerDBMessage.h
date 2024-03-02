#pragma once
#include "DBMessage.h"
#include "DBHelper.h"
#include "ClientSession.h"

class JoinPlayerDBMessage : public DBMessage
{
public:
	JoinPlayerDBMessage(ClientSessionPtr owner);

	virtual void OnSuccess();
	virtual void OnFail();

	// Inherited via DBMessage
	virtual bool OnSqlExecute() override;

	std::string id;
	std::string pwd;
	int32 resultId;
	ClientSessionPtr playerSession;
};

class LoginPlayerDBMessage : public DBMessage
{
public:
	LoginPlayerDBMessage(ClientSessionPtr owner);

	virtual void OnSuccess();
	virtual void OnFail();

	// Inherited via DBMessage
	virtual bool OnSqlExecute() override;

	std::string id;
	std::string pwd;
	int32 resultId;
	ClientSessionPtr playerSession;
};