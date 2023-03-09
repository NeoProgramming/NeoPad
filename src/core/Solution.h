#pragma once
#include <QStringList>
#include <QFile>

#include "../service/pugitools.h"
#include "../service/numerator.h"
#include "../service/fail.h"

#include "PrjTree.h"
#include "Snippets.h"
#include "vmbsrv.h"
#include "Bases.h"


class CSolution : public PrjTree
{
public:


	static ETreeStatus	GetTreeStatus(const char* attr);
	static const char*	GetTreeStatus(ETreeStatus status);
	
public:
	bool m_bModify;			// an indication that the tree or files have been modified at least once since the start of the program
	QString m_sProgDir;		// directory with program and settings files
    QString m_sPargPath;    // path to exe module (for display in 'about' dialog)
	Numerator m_fnum;		// file numbering
	QString m_ImageDir;		// path to the directory with pictures
	Snippets m_Snippets;
	QString m_RootDir;		// main base for xml files
	QString m_Password;
	NeopadCallback *m_pCB = nullptr;
	QString m_RecentClipboard;
    Bases m_Bases;
public:
	CSolution(void);
	~CSolution(void);
	void	LoadSettings();
	void	SaveSettings();

    QString GetBookDir(int bi);
	QString GetPrjTitle();

	// work with the project
	void	CreateProject(const QString &name, const QString& dir, const QString &btitle0, const QString &bsuffix0);
	bool	LoadProject(const QString &fpath);
	bool	SaveProject(bool recursive);
	bool	SaveSubBase(MTPOS tpPar, bool recursive);

	bool    LoadDoc(MTPOS item, int di, QString &content);
	bool    SaveDoc(MTPOS item, int di, const QString &content);

	MTPOS	AddItem(MTPOS tpPar, MTPOS tpAfter, const QString& title, const QString& id);
	bool	RenameItem(MTPOS tpos, const QString & id);
	void    RenameTitle(MTPOS item, const QString & title, int di);
	void    SetStatus(MTPOS item, ETreeStatus status, bool rec);

	bool	RemoveNode(MTPOS tpItem, bool del_files);
	void    RemoveNodeFiles(MTPOS tpItem);
	void	RemoveNodeDoc(MTPOS tpItem, int bi);
		
	bool	MakeDoc(MTPOS tpItem, int bi);

	bool    Move(MTPOS tpItem, MTPOS tpNewPar, MTPOS tpAfter);
	bool    MoveUp(MTPOS tpItem);
	bool    MoveDown(MTPOS tpItem);
	bool    MoveChild(MTPOS tpItem);
	bool    MoveParent(MTPOS tpItem);
	
    void    SaveItem(MTPOS tpItem, int bi);
	void    MakeUnsavedList(CMtposList &mpl);
	void	GenContents(int bi, const QString &fpath, const QString &base);
	QString GetCssAbsPath(int bi);
	void    EncryptDocs(MTPOS tposParent, const QString &oldPsw, const QString &newPsw);
	void    TransformDocs(int bi);
	bool	TransformFile(MTPOS tpos, int bi);

	MTPOS	Locate(const QString &guid);
    void    Search(const QString &text, unsigned int scope, MTPOS root, CMtposList &results);
    bool    SearchInFile(MTPOS pos, const QString &text, unsigned int scope);
protected:
	bool    IsFNamesAvailable(MTPOS pos, const QString &id);

	bool    LoadXmlDoc(const QString &fpath, pugi::xml_document &xdoc, pugi::xml_node &xroot);
	void	MakeXmlDoc(pugi::xml_document &xdoc, pugi::xml_node &xroot, pugi::xml_node &xbase);
	bool	SaveXmlDoc(const QString &path, const pugi::xml_document &xdoc);

	MTPOS   GetAncestorWithFile(MTPOS item, bool include_this = false);

	void	UpdateBaseDirs(MTPOS tpNode);
	MTPOS   CreateRoot(const QString& name, const QString& dir);

	bool	LoadSubTag(pugi::xml_node txPar, MTPOS tpPar);
	bool	LoadSubBase(const QString &id, MTPOS tpParNode);
	void	LoadItemData(pugi::xml_node txItem, MT_ITEM *item, bool vmb);

	void	LoadFavorites(pugi::xml_node txRoot);

	void	SaveSubTag(pugi::xml_node pxParent, MTPOS tposParent, bool recursive);
	void    SaveItemData(pugi::xml_node txItem, MT_ITEM *item);

	void	SaveFavorites(pugi::xml_node txRoot);

	void	addProjectToRecent(const QString &path);
	
	void    HandleChanges(MTPOS tpItem, bool recursive);
	void    HandleChanges(MTPOS tpItem1, MTPOS tpItem2);
	bool    MoveFiles(MTPOS tpItem, MTPOS tpNewPar);
	void	MakeUnsavedListR(CMtposList &mpl, MTPOS mtNode);
	void    GenContentsLevel(MTPOS node, int bi, QFile &file, const QString &base);
};

extern CSolution theSln;

