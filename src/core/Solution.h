#pragma once
#include <QStringList>
#include <QFile>

#include "../service/pugitools.h"
#include "../service/numerator.h"
#include "../service/fail.h"

#include "PrjTree.h"
#include "DocItem.h"
#include "Snippets.h"
#include "vmbsrv.h"
#include "Books.h"
#include "Favorites.h"

class CSolution : public PrjTree<DocItem>
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
    Books m_Books;
	Favorites m_Favs;
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
	bool	SaveSubBase(DocItem* tpPar, bool recursive);

	bool    LoadDoc(DocItem* item, int bi, QString &content);
	bool    SaveDoc(DocItem* item, int bi, const QString &content);
	
	DocItem*AddItem(DocItem* tpPar, DocItem* tpAfter, const QString& title, const QString& id);
	bool	RenameItem(DocItem* tpos, const QString & id);
	void    RenameTitle(DocItem* item, const QString & title, int bi);
	void    SetStatus(DocItem* item, ETreeStatus status, bool rec);

	bool	RemoveNode(DocItem* tpItem, bool del_files);
	void    RemoveNodeFiles(DocItem* tpItem);
	void	RemoveNodeDoc(DocItem* tpItem, int bi);
		
	bool	MakeDoc(DocItem* tpItem, int bi);

	bool    Move(DocItem* tpItem, DocItem* tpNewPar, DocItem* tpAfter);
	bool    MoveUp(DocItem* tpItem);
	bool    MoveDown(DocItem* tpItem);
	bool    MoveChild(DocItem* tpItem);
	bool    MoveParent(DocItem* tpItem);
	
    void    SaveItem(DocItem* tpItem, int bi);
	void    MakeUnsavedList(std::list<DocItem*> &mpl);
	void	GenContents(int bi, const QString &fpath, const QString &base);
	QString GetCssAbsPath(int bi);
	void    EncryptDocs(DocItem* tposParent, const QString &oldPsw, const QString &newPsw);
	void    TransformDocs(int bi);
	bool	TransformFile(DocItem* tpos, int bi);

	DocItem*Locate(const QString &guid);
    void    Search(const QString &text, unsigned int scope, DocItem* root, std::list<DocItem*> &results);
    bool    SearchInFile(DocItem* pos, const QString &text, unsigned int scope);
protected:
	bool    IsFNamesAvailable(DocItem* pos, const QString &id);

	bool    LoadXmlDoc(const QString &fpath, pugi::xml_document &xdoc, pugi::xml_node &xroot);
	void	MakeXmlDoc(pugi::xml_document &xdoc, pugi::xml_node &xroot, pugi::xml_node &xbase);
	bool	SaveXmlDoc(const QString &path, const pugi::xml_document &xdoc);

	DocItem*   GetAncestorWithFile(DocItem* item, bool include_this = false);

	void	UpdateBaseDirs(DocItem* tpNode);
	DocItem* CreateRoot(const QString& name, const QString& dir);

	bool	LoadSubTag(pugi::xml_node txPar, DocItem* tpPar);
	bool	LoadSubBase(const QString &id, DocItem* tpParNode);
	void	LoadItemData(pugi::xml_node txItem, DocItem *item);
	
	void	SaveSubTag(pugi::xml_node pxParent, DocItem* tposParent, bool recursive);
	void    SaveItemData(pugi::xml_node txItem, DocItem *item);
	
	void	addProjectToRecent(const QString &path);
	
	void    HandleChanges(DocItem* tpItem, bool recursive);
	void    HandleChanges(DocItem* tpItem1, DocItem* tpItem2);
	bool    MoveFiles(DocItem* tpItem, DocItem* tpNewPar);
	void	MakeUnsavedListR(std::list<DocItem*> &mpl, DocItem* mtNode);
	void    GenContentsLevel(DocItem* node, int bi, QFile &file, const QString &base);
};

extern CSolution theSln;

