#pragma once

#include <list>
#include <QString>
#include <QMetaType>
#include "vmbsrv.h"

// statuses (pictures) of the main items in the tree
enum class ETreeStatus 
{
	TS_UNKNOWN,		// unknown what ...
	TS_READY,		// regular document
	TS_ALMOST,		
	TS_75,
	TS_50,
	TS_25,
	TS_UNREADY,		// unfinished document
	TS_LOCKED,		// ready but blocked for publication
	TS_FOLDER,

	TS_ITEMS_COUNT
};

// statuses (pictures) of transfers in the tree
enum class ELangStatus
{
	LS_NONE,	// no translation
	LS_OK,		// translation is newer than the original
	LS_OLD,		// the original is newer than the translation - you need to translate
	LS_QOK,     // the translation is newer than the original, one of the dates is unreliable
	LS_QOLD,    // the original is newer than the translation, one of the dates is unreliable
	LS_ITEMS_COUNT
};

struct BaseItem
{
	union {
		unsigned long attrs;		// object attributes : 32
		struct {
			unsigned p_subbase : 1;	// own file or not
			unsigned p_modify : 1;	// modification sign
			unsigned check : 1;		// some document mark
		};
	};
	ETreeStatus status = ETreeStatus::TS_UNREADY;    // status
	QString		id;
	QString     guid;
	BaseItem*	parent = nullptr;	// tree connections
	std::list<BaseItem*> children;	// tree connections
public:
	BaseItem();
	virtual ~BaseItem();

	bool	IsAncestor(BaseItem *item);
	bool    IsPublic();
	void    SetCheck(bool check);
	bool    GetCheck();
	QString GetId();
	void    ChangeModify(bool modify, bool recursive);
	int     GetPublicChildrenCount();
	void	RemoveChildren();

	template<class T>
	T* This()
	{
		return dynamic_cast<T*>(this);
	}

	template<class T>
	T* Par()
	{
		return dynamic_cast<T*>(parent);
	}
};
Q_DECLARE_METATYPE(BaseItem*)

// temporary
typedef BaseItem* MTPOS;

struct NeopadCallback
{
	virtual void* FindOpenedDoc(MTPOS pos, int di) = 0;
	virtual void  GetDocData(void* wnd, QString &html) = 0;
};

