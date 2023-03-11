#pragma once
#include <list>
#include <functional>
#include <algorithm>
#include "BaseItem.h"

template<class T>
class PrjTree
{
public:
	T* GetRoot()
	{
		return m_root;
	}

	T* GetPrevSibling(T* pos)
	{
		if (!pos->parent)
			return nullptr;
		auto i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
		if (i == pos->parent->children.end())
			return nullptr;
		if (i == pos->parent->children.begin())
			return nullptr;
		--i;
		return (*i)->This<T>();
	}

	T* GetNextSibling(T* pos)
	{
		if (!pos->parent)
			return nullptr;
		auto i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
		if (i == pos->parent->children.end())
			return nullptr;
		i++;
		if (i == pos->parent->children.end())
			return nullptr;
		return (*i)->This<T>();
	}
	
    
	void ForEach(T* node, const std::function<void(T*)> &fn)
	{
		if (!node)
			node = m_root;
		fn(node);
		for (auto ch : node->children)
			ForEach(ch->This<T>(), fn);
	}

	void ForEach(const std::function<void(T*)> &fn)
	{
		ForEach(m_root, fn);
	}

	bool AddRoot(T *item)
	{
		if (!item)
			return false;
		if (!m_root)
			m_root = item;
		return true;
	}

	bool AddCTail(T* pos, T* item)
	{
		if (!pos || !item)
			return false;
		pos->children.push_back(item);
		item->parent = pos;
		return true;
	}

	bool AddAfter(T* pos, T* item)
	{
		if (!pos || !pos->parent || !item)
			return false;

		std::list<BaseItem*>::iterator i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
		if (i == pos->parent->children.end())
			return false;
		++i;

		pos->parent->children.insert(i, item);
		item->parent = pos->parent;
		return true;
	}
	
	T* AddCTail(T* pos)
	{
		T* item = new T;
		if (!AddCTail(pos, item)) {
			delete item;
			return nullptr;
		}
		return item;
	}

	template<class T>
	T* AddAfter(T* pos)
	{
		T* item = new T;
		if (!AddAfter(pos, item)) {
			delete item;
			return nullptr;
		}
		return item;
	}
	
	static bool  IsMovePossible(T* tpItem, T* tpNewPar)
	{
		// elements are not set or drag itself into itself
		if (!tpItem || !tpNewPar || tpItem == tpNewPar)
			return false;
		// you cannot drag an element to a descendant of this element
		if (tpItem->IsAncestor(tpNewPar))
			return false;
		return true;
	}


	static bool  IsExchangePossible(T* pos1, T* pos2)
	{
		if (!pos1->parent || !pos2->parent)
			return false;
		if (pos1->IsAncestor(pos2) || pos2->IsAncestor(pos1))
			return false;
		return true;
	}

	void  RemoveAll()
	{
		if (m_root) {
			m_root->RemoveChildren();
			delete m_root;
			m_root = nullptr;
		}
	}

	void  Exchange(T* pos1, T* pos2)
	{
		// exchange only pointers in tree nodes; not files and folders !!!
		if (!IsExchangePossible(pos1, pos2))
			return;
		// find items in lists; the items are POINTERS!
		auto i1 = std::find(pos1->parent->children.begin(), pos1->parent->children.end(), pos1);
		auto i2 = std::find(pos2->parent->children.begin(), pos2->parent->children.end(), pos2);
		// just change the contents of the list cells
		MTPOS pos = *i1;
		*i1 = *i2;
		*i2 = pos;
		// change parent pointers
		pos = pos1->parent;
		pos1->parent = pos2->parent;
		pos2->parent = pos;
	}
	    
protected:
	T* m_root = nullptr;
};

