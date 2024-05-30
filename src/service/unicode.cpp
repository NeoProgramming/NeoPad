#include "unicode.h"
#include <QFile>
#include <QRegExp>
#include <QString>
#include "../service/fail.h"

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
        QByteArray line = file.readLine();
        if(line.length()==0)
            continue;
        if(line[0]=='#')
            continue;
        QString s = line;
        QStringList parts = s.split(QRegExp("[\\.\\s;]+"), QString::SkipEmptyParts);
        if(parts.size() >= 3) {
            unsigned int i = parts.at(0).toUInt(nullptr, 16);
            unsigned int j = parts.at(1).toUInt(nullptr, 16);
            if(i < Count && j < Count) {
                Group g;
                g.name = parts.at(2).toStdString();
                g.ranges.push_back(QPair<unsigned int, unsigned int>(i, j));
                children.push_back(g);
            }
        }
    }

    return true;
}
