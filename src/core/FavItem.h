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
public:
	void SetTitle(const QString &t);
};
Q_DECLARE_METATYPE(FavItem*)
