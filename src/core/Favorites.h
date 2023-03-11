#pragma once
#include "../service/pugitools.h"
#include "PrjTree.h"
#include "RefItem.h"

class Favorites : public PrjTree<RefItem>
{
public:
	void	LoadFavorites(pugi::xml_node txRoot);
	void	SaveFavorites(pugi::xml_node txRoot);

};

