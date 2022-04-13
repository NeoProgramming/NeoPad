#pragma once

#include <list>
#include <QString>
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

// tree element (main structural element of the base)
struct MT_ITEM
{
	union
	{
		unsigned long attrs;		// object attributes : 32
		struct
		{
			unsigned p_subbase : 1;	// own file or not
			unsigned p_modify : 1;	// modification sign
			unsigned check : 1;		// some document mark
		};
	};
public:
	// persistent variables
	ETreeStatus status = ETreeStatus::TS_UNREADY;    // status
	QString		title[BCNT];// custom title
	time_t		time[BCNT]; // times of last modification
	QString		id;
	QString     guid;
	// internal (non-persistent) variables - for ease of use
	QString     rdir;		// path to the folder where THIS vmbase file is located, relative to the base root (not used for all MY_ITEMs)
	// tree connections
	MT_ITEM*	parent = nullptr;
	std::list<MT_ITEM*>	children;
public:
	MT_ITEM();
	MT_ITEM(const char *text);
	MT_ITEM(const MT_ITEM &obj);

	bool	IsAncestor(MT_ITEM *item);
    bool    IsPublic();

	void	RemoveChildren();
	void    LoadItemPaths(const QString &apath);

	void    SetCheck(bool check);
	bool    GetCheck();
	void    SetDocTime(const char *tstr, int bi);
	void    SetDocTime(int bi);

	QString GetInfo();
	QString GetInfo2();
	QString GetUrl();

	QString GetId();
	QString GetGuid();
	QString GetTitle(int di);
	QString GetDocAbsPath(int di);
	QString GetDocRelPath(int bi);
	QString GetDocLocPath(int bi);
	time_t  GetDocTime(int di);
	QString GetVmbAbsPath();
	QString GetVmbLocPath();
	QString GetDocTimeStr(int bi);
	QString GetCssRelPath(int bi);

	ETreeStatus GetTreeStatus();
	ELangStatus GetLangStatus(int di2);
	

	void    ChangeModify(bool modify, bool recursive);

	QString GetAbsDir(int bi);
	QString GetBaseDir();
	void	UpdateBaseDir();

	QString GetRelPath(MT_ITEM *item, int di);
	QString GetRelUrl(MT_ITEM *item, int di);
};

// temporary
typedef MT_ITEM* MTPOS;
typedef std::list<MTPOS> CMtposList;

struct NeopadCallback
{
	virtual void* FindOpenedDoc(MTPOS pos, int di) = 0;
	virtual void  GetDocData(void* wnd, QString &html) = 0;
};

