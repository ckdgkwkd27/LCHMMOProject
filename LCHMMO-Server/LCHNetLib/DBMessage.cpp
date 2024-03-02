#include "pch.h"
#include "DBMessage.h"
#include "Session.h"

DBMessage::DBMessage(SessionPtr owner)
{
	this->session = owner;
}

void DBMessage::OnResult()
{
	if (isSuccess)
		OnSuccess();
	else
		OnFail();
}

bool DBMessage::SqlExecute()
{
	return OnSqlExecute();
}
