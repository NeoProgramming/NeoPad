#pragma once
#include <QString>
#include <QMetaType>
#include "BaseItem.h"
#include "vmbsrv.h"
#include "TreeStatus.h"


// tree element (main structural element of the base)
struct DocItem : public BaseItem<DocItem>
{
public:
	// persistent variables
	union {
		unsigned long attrs = 0;	// object attributes : 32
		struct {
			unsigned p_subbase : 1;	// own file or not
			unsigned p_modify : 1;	// modification sign
			unsigned p_check : 1;	// some document mark
            unsigned p_remove : 1;  // mark for removed, used in clear-favorites algirithm
			unsigned p_empty : 1;	// empty document
		};
	};
	ETreeStatus status = ETreeStatus::TS_UNREADY;    // status
	QString		id;
	QString     guid;
    // move to struct?
    std::vector<QString> title;
    std::vector<time_t> time;

    // internal (non-persistent) variables - for ease of use
    QString     rdir;	    // path to the folder where THIS vmbase file is located, relative to the base root (not used for all MY_ITEMs)
    float       progress;   // calculated proress for this node
    int         procount;   // count of nodes in progress calculation
public:
	DocItem();
	DocItem(const char *text);
	DocItem(const DocItem &obj);

	bool    IsPublic();
	int     GetPublicChildrenCount();
	int     GetDescendantsCount();
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

	ETreeStatus GetTreeStatusCode();
	ELangStatus GetLangStatusCode(int di2);
	   
	QString GetAbsDir(int bi);
	QString GetBaseDir();
	void	UpdateBaseDir();
    void    UpdateProgress(int ci);
	QString GetRelPath(DocItem *item, int di);
	QString GetRelUrl(DocItem *item, int di);
};
Q_DECLARE_METATYPE(DocItem*)

struct NeopadCallback
{
	virtual void* FindOpenedDoc(DocItem* pos, int di) = 0;
	virtual void  GetDocData(void* wnd, QString &html) = 0;
};

