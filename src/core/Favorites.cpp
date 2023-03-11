#include "Favorites.h"



void Favorites::LoadFavorites(pugi::xml_node txRoot)
{
	pugi::xml_node txFavs = txRoot.child("favorites");
	if (!txFavs)
		return;
	/*
	// child reading loop
	pugi::xml_node txElem = txFavs.first_child();
	while (txElem)
	{
		// adding an empty element to the tree
		MTPOS tpItem = AddCTail(tpNode);

		// see what kind of element is in xml
		if (txElem.child(MBA::node))
			LoadSubTag(txElem, tpItem);

		// next item
		txElem = txElem.next_sibling();
	}*/
}

void Favorites::SaveFavorites(pugi::xml_node txRoot)
{

}