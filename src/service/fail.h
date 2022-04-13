#pragma once
#include <QString>

extern QString FailMsg;

void Fail(const QString &msg);
void Fail(const char* msg);
