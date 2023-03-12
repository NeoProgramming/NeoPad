#pragma once
#include "../service/pugitools.h"
#include "PrjTree.h"
#include "FavItem.h"

class Favorites : public PrjTree<FavItem>
{
public:
	bool	m_bModify = false;
public:
	void	LoadFavorites(pugi::xml_node txRoot);
	void	SaveFavorites(pugi::xml_node txRoot);
	FavItem*AddGroup(FavItem* tpPar, FavItem* tpAfter, const QString& title);
	FavItem*AddRef(FavItem* tpPar, FavItem* tpAfter, DocItem *ref);
	bool	RemoveNode(FavItem* tpItem);
	void    RenameTitle(FavItem* item, const QString & title);
protected:
	void	LoadFavoritesLevel(pugi::xml_node txNode, FavItem *node);
	void    SaveFavoritesLevel(pugi::xml_node txNode, FavItem *node);
};

