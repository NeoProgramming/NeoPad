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

class MainWindow;

struct TREEITEM {
	QTreeWidgetItem *qitem = nullptr;
	DocItem *doc = nullptr;
	FavItem *fav = nullptr;

	bool bad()    { return !qitem || (!doc && !fav); }
	bool badDoc() { return !qitem || !doc; }
	bool badFav() { return !qitem || !fav; }
};

class SlnPanel : public QWidget
{
    Q_OBJECT
public:
    SlnPanel(QWidget *parent, MainWindow *h);

    bool eventFilter(QObject *, QEvent *);
	
	void UpdateTreesIfNeeded(bool all);

    void UpdateDocItemsByObj(DocItem* item);
    void UpdateDocItem(QTreeWidgetItem * item, DocItem* tpos);
    void UpdateDocItem(QTreeWidgetItem * item);
    void UpdateFavItem(QTreeWidgetItem * item);
   
    void UpdateDocNode(QTreeWidgetItem * qnode, DocItem *node);
    void UpdateDocTree();
    void UpdateFavTree();
	void UpdateSymbols();
    void Update(QTreeWidgetItem *qitem, DocItem *doc);

    void Search(const QString &text);
    void EnsureVisible(DocItem* node);
	void Load();
	void LoadBookTitles();	
	void SaveDocs();
	void SaveFavs();
private slots:
    void onSearch();
	void onFindNext();
	void onFindPrev();
	void onSelNode();

    void onTabChanged(int tab);
	void onDocDoubleClicked(QTreeWidgetItem* item, int column);
    void onResDoubleClicked(QTreeWidgetItem* item, int column);
	void onFavDoubleClicked(QTreeWidgetItem* item, int column);
	void onDocContextMenu(const QPoint &pos);
	void onFavContextMenu(const QPoint &pos);
    void onFavRootChanged(int index);
	int  onDropping(QTreeWidgetItem *drag, QTreeWidgetItem *drop, int m);
	void onDoubleClickSymbol(QTableWidgetItem *item);

	void onItemProperties();
	void onOpenInNewTab(int bi);
	void onOpenInExtBrowser(int bi);
	void onOpenInExtDocEditor(int bi);
	void onOpenInExtTextEditor(int bi);
    void onOpenVmbaseInExtTextEditor();
    void onOpenFolder(int bi);
	void onOpenFolderVmb();
	void onCloseDocs(bool recursive, bool invert);

    void onAddChildDoc();
	void onAddSiblingDoc();
	void onRemoveItem();
	void onDeleteItem();
    void onDeleteDoc();
    void onDeleteDoc(int bi);

	void onMoveItemUp();
	void onMoveItemDown();
	void onMoveItemParent();
	void onMoveItemChild();
	void onMoveItem();
	
	void onAddToFavorites();
	void onRemoveFromFavorites();
	void onMarkAsImportant();
	void onCopyLink();
	void onEditFavoriteRef();
	void onAddSiblingGroup();
	void onAddChildGroup();
	void onEditGroup();
	void onAddSiblingFav();
	void onAddChildFav();

    void processEvents();
	
private:
	void resizeEvent(QResizeEvent* event);

	TREEITEM CurrItem();

    void SetCurrItemStatus(ETreeStatus status);
    void SetCurrNodeStatus(ETreeStatus status);

    QIcon& GetTreeItemIcon(ETreeStatus i);
    QIcon& GetLangItemIcon(ELangStatus i);

    void LoadDocTree();
    void LoadDocLevel(DocItem* node, QTreeWidgetItem *parent);
    void LoadFavTree(FavItem* root);
    void LoadFavLevel(FavItem* node, QTreeWidgetItem *parent);
    void LoadFavCombo();
    void ReloadFavCombo(FavItem *item);

    void OpenInExtProgram(const QString& program, int di);
    void RemoveItemDontAsk(bool del_files);	//remove item without asking
	void initTree(QTreeWidget *tree);
	
    void ForEachItem(QTreeWidgetItem *par, const std::function<bool(QTreeWidgetItem *)> fn);
    int  FindChildItemIndex(QTreeWidgetItem *par, DocItem* item, int startIndex);

    QAction *MakeAction(QString text, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QKeySequence skey, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QMenu *menu, void (SlnPanel::*slot)());
	QAction *MakeAction(QString text, QMenu *menu, const std::function<void()> &fn);
	QAction *MakeAction(QString text, QKeySequence skey, QMenu *menu, void (SlnPanel::*slot)());

private:
	Ui::SlnForm ui;
	MainWindow *mw;
    bool m_contentsNeedsToRefresh = false;
    bool m_favoritesNeedsToRefresh = false;
	QString searchRoot;	// guid
	QIcon m_TreeIcons[(int)ETreeStatus::TS_ITEMS_COUNT];
	QIcon m_LangIcons[(int)ELangStatus::LS_ITEMS_COUNT];
	QFont m_fontSymbols;

	QMenu *menuPopupDoc;
    QMenu *menuPopupAllFav;
	QMenu *menuPopupGroup;
	QMenu *menuPopupRef;
	QMenu *menuPopupDangling;
			
	QMenu *submenuOpen0Ext;
	QMenu *submenuOpen1Ext;
	QMenu *submenuClose;

	QAction *actionImportant;
	QAction *actionCopyLink;

	QAction *actionCheckTree;
	QAction *actionCheckIds;
	QAction *actionCheckText;
	QAction *actionCheckTags;
	QAction *actionCheckAttrs;
};

#endif // SLNPANEL_H
