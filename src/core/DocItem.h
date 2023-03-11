#pragma once
#include "BaseItem.h"

// tree element (main structural element of the base)
struct DocItem : public BaseItem
{
public:
	// move all BCNT-items to struct
	// persistent variables

	QString		title[BCNT];// custom title
	time_t		time[BCNT]; // times of last modification

	// internal (non-persistent) variables - for ease of use
	QString     rdir;		// path to the folder where THIS vmbase file is located, relative to the base root (not used for all MY_ITEMs)

public:
	DocItem();
	DocItem(const char *text);
	DocItem(const DocItem &obj);
		
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
