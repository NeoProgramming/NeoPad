#include "fail.h"

QString FailMsg;

void Fail(const QString &msg)
{
	FailMsg = msg;
}

void Fail(const char* msg)
{
	FailMsg = msg;
}

