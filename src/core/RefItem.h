#pragma once
#include "BaseItem.h"
#include "DocItem.h"

struct RefItem : public BaseItem
{
	DocItem *ref = nullptr;
	QString title;
};
