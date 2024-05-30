#pragma once
#include <string>
#include <QPair>
#include <QList>
#include <QSharedPointer>

class Unicode {
public:
    struct Symbol {
        unsigned int code = 0;
        std::string name;
    };
    struct Group {
        std::string name;
        QList<QPair<unsigned int, unsigned int> > ranges;
        QList<Group> children;

        Group* AddGroup(const char *name);
        bool   LoadGroups(const char *fpath);
    };

public:
    bool Load(const char* fpath);
public:
    Group All;
    Group Recent;
    Group Faves;
private:
    enum { Count = 0xF0000 };
    Symbol Data[Count];

};

