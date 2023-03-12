#pragma once

#include <list>
#include <QString>
#include <QMetaType>
#include "vmbsrv.h"


template<class T>
struct BaseItem
{
	T*	parent = nullptr;	// tree connections
	std::list<T*> children;	// tree connections
public:

	bool	IsAncestor(T *item)
	{
		// check if the given element is an ancestor of the item element
		while (item) {
			if (item == this)
				return true;
			item = item->parent;
		}
		return false;
	}

	void	RemoveChildren()
	{
		for (auto child : children) {
			child->RemoveChildren();
			delete child;
		}
		children.clear();
	}
};


