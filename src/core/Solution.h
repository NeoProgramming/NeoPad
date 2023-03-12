#pragma once
#include <QStringList>
#include <QFile>

#include "../service/pugitools.h"
#include "../service/numerator.h"
#include "../service/fail.h"


#include "Snippets.h"
#include "vmbsrv.h"
#include "Documents.h"
#include "Favorites.h"

class CSolution 
	: public Documents // temporary
{
public:
	QString   m_sProgDir;		// directory with program and settings files
    QString   m_sPargPath;		// path to exe module (for display in 'about' dialog)
	Numerator m_fnum;			// file numbering
	QString   m_ImageDir;		// path to the directory with pictures
	Snippets  m_Snippets;
	QString   m_RecentClipboard;

	BooksInfo Books;
	//Documents Docs; // temporary commented
	Favorites Favs;
public:
	CSolution(void);
	~CSolution(void);
	void	loadSettings();
	void	saveSettings();
	void	addProjectToRecent(const QString &path);

	bool    CreateProject(const QString& name, const QString& dir, const QString &btitle0, const QString &bsuffix0);
	bool	LoadProject(const QString &fpath);
	bool	SaveProject(bool recursive);
};

extern CSolution theSln;

