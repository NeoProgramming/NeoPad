#pragma once
#include <list>
#include <functional>
#include <algorithm>
#include "BaseItem.h"

// usage:
// 1. CRTP for items: class MyItem : public BaseItem<MyItem> { ... }
// 2. PrjTree for tree: PrjTree<MyItem> my_tree;

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
		return (*i);
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
		return (*i);
	}
	
    
	void ForEach(T* node, const std::function<void(T*)> &fn)
	{
		if (!node)
			node = m_root;
		if (!m_root)
			return;
		fn(node);
		for (auto ch : node->children)
			ForEach(ch, fn);
	}

	void ForEach(const std::function<void(T*)> &fn)
	{
		ForEach(m_root, fn);
	}

	T* AddRoot()
	{
		if (!m_root)
			m_root = new T;
		return m_root;
	}

	T* AddCTail(T* par)
	{
		if (!par)
			return nullptr;
		T* item = new T;
		if (!item)
			return nullptr;
		par->children.push_back(item);
		item->parent = par;
		return item;
	}

	T* AddAfter(T* pos)
	{
		if (!pos || !pos->parent)
			return nullptr;
		
        typename std::list<T*>::iterator i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
		if (i == pos->parent->children.end())
			return nullptr;
		++i;

		T* item = new T;
		if (!item)
			return nullptr;

		pos->parent->children.insert(i, item);
		item->parent = pos->parent;
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

	void  RemoveNode(T *node)
	{
		node->RemoveChildren();
		if (node->parent) {
			node->parent->children.remove(node);
		}
		else {
			m_root = nullptr;
		}
		delete node;
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
		T* pos = *i1;
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

