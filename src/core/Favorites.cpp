#include <QTextCodec>
#include <QDebug>
#include "Favorites.h"
#include "Solution.h"
#include "ini.h"

extern QTextCodec *codecUtf8;

Favorites::Favorites()
{
	MakeRoot();
}

void Favorites::MakeRoot()
{
	RemoveAll();
	if (!AddRoot())
		return;
	m_root->type = FavItem::T_GROUP;
	m_root->title = "ALL FAVORITES";
}

void Favorites::LoadFavorites(pugi::xml_node txRoot)
{
	pugi::xml_node txFavs = txRoot.child("favorites");
	if (!txFavs)
		return;
	MakeRoot();
	
	LoadFavoritesLevel(txFavs, m_root);
}

void Favorites::LoadFavoritesLevel(pugi::xml_node txNode, FavItem *node)
{
	pugi::xml_node txElem = txNode.first_child();
	while (txElem) {
		FavItem* item = AddCTail(node);

		auto name = txElem.name();
		if (!strcmp(name, "group")) {
			item->type = FavItem::T_GROUP;
			item->title = txElem.attribute("title").as_string();
			item->ref = nullptr;
			LoadFavoritesLevel(txElem, item);
		}
		else if (!strcmp(name, "ref")) {
			item->type = FavItem::T_REF;
			item->title = txElem.attribute("guid").as_string();
			item->ref = theSln.Locate(item->title);
		}

		// next item
		txElem = txElem.next_sibling();
	}
}

void Favorites::SaveFavorites(pugi::xml_node txRoot)
{
	pugi::xml_node txFavs = txRoot.child("favorites");
	if (!txFavs)
		txFavs = txRoot.append_child("favorites");
	txFavs.remove_children();
	SaveFavoritesLevel(txFavs, m_root);
}

void Favorites::SaveFavoritesLevel(pugi::xml_node txNode, FavItem *node)
{
	for (auto item : node->children) {
		if (item->type == FavItem::T_GROUP) {
			pugi::xml_node txItem = txNode.append_child("group");
			set_attr(txItem, "title").set_value(codecUtf8->fromUnicode(item->title).constData());
			SaveFavoritesLevel(txItem, item);
		}
		else {
			pugi::xml_node txItem = txNode.append_child("ref");
			set_attr(txItem, "guid").set_value(codecUtf8->fromUnicode(item->title).constData());
		}
	}
}

FavItem* Favorites::AddGroup(FavItem* tpPar, FavItem* tpAfter, const QString& title)
{
	FavItem* item = tpAfter ? AddAfter(tpAfter) : AddCTail(tpPar);
	item->type = FavItem::T_GROUP;
	item->title = title;
    HandleChanges();
	return item;
}

FavItem* Favorites::AddRef(FavItem* tpPar, FavItem* tpAfter, DocItem *ref)
{
	FavItem* item = tpAfter ? AddAfter(tpAfter) : AddCTail(tpPar);
	item->type = FavItem::T_REF;
	item->ref = ref;
    item->title = ref->guid;
    HandleChanges();
	return item;
}

bool Favorites::RemoveNode(FavItem* tpItem)
{
	FavItem* tpPar = tpItem->parent;
	if (!tpPar)
		return false;
	PrjTree::RemoveNode(tpItem);
    HandleChanges();
	return true;
}

void Favorites::ChangeTitle(FavItem* item, const QString & title)
{
	item->title = title;
    HandleChanges();
}

void Favorites::ChangeRef(FavItem* item, DocItem *ref)
{
    item->ref = ref;
    item->title = ref->guid;
    HandleChanges();
}

void Favorites::HandleChanges()
{
    theSln.Favs.m_bModify = true;
	if (INI::AutoSavePages)
	{
        qDebug() << "auto save";
        theSln.SaveSubBase(theSln.GetRoot(), false);
	}
}
