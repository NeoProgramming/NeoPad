#include "BaseItem.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "Solution.h"
#include "../service/tools.h"



BaseItem::BaseItem()
{
	attrs = 0;
	status = ETreeStatus::TS_UNREADY;
	check = 1;
}

BaseItem::~BaseItem()
{}

bool BaseItem::IsAncestor(BaseItem *item)
{
	// check if the given element is an ancestor of the item element
	while (item) {
		if (item == this)
			return true;
		item = item->parent;
	}
	return false;
}

bool BaseItem::IsPublic()
{
    // check if the document is published
    return status == ETreeStatus::TS_READY || status == ETreeStatus::TS_ALMOST;
}

int BaseItem::GetPublicChildrenCount()
{
	int count = 0;
	for (auto child : children) {
		if (child->IsPublic())
			count++;
	}
	return count;
}

void BaseItem::RemoveChildren()
{
	for (auto child : children) {
		child->RemoveChildren();
		delete child;
	}
	children.clear();
}

void BaseItem::SetCheck(bool _check)
{
	this->check = _check;
}

bool BaseItem::GetCheck()
{
	return check;
}

QString BaseItem::GetId()
{
	return id;
}

void BaseItem::ChangeModify(bool modify, bool recursive)
{
	p_modify = modify;
	if (recursive) 
		for (MTPOS child : children) 
			child->ChangeModify(modify, recursive);
}




