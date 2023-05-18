#pragma once

#include <QString>
#include <vector>
#include "vmbsrv.h"
#include "../service/pugitools.h"

enum EColumnType {
    CT_NONE,
    CT_BASE,
    CT_BOOK,
    CT_PROGRESS
};

struct NeopadCol
{
    EColumnType type;
    QString title;
    // for types: base, book
    QString suffix;
	QString rpath;			// path relative to NPBase
	QString csspath;
	QString load_prefix;	// for Jekyll
	QString save_prefix;	// for Jekyll
	bool path_is_unique;	// unique path requiring file operations

    bool isBook() const
    {
        return type==CT_BASE || type==CT_BOOK;
    }
};

class Columns {
public:
    int     BCnt()
    {
        return booksCnt;
    }
    void    AddInfo(EColumnType t, const QString &title);
    void    AddBook(EColumnType t, const QString &title, const QString &suffix, const QString &rpath, const QString &csspath, const QString &prefix);
    QString	GetDocExt(int di);
    bool	LoadBooksInfo(pugi::xml_node txRoot);
    void	SaveBooksInfo(pugi::xml_node txRoot);
	void    SetLPrefix(int bi);
	void    RemovePrefix(int bi, QString &content);
    EColumnType GetColType(int ci);
    int     ProgressCol();
public:
    std::vector<NeopadCol> books;
    int booksCnt = 0;
};
