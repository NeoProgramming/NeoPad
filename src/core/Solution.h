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
#include "Workspace.h"
#include "Importants.h"

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
	Workspace WS;
    Columns   Cols;
	//Documents Docs; // temporary commented
	Favorites Favs;
	Importants Imps;
public:
	CSolution(void);
	~CSolution(void);
	void	loadSettings();
	void	saveSettings();
	void	addProjectToRecent(const QString &path);

	void    QuitProject();
    bool    MakeProject(const QString& name, const QString& dir, const QString &btitle0, const QString &bsuffix0);
    bool	LoadProject(const QString &fpath);
    bool	SaveProject(bool recursive);

    void    SaveProjectData(pugi::xml_node xRoot);

    // xml helpers, including base sructure and encryption/decryption
    bool    LoadXml(const QString &fpath, pugi::xml_document &xdoc, pugi::xml_node &xroot);
    void	MakeXml(pugi::xml_document &xdoc, pugi::xml_node &xroot, pugi::xml_node &xbase);
    bool	SaveXml(const QString &path, const pugi::xml_document &xdoc);

    // helpers
    QString GetPrjTitle();
    QString GetBookDir(int bi);
    QString GetCssAbsPath(int bi);
protected:
};

extern CSolution theSln;

