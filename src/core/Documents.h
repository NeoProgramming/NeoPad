#pragma once
#include <QStringList>
#include <QFile>
#include "PrjTree.h"
#include "DocItem.h"
#include "Books.h"

class Documents : public PrjTree<DocItem> {
public:

	static ETreeStatus	GetTreeStatus(const char* attr);
	static const char*	GetTreeStatus(ETreeStatus status);

public:
	Columns *m_BI = nullptr;
	bool m_bModify = false;	// an indication that the tree or files have been modified at least once since the start of the program
	QString m_RootDir;		// main base for xml files
	QString m_Password;
	NeopadCallback *m_pCB = nullptr;

public:
	// work with the project
    bool	LoadSubBase(const QString &id, DocItem* tpParNode);
	bool	SaveSubBase(DocItem* tpPar, bool recursive);

    bool	LoadRootBase(const QString &fpath, pugi::xml_node xRoot);
    bool	SaveRootBase(bool recursive, pugi::xml_node xBase);
    bool	MakeRootBase(const QString &name, const QString& dir, pugi::xml_node xBase);

    // load/save html documents
	bool    LoadDoc(DocItem* item, int bi, QString &content);
	bool    SaveDoc(DocItem* item, int bi, const QString &content);
    bool	MakeDoc(DocItem* tpItem, int bi);

	DocItem*AddItem(DocItem* tpPar, DocItem* tpAfter, const QString& title, const QString& id);
	bool	RenameItem(DocItem* tpos, const QString & id);
	void    RenameTitle(DocItem* item, const QString & title, int bi);
	void    SetStatus(DocItem* item, ETreeStatus status, bool rec);

	bool	RemoveNode(DocItem* tpItem, bool del_files);
	void    RemoveNodeFiles(DocItem* tpItem);
	void	RemoveNodeDoc(DocItem* tpItem, int bi);	

	bool    Move(DocItem* tpItem, DocItem* tpNewPar, DocItem* tpAfter);
	bool    MoveUp(DocItem* tpItem);
	bool    MoveDown(DocItem* tpItem);
	bool    MoveChild(DocItem* tpItem);
	bool    MoveParent(DocItem* tpItem);

	void    SaveItem(DocItem* tpItem, int bi);
	void    MakeUnsavedList(std::list<DocItem*> &mpl);
	void	GenContents(int bi, const QString &fpath, const QString &base);

	void    EncryptDocs(DocItem* tposParent, const QString &oldPsw, const QString &newPsw);
	void    TransformDocs(int bi);
	bool	TransformFile(DocItem* tpos, int bi);

	DocItem*Locate(const QString &guid);
	void    Search(const QString &text, unsigned int scope, DocItem* root, std::list<DocItem*> &results);
	bool    SearchInFile(DocItem* pos, const QString &text, unsigned int scope);

	void    HandleChanges(DocItem* tpItem, bool recursive);
	void    HandleChanges(DocItem* tpItem1, DocItem* tpItem2);
	bool    MoveFiles(DocItem* tpItem, DocItem* tpNewPar);
    void	MakeUnsavedLevel(DocItem* mtNode, std::list<DocItem*> &mpl);
	void    GenContentsLevel(DocItem* node, int bi, QFile &file, const QString &base);
protected:
	bool    IsFNamesAvailable(DocItem* pos, const QString &id);
    DocItem*GetAncestorWithFile(DocItem* item, bool include_this = false);
    void	UpdateRelDirs(DocItem* tpNode);
    DocItem*CreateRoot(const QString& name, const QString& dir);

    bool	LoadSubTag(pugi::xml_node txPar, DocItem* tpPar);
    void	LoadItemData(pugi::xml_node txItem, DocItem *item);

	void	SaveSubTag(pugi::xml_node pxParent, DocItem* tposParent, bool recursive);
	void    SaveItemData(pugi::xml_node txItem, DocItem *item);
};
