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

    inline QTabWidget *tabWidget() const  { return ui.tabWidget; }
    bool eventFilter(QObject *, QEvent *);
	
	void DoChangeTreeItemStatus(ETreeStatus status, bool rec);
	void UpdateItem(QTreeWidgetItem * item);
	void UpdateNode(QTreeWidgetItem * item);

	void UpdateTree();
	void UpdateTreeItem(MTPOS item);

	void SetCurrItemStatus(ETreeStatus status);
	void SetCurrNodeStatus(ETreeStatus status);

    void Search(const QString &text);

	QIcon& GetTreeItemIcon(ETreeStatus i);
	QIcon& GetLangItemIcon(ELangStatus i);

    void EnsureVisible(MTPOS node);

    void initialize();
	void UpdateBases();
    void LoadTree();
	void LoadTreeLevel(MTPOS node, QTreeWidgetItem *parent);
	
	void OpenDoc(QTreeWidgetItem *item, int di);
	void OpenInExtProgram(const QString& program, int di);
	void RemoveItemDontAsk(bool del_files);	//remove item without asking
	
private slots:
    void onSearch();
	void onFindNext();
	void onFindPrev();

	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onResDoubleClicked(QTreeWidgetItem* curItem, int column);
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

	void onInsertNewChild();
	void onInsertNewSibling();
	void onRemoveItem();
	void onDeleteItem();
	void onMoveItemUp();
	void onMoveItemDown();
	void onMoveItemParent();
	void onMoveItemChild();
	void onMoveItem();

    void onShowContentsMenu(const QPoint &pos);
      
    void processEvents();
	
private:
   
	MTPOS GetMtposFromRes();
	QTreeWidgetItem* FindItem(QTreeWidgetItem *par, MTPOS mtpos);
    void showInitDoneMessage();
            
    Ui::SlnForm ui;
    MainWindow *mw;
	QIcon m_TreeIcons[(int)ETreeStatus::TS_ITEMS_COUNT];
	QIcon m_LangIcons[(int)ELangStatus::LS_ITEMS_COUNT];
	
    bool initDoneMsgShown;
        
	QAction *MakeAction(QString text, const char *slot);
	QAction *MakeAction(QString text, QKeySequence skey, const char *slot);
	QAction *MakeAction(QString text, QMenu *menu, const char *slot);
	QAction *MakeAction(QString text, QMenu *menu, const std::function<void()> &fn);
	QAction *MakeAction(QString text, QKeySequence skey, QMenu *menu, const char *slot);
    
	QMenu *menuPopupContents;
		
	QAction *actionOpenVmbaseInTextEditor;
	QAction *actionOpenFolder;
	QAction *actionItemProperties;
	QAction *actionItemMove;

	QMenu *submenuOpen0Ext;
	QMenu *submenuOpen1Ext;

	QAction *actionCheckTree;
	QAction *actionCheckText;
	QAction *actionCheckTags;
	QAction *actionCheckAttrs;
};

#endif // SLNPANEL_H
