#include "PrjTree.h"
#include <algorithm>
#include <QDir>

PrjTree::PrjTree()
{
}


PrjTree::~PrjTree()
{
}

MTPOS PrjTree::GetRoot()
{
	return m_root;
}

MTPOS PrjTree::AddRoot()
{
	if (!m_root)
		m_root = new MT_ITEM;
	return m_root;
}

MTPOS PrjTree::AddCTail(MTPOS pos, MT_ITEM *copy)
{
	if (!pos)
		return nullptr;
	MTPOS p = new MT_ITEM;
	if (copy)
		*p = *copy;
	pos->children.push_back(p);
	p->parent = pos;
	return p;
}

MTPOS PrjTree::AddAfter(MTPOS pos, MT_ITEM *copy)
{
	if (!pos || !pos->parent)
		return nullptr;
	
	std::list<MT_ITEM*>::iterator i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
	if (i == pos->parent->children.end())
		return nullptr;
	++i;

	MTPOS p = new MT_ITEM;
	if (copy)
		*p = *copy;
	pos->parent->children.insert(i, p);
	p->parent = pos->parent;
	return p;
}

void PrjTree::RemoveAll()
{
	if (m_root) {
		m_root->RemoveChildren();
		delete m_root;
		m_root = nullptr;
	}
}

MTPOS PrjTree::GetPrevSibling(MTPOS pos)
{
	if (!pos->parent)
		return nullptr;
	auto i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
	if (i == pos->parent->children.end())
		return nullptr;
	if (i == pos->parent->children.begin())
		return nullptr;
	--i;
	return *i;
}

MTPOS PrjTree::GetNextSibling(MTPOS pos)
{
	if (!pos->parent)
		return nullptr;
	auto i = std::find(pos->parent->children.begin(), pos->parent->children.end(), pos);
	if (i == pos->parent->children.end())
		return nullptr;
	i++;
	if (i == pos->parent->children.end())
		return nullptr;
	return *i;
}

bool PrjTree::IsMovePossible(MTPOS tpItem, MTPOS tpNewPar)
{
	// elements are not set or drag itself into itself
	if (!tpItem || !tpNewPar || tpItem == tpNewPar)
		return false;
	// you cannot drag an element to a descendant of this element
	if (tpItem->IsAncestor(tpNewPar))
		return false;
	return true;
}

bool PrjTree::IsExchangePossible(MTPOS pos1, MTPOS pos2)
{
	if (!pos1->parent || !pos2->parent)
		return false;
	if (pos1->IsAncestor(pos2) || pos2->IsAncestor(pos1))
		return false;
	return true;
}

void PrjTree::Exchange(MTPOS pos1, MTPOS pos2)
{
	// exchange only tree nodes; not files and folders !!!
	if (!IsExchangePossible(pos1, pos2))
		return;
	// find items in lists; the items are POINTERS!
	auto i1 = std::find(pos1->parent->children.begin(), pos1->parent->children.end(), pos1);
	auto i2 = std::find(pos2->parent->children.begin(), pos2->parent->children.end(), pos2);
	// just change the contents of the list cells
	MTPOS pos = *i1;
	*i1 = *i2;
	*i2 = pos;
	// change parent elements
	pos = pos1->parent;
	pos1->parent = pos2->parent;
	pos2->parent = pos;
}

void PrjTree::ForEach(const std::function<void(MTPOS)> &fn)
{
    ForEach(m_root, fn);
}

void PrjTree::ForEach(MTPOS node, const std::function<void(MTPOS)> &fn)
{
	if (!node)
		node = m_root;
    fn(node);
    for(auto ch : node->children)
        ForEach(ch, fn);
}
