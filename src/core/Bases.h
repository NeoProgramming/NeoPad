#pragma once

#include <QString>
#include "vmbsrv.h"
#include "../service/pugitools.h"

class Bases {
public:
    int     BCnt()
    {
        return booksCnt;
    }
    void    AddBase(const QString &title, const QString &suffix, const QString &rpath, const QString &csspath, const QString &prefix);
    QString	GetDocExt(int di);
    bool	LoadBasesInfo(pugi::xml_node txRoot);
    void	SaveBasesInfo(pugi::xml_node txRoot);
public:
    NeopadBook	books[BCNT];
    int booksCnt = 0;
};
