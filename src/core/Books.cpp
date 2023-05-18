#include "Books.h"
#include <QDir>
#include "../service/tools.h"

void Columns::AddInfo(EColumnType t, const QString &title)
{
    NeopadCol base;
    base.type = t;
    base.title = title;

    books.push_back(base);
    booksCnt++;
}

void Columns::AddBook(EColumnType t, const QString &title, const QString &suffix, const QString &rpath,
    const QString &csspath, const QString &prefix)
{
    NeopadCol base;
    base.type = t;
    base.title = title;
    base.suffix = suffix;
    base.rpath = rpath;
    base.csspath = csspath;
    base.load_prefix = prefix;
    base.save_prefix = prefix;

    if (base.rpath.isEmpty())
        base.rpath = ".";
    if (base.csspath.isEmpty())
        base.csspath = "default.css";

    // unique path for base
    base.path_is_unique = QDir::cleanPath(base.rpath) != "." &&
        !std::count_if(books.begin(), books.end(), [&base](const NeopadCol &b)->bool {
            if(!b.isBook())
                return true;
            return QDir::cleanPath(base.rpath) == QDir::cleanPath(b.rpath);
        }
    );

    // only one element without prefix is possible
    if (base.suffix.isEmpty()) {
        for (int i = 0; i < booksCnt; i++)
            if (books[i].isBook() && books[i].suffix.isEmpty())
                return;
    }

    // add to bases
    books.push_back(base);
    booksCnt++;
}

QString Columns::GetDocExt(int bi)
{
    // get extension by document type
    if (bi < 0)
        return MBA::extVmbase;
    if (bi >= BCnt())
        return QString(".error") + MBA::extHtml;
    if(books[bi].suffix.isEmpty())
        return MBA::extHtml;
    return "." + books[bi].suffix + MBA::extHtml;
}

bool Columns::LoadBooksInfo(pugi::xml_node txRoot)
{
    pugi::xml_node txBases = txRoot.child("bases");
    if (!txBases)
        return false;

    books.clear();
    booksCnt = 0;
    pugi::xml_node txBase = txBases.first_child();
    while (txBase) {
        const char *t = txBase.name();
        EColumnType ct = CT_NONE;
        if(!strcmp(t, "base"))
            ct = CT_BASE;
        else if(!strcmp(t, "book"))
            ct = CT_BOOK;
        else if(!strcmp(t, "progress"))
            ct = CT_PROGRESS;

       // if(booksCnt==1 && ct == CT_BASE)
        //    ct = CT_BOOK;

        if(ct == CT_BASE || ct == CT_BOOK) {
            AddBook(
                ct,
                U16(txBase.attribute("title").as_string()),
                U16(txBase.attribute("suffix").as_string()),
                U16(txBase.attribute("rpath").as_string()),
                U16(txBase.attribute("csspath").as_string()),
                U16(txBase.attribute("prefix").as_string())
                );
        }
        else if(ct == CT_PROGRESS) {
            AddInfo(ct, U16(txBase.attribute("title").as_string()));
        }
        txBase = txBase.next_sibling();
    }
    return (booksCnt >= 1);
}


void Columns::SaveBooksInfo(pugi::xml_node txRoot)
{
    pugi::xml_node txBases = txRoot.append_child("bases");
    if (!txBases)
        return;
    for (const NeopadCol& base : books) {
        const char *types[] = {"none", "base", "book", "progress"};

        pugi::xml_node txBase = txBases.append_child(types[base.type]);
        set_attr(txBase, "title").set_value(U8a(base.title).constData());

        if(base.type == CT_BASE || base.type == CT_BOOK) {
            set_attr(txBase, "suffix").set_value(U8a(base.suffix).constData());
            set_attr(txBase, "rpath").set_value(U8a(base.rpath).constData());
            set_attr(txBase, "csspath").set_value(U8a(base.csspath).constData());
            set_attr(txBase, "prefix").set_value(U8a(base.save_prefix).constData());
        }
    }
}

void Columns::SetLPrefix(int bi)
{
	books[bi].load_prefix = books[bi].save_prefix;
}

void Columns::RemovePrefix(int bi, QString &content)
{
	if (!books[bi].load_prefix.isEmpty()) {
		if (content.startsWith(books[bi].load_prefix))
			content = content.mid(books[bi].load_prefix.length());
	}
}

EColumnType Columns::GetColType(int ci)
{
    if(ci<0 || ci>=booksCnt)
        return CT_NONE;
    return books[ci].type;
}

int Columns::ProgressCol()
{
    for(int i=0; i<books.size(); i++)
        if(books[i].type == CT_PROGRESS)
            return i;
    return -1;
}

