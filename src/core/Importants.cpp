#include "Importants.h"
#include <QTextCodec>
extern QTextCodec *codecUtf8;

void Importants::Load(pugi::xml_node txRoot)
{
	AutoLoadedItems.clear();
	pugi::xml_node xlist = txRoot.child("autoload");
	if (xlist) {
		pugi::xml_node xitem = xlist.first_child();
		while (xitem) {
			AutoLoadedItems.push_back(codecUtf8->toUnicode(xitem.text().as_string()));
			xitem = xitem.next_sibling();
		}
	}
}

void Importants::Save(pugi::xml_node txRoot)
{
	pugi::xml_node xlist = txRoot.child("autoload");
	if (!xlist)
		xlist = txRoot.append_child("autoload");
	xlist.remove_children();
	for (const auto &item : AutoLoadedItems) {
		pugi::xml_node xitem = xlist.append_child("item");
		xitem.text().set(codecUtf8->fromUnicode(item).constData());
	}
}

void Importants::Add(const QString &guid)
{
	if (AutoLoadedItems.contains(guid))
		return;
	AutoLoadedItems.push_back(guid);
}

void Importants::Del(const QString &guid)
{
	AutoLoadedItems.removeAll(guid);
}

bool Importants::Contains(const QString &guid)
{
	return AutoLoadedItems.contains(guid);
}

void Importants::Toggle(const QString &guid)
{
	if (AutoLoadedItems.contains(guid))
		Del(guid);
	else
		Add(guid);
}
