#include "unicode.h"
#include <QFile>
#include <QRegExp>
#include <QString>
#include "../service/fail.h"

Unicode::Unicode()
{
	All.name = "All";
	Faves.name = "Faves";
	Recent.name = "Recent";
}

bool Unicode::Load(const char* fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly))
        return Fail("Unicode::Load: file.open() error"), false;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        auto lst = line.split(';');
        if(lst.size() >= 2) {
            unsigned int code = lst.at(0).toUInt(nullptr, 16);
            if(code < Count) {
                Data[code].code = code;
                QByteArray a = lst.at(1);
                Data[code].name = a.toStdString();
            }
        }
    }
    return true;
}

Unicode::Group* Unicode::Group::AddGroup(const char *name)
{

    return nullptr;
}

bool Unicode::Group::LoadGroups(const char *fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly))
        return Fail("Unicode::Group::LoadGroups: file.open() error"), false;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if(line.length()==0)
            continue;
        if(line[0]=='#')
            continue;
        QString s = line;
        QStringList parts = s.split(QRegExp("[\\.;]+"), QString::SkipEmptyParts);
        if(parts.size() >= 3) {
            unsigned int i = parts.at(0).toUInt(nullptr, 16);
            unsigned int j = parts.at(1).toUInt(nullptr, 16);
            if(i < Count && j < Count) {
                Group g;
                g.name = parts.at(2).toStdString();
                g.ranges.push_back(std::pair<unsigned int, unsigned int>(i, j));
                children.push_back(g);
            }
        }
    }

    return true;
}

unsigned int Unicode::Group::GetCount()
{
	unsigned int c = 0;
	for (auto &p : ranges) {
		c += (p.second - p.first + 1);
	}

	return c;
}

Unicode::Group::Iterator Unicode::Group::begin()
{
	Unicode::Group::Iterator it;
	it.ri = ranges.begin();
	it.re = ranges.end();
	if (it.ri != it.re)
		it.rc = it.ri->first;
	else
		it.rc = 0;
	return it;
}

Unicode::Group::Iterator Unicode::Group::end()
{
	Unicode::Group::Iterator it;
	it.ri = it.re = ranges.end();
	it.rc = 0;
	return it;
}

// Dereference operator
unsigned int Unicode::Group::Iterator::operator*() const 
{ 
	return rc; 
}

Unicode::Group::Iterator& Unicode::Group::Iterator::operator++()
{
	if (ri != re) {
		if (rc < ri->second) {
			++rc;
		}
		else {
			++ri;
			if (ri != re)
				rc = ri->first;
			else
				rc = 0;
		}
	}
	return *this;
}

bool Unicode::Group::Iterator::operator==(const Unicode::Group::Iterator& other) const 
{ 
	return ri == other.ri && re == other.re && rc == other.rc; 
}

bool Unicode::Group::Iterator::operator!=(const Unicode::Group::Iterator& other) const 
{ 
	return !(*this == other); 
}

