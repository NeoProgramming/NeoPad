#ifndef SLNPANEL_H
#define SLNPANEL_H

#include "ui_slnpanel.h"

#include <functional>

#include <QList>
#include <QPair>
#include <QListWidget>
#include <QTreeWidget>
#include <QMap>
#include <QStringList>
#include <QValidator>
#include <QMenu>
#include <QHash>
#include "slntreewidget.h"

#include "../core/Solution.h"

class QProgressBar;
class MainWindow;

class SlnPanel : public QWidget
{
    Q_OBJECT
public:
    SlnPanel(QWidget *parent, MainWindow *h);

    bool eventFilter(QObject *, QEvent *);
	
	void UpdateDocItem(QTreeWidgetItem * item);
	void UpdateDocItem(QTreeWidgetItem * item, DocItem* tpos);
	void UpdateFavItem(QTreeWidgetItem * item);
	void UpdateNode(QTreeWidgetItem * item);

	void UpdateDocNode(QTreeWidgetItem * node, DocItem *inode);

	void UpdateTree();
	void UpdateDocItem(DocItem* item);

	void SetCurrItemStatus(ETreeStatus status);
	void SetCurrNodeStatus(ETreeStatus status);

    void Search(const QString &text);

	QIcon& GetTreeItemIcon(ETreeStatus i);
	QIcon& GetLangItemIcon(ELangStatus i);

    void EnsureVisible(DocItem* node);

    void UpdateBookTitles();

	void Load();
    void LoadDocTree();
	void LoadDocLevel(DocItem* node, QTreeWidgetItem *parent);
	void LoadFavTree();
	void LoadFavLevel(FavItem* node, QTreeWidgetItem *parent);
	
	void OpenInExtProgram(const QString& program, int di);
	void RemoveItemDontAsk(bool del_files);	//remove item without asking
	
private slots:
    void onSearch();
	void onFindNext();
	void onFindPrev();
	void onSelNode();

	void onDocDoubleClicked(QTreeWidgetItem* item, int column);
    void onResDoubleClicked(QTreeWidgetItem* item, int column);
	void onFavDoubleClicked(QTreeWidgetItem* item, int column);
	void onDocContextMenu(const QPoint &pos);
	void onFavContextMenu(const QPoint &pos);
	int  onDropping(QTreeWidgetItem *drag, QTreeWidgetItem *drop, int m);

	void onItemProperties();
	void onOpenInNewTab(int bi);
	void onOpenInExtBrowser(int bi);
	void onOpenInExtDocEditor(int bi);
	void onOpenInExtTextEditor(int bi);
	void onOpenFolder(int bi);
	void onOpenFolderVmb();
	void onDeleteDoc();
	void onDeleteDoc(int bi);
	void onOpenVmbaseInExtTextEditor();
	void onAddChildDoc();
	void onAddSiblingDoc();
	void onRemoveItem();
	void onDeleteItem();
	void onMoveItemUp();
	void onMoveItemDown();
	void onMoveItemParent();
	void onMoveItemChild();
	void onMoveItem();
	void onAddToFavorites();
	void onRemoveFromFavorites();
	void onEditFavoriteRef();
	void onAddSiblingGroup();
	void onAddChildGroup();
	void onEditGroup();
	void onAddSiblingFav();
	void onAddChildFav();

    void processEvents();
	
private:
	void initColumns(QTreeWidget *tree);
	QTreeWidgetItem* FindItem(QTreeWidgetItem *par, DocItem* mtpos);
    void showInitDoneMessage();
	DocItem* currDoc();
	FavItem* currFav();
	QTreeWidgetItem *AddWorkpieceItem(QTreeWidgetItem *par, QTreeWidgetItem *after, const QString &text, ETreeStatus st);
	void RemoveWorkpieceItem(QTreeWidgetItem *newitem, QTreeWidgetItem *olditem);
	       
	QAction *MakeAction(QString text, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QKeySequence skey, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QMenu *menu, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QMenu *menu, const std::function<void()> &fn);
	QAction *MakeAction(QString text, QKeySequence skey, QMenu *menu, void (SlnPanel::*slot)());

private:
	Ui::SlnForm ui;
	MainWindow *mw;
	QString searchRoot;	// guid
	QIcon m_TreeIcons[(int)ETreeStatus::TS_ITEMS_COUNT];
	QIcon m_LangIcons[(int)ELangStatus::LS_ITEMS_COUNT];

	bool initDoneMsgShown;

	QMenu *menuPopupDoc;
	QMenu *menuPopupGroup;
	QMenu *menuPopupRef;
	QMenu *menuPopupDangling;
			
	QMenu *submenuOpen0Ext;
	QMenu *submenuOpen1Ext;

	QAction *actionCheckTree;
	QAction *actionCheckText;
	QAction *actionCheckTags;
	QAction *actionCheckAttrs;
};

#endif // SLNPANEL_H
