#include "numerator.h"


QString Numerator::GenNewName(const char *prefix)
{
	QString name;
	m_GenNumber = m_MaxNumber + 1;
	name.sprintf("%s%d", prefix, m_GenNumber);
	return name;
}

void Numerator::UpdateNum(int num)
{
	if (num > m_MaxNumber)
		m_MaxNumber = num;
}

void Numerator::ResetNum()
{
	m_MaxNumber = m_GenNumber;
	m_GenNumber = 0;
}

