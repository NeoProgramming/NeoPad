#include "Bases.h"
#include <QDir>
#include "../service/tools.h"

void Bases::AddBase(const QString &title, const QString &suffix, const QString &rpath,
    const QString &csspath, const QString &prefix)
{
    NeopadBook base;
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
        !std::count_if(&books[0], &books[BCNT], [&base](const NeopadBook&b)->bool {
            return QDir::cleanPath(base.rpath) == QDir::cleanPath(b.rpath);
        }
    );

    // only one element without prefix is possible
    if (base.suffix.isEmpty()) {
        for (int i = 0; i < booksCnt; i++)
            if (books[i].suffix.isEmpty())
                return;
    }

    // add to bases
    books[booksCnt] = base;
    booksCnt++;
}

QString Bases::GetDocExt(int bi)
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

bool Bases::LoadBasesInfo(pugi::xml_node txRoot)
{
    pugi::xml_node txBases = txRoot.child("bases");
    if (!txBases)
        return false;

    booksCnt = 0;
    pugi::xml_node txBase = txBases.first_child();
    while (txBase && booksCnt<BCNT) {

        AddBase(
            U16(txBase.attribute("title").as_string()),
            U16(txBase.attribute("suffix").as_string()),
            U16(txBase.attribute("rpath").as_string()),
            U16(txBase.attribute("csspath").as_string()),
            U16(txBase.attribute("prefix").as_string())
            );

        txBase = txBase.next_sibling();
    }
    return (booksCnt >= 1);
}


void Bases::SaveBasesInfo(pugi::xml_node txRoot)
{
    pugi::xml_node txBases = txRoot.append_child("bases");
    if (!txBases)
        return;
    for (const NeopadBook& base : books) {
        pugi::xml_node txBase = txBases.append_child("base");
        set_attr(txBase, "title").set_value(U8a(base.title).constData());
        set_attr(txBase, "suffix").set_value(U8a(base.suffix).constData());
        set_attr(txBase, "rpath").set_value(U8a(base.rpath).constData());
        set_attr(txBase, "csspath").set_value(U8a(base.csspath).constData());
        set_attr(txBase, "prefix").set_value(U8a(base.save_prefix).constData());
    }
}
