#pragma once
#include "../service/pugitools.h"
#include "PrjTree.h"
#include "FavItem.h"

class Favorites : public PrjTree<FavItem>
{
public:
	void	LoadFavorites(pugi::xml_node txRoot);
	void	SaveFavorites(pugi::xml_node txRoot);
protected:
	void	LoadFavoritesLevel(pugi::xml_node txNode, FavItem *node);
	void    SaveFavoritesLevel(pugi::xml_node txNode, FavItem *node);
};

