#include "unicode.h"
#include <QFile>
#include <QRegExp>
#include <QString>
#include <algorithm>
#include "../service/fail.h"

Unicode::Unicode()
{
	All.name = "All";
	Faves.name = "Faves";
	Recent.name = "Recent";
	Quick.name = "Quick";
}

bool Unicode::Load(const QString& fpath)
{    
	QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly))
        return Fail("Unicode::Load: file.open() error"), false;
	if(!Data)
		Data = new Symbol[Count];

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        auto lst = line.split(';');
        if(lst.size() >= 2) {
            unsigned int code = lst.at(0).toUInt(nullptr, 16);
            if(code < Count) {
                Data[code].code = code;
                QByteArray a = lst.at(1);
                Data[code].name = a;
            }
        }
    }
    return true;
}

bool Unicode::Group::Load(const QString& fpath)
{
	QFile file(fpath);
	if (!file.open(QIODevice::ReadOnly))
		return Fail("Unicode::Group::Load: file.open() error"), false;
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		bool ok;
		unsigned int c = line.toUInt(&ok, 16);
		if (ok) {
			ranges.push_back(std::pair<unsigned int, unsigned int>(c, c));
		}
	}
	return true;
}

bool Unicode::Group::Save(const QString& fpath)
{
	QFile file(fpath);
	if (!file.open(QIODevice::WriteOnly))
		return Fail("Unicode::Group::Save: file.open() error"), false;
	for (auto i : *this) {
		QString s = QString::asprintf("%X\r\n", i, i);
		QByteArray a = s.toLocal8Bit();
		file.write(a);
	}
	return true;
}

Unicode::~Unicode()
{
	delete[] Data;
}

QString Unicode::GetName(unsigned int c)
{
    if(c >= Count)
        return "";
    return Data[c].name;
}

void Unicode::FindByName(const QString& name, Unicode::Group &gr)
{
	if (name.length() >= 3) {
		for (unsigned int i = 0; i < Count; i++) {
			if (Data[i].name.indexOf(name, 0, Qt::CaseInsensitive)>=0)
				gr.AddRange(i, i);
		}
	}
}

void Unicode::Group::AddRange(unsigned int from, unsigned int to)
{
	ranges.push_back(std::pair<unsigned int, unsigned int>(from, to));
}

void Unicode::Group::AddRecent(unsigned int c)
{
	// 1 searh & remove
	for (auto i : ranges) {
		if (i.first == c && c == i.second) {
			ranges.remove(i);
			break;
		}
	}
	// 2 push front
	ranges.push_front(std::pair<unsigned int, unsigned int>(c, c));

	// 3 if need remove tail 
	while (ranges.size() > 256)
		ranges.pop_back();
}

void Unicode::Group::AddQuick(unsigned int c)
{
	ranges.push_back(std::pair<unsigned int, unsigned int>(c, c));
}

bool Unicode::Group::LoadGroups(const QString& fpath)
{
	// 2200..22FF; Mathematical Operators
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly))
        return Fail("Unicode::Group::LoadGroups: file.open() error"), false;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if(line.length()==0)
            continue;
        if(line[0]=='#')
            continue;
        if(line[0]=='<')
            continue;
        if(line[0]=='>')
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

