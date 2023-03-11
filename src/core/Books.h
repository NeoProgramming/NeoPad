#pragma once

#include <QString>
#include "vmbsrv.h"
#include "../service/pugitools.h"

class Books {
public:
    int     BCnt()
    {
        return booksCnt;
    }
    void    AddBook(const QString &title, const QString &suffix, const QString &rpath, const QString &csspath, const QString &prefix);
    QString	GetDocExt(int di);
    bool	LoadBooksInfo(pugi::xml_node txRoot);
    void	SaveBooksInfo(pugi::xml_node txRoot);
	void    SetLPrefix(int bi);
	void    RemovePrefix(int bi, QString &content);
public:
    NeopadBook	books[BCNT];
    int booksCnt = 0;
};
