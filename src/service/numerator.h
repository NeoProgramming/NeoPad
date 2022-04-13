#pragma once
#include <QString>

class Numerator
{
public:
	QString GenNewName(const char *prefix);
	void    UpdateNum(int num);
	void    ResetNum();
private:
	int m_GenNumber = 0;		// current generated document number Doc% d
	int m_MaxNumber = 0;		// maximum document number of Doc format% d
};
