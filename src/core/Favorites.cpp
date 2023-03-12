#include "Favorites.h"
#include "Solution.h"


void Favorites::LoadFavorites(pugi::xml_node txRoot)
{
	pugi::xml_node txFavs = txRoot.child("favorites");
	if (!txFavs)
		return;
	if (!AddRoot(new FavItem))
		return;
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
			item->type = FavItem::R_REF;
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
	for (auto elem : node->children) {
		
	}
}
