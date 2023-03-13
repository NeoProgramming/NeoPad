#pragma once
#include "BaseItem.h"
#include "DocItem.h"

struct FavItem : public BaseItem<FavItem>
{
	DocItem *ref = nullptr;
	QString title;
	enum EType {
		T_GROUP,
		T_REF
	} type;	

};
Q_DECLARE_METATYPE(FavItem*)
