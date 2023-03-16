#pragma once
#include <QString>
#include <QMetaType>
#include "BaseItem.h"
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

// tree element (main structural element of the base)
struct DocItem : public BaseItem<DocItem>
{
public:
	// move all BCNT-items to struct
	// persistent variables
	union {
		unsigned long attrs = 0;	// object attributes : 32
		struct {
			unsigned p_subbase : 1;	// own file or not
			unsigned p_modify : 1;	// modification sign
			unsigned check : 1;		// some document mark
            unsigned p_remove : 1;  // mark for removed, used in clear-favorites algirithm
		};
	};
	ETreeStatus status = ETreeStatus::TS_UNREADY;    // status
	QString		id;
	QString     guid;
	QString		title[BCNT];// custom title
	time_t		time[BCNT]; // times of last modification

	// internal (non-persistent) variables - for ease of use
	QString     rdir;		// path to the folder where THIS vmbase file is located, relative to the base root (not used for all MY_ITEMs)

public:
	DocItem();
	DocItem(const char *text);
	DocItem(const DocItem &obj);

	bool    IsPublic();
	int     GetPublicChildrenCount();
	void    SetCheck(bool check);
	bool    GetCheck();
	QString GetId();
	void    ChangeModify(bool modify, bool recursive);
		
	void    LoadItemPaths(const QString &apath);
	void    SetDocTime(const char *tstr, int bi);
	void    SetDocTime(int bi);

	QString GetInfo();
	QString GetInfo2();
		
	QString GetGuid();
	QString GetTitle(int bi);
	QString GetTitles(int bi);
	QString GetDocAbsPath(int bi);
	QString GetDocRelPath(int bi);
	QString GetDocLocPath(int bi);
	time_t  GetDocTime(int bi);
	QString GetVmbAbsPath();
	QString GetVmbLocPath();
	QString GetDocTimeStr(int bi);
	QString GetCssRelPath(int bi);

	ETreeStatus GetTreeStatus();
	ELangStatus GetLangStatus(int di2);
	   
	QString GetAbsDir(int bi);
	QString GetBaseDir();
	void	UpdateBaseDir();

	QString GetRelPath(DocItem *item, int di);
	QString GetRelUrl(DocItem *item, int di);
};
Q_DECLARE_METATYPE(DocItem*)

struct NeopadCallback
{
	virtual void* FindOpenedDoc(DocItem* pos, int di) = 0;
	virtual void  GetDocData(void* wnd, QString &html) = 0;
};

