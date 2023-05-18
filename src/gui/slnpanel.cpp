#include "slnpanel.h"
#include "topicchooser.h"
#include "mainwindow.h"
#include "itemproperty.h"
#include "newitemdlg.h"
#include "existitemdlg.h"

#include <QtGui>
#include <QtDebug>
#include <QTreeWidgetItem>
#include <QInputDialog>

#include <stdlib.h>
#include <limits.h>

#include "../core/vmbsrv.h"
#include "../core/ini.h"
#include "../service/tools.h"
#include "../service/search.h"

extern QTextCodec *codecUtf8;


SlnPanel::SlnPanel(QWidget *parent, MainWindow *h)
: QWidget(parent), mw(h)
{
	ui.setupUi(this);

	QString fmt;
	fmt.sprintf("background-color: #%06X; alternate-background-color: #%06X", INI::BackColor1, INI::BackColor2);
	ui.treeContents->setStyleSheet(fmt);//"background-color: #FFC782; alternate-background-color: #F0CF72");
			
	initTree(ui.treeContents);
	initTree(ui.treeFavorites);

	connect(ui.treeContents, SIGNAL(dropping(QTreeWidgetItem*, QTreeWidgetItem*, int)), this, SLOT(onDropping(QTreeWidgetItem*, QTreeWidgetItem*, int)));
    connect(ui.pushSearch,		&QPushButton::clicked, this, &SlnPanel::onSearch);
	connect(ui.pushFindNext,	&QPushButton::clicked, this, &SlnPanel::onFindNext);
	connect(ui.pushFindPrev,	&QPushButton::clicked, this, &SlnPanel::onFindPrev);
	connect(ui.pushSelNode,		&QPushButton::clicked, this, &SlnPanel::onSelNode);

	m_TreeIcons[(int)ETreeStatus::TS_UNKNOWN] = QIcon(":/treeicons/images/ti-unknown.png");
	m_TreeIcons[(int)ETreeStatus::TS_READY] = QIcon(":/treeicons/images/ti-html.png");
	m_TreeIcons[(int)ETreeStatus::TS_ALMOST] = QIcon(":/treeicons/images/ti-htmlx.png");
	m_TreeIcons[(int)ETreeStatus::TS_75] = QIcon(":/treeicons/images/ti-html75.png");
	m_TreeIcons[(int)ETreeStatus::TS_50] = QIcon(":/treeicons/images/ti-html50.png");
	m_TreeIcons[(int)ETreeStatus::TS_25] = QIcon(":/treeicons/images/ti-html25.png");
	m_TreeIcons[(int)ETreeStatus::TS_UNREADY] = QIcon(":/treeicons/images/ti-html0.png");
	m_TreeIcons[(int)ETreeStatus::TS_LOCKED] = QIcon(":/treeicons/images/ti-locked.png");
	m_TreeIcons[(int)ETreeStatus::TS_FOLDER] = QIcon(":/treeicons/images/ti-folder.png");

	m_LangIcons[(int)ELangStatus::LS_NONE] = QIcon(":/langicons/images/li-none.png");
	m_LangIcons[(int)ELangStatus::LS_OK] = QIcon(":/langicons/images/li-ok.png");
	m_LangIcons[(int)ELangStatus::LS_OLD] = QIcon(":/langicons/images/li-old.png");
	m_LangIcons[(int)ELangStatus::LS_QOK] = QIcon(":/langicons/images/li-qok.png");
	m_LangIcons[(int)ELangStatus::LS_QOLD] = QIcon(":/langicons/images/li-qold.png");

	QMenu *menu = new QMenu(this);	
	actionCheckTree = menu->addAction("Search in Tree Titles");
	actionCheckIds  = menu->addAction("Search in Tree Ids");
	actionCheckText = menu->addAction("Search in Text");
	actionCheckTags = menu->addAction("Search in HTML Tags");
	actionCheckAttrs= menu->addAction("Search in HTML Attributes");
//	menu->addSeparator();
//	menu->addAction("Match case");
//	menu->addAction("Whole words");

	actionCheckTree->setCheckable(true);
	actionCheckIds->setCheckable(true);
	actionCheckText->setCheckable(true);
	actionCheckTags->setCheckable(true);
	actionCheckAttrs->setCheckable(true);

	actionCheckTree->setChecked(true);
	actionCheckText->setChecked(true);

	ui.pushOptions->setMenu(menu);

	ui.comboFavRoot->addItem("ALL FAVORITES");

	// show tab titles, do not elide them by placing "..."
	ui.tabWidget->setElideMode(Qt::ElideNone);

	// trees
	// you need to set 'CustomContextMenu' in 'ContextMenuPolicy' in form editor
	connect(ui.treeContents,  &QTreeWidget::customContextMenuRequested, this, &SlnPanel::onDocContextMenu);
	connect(ui.treeFavorites, &QTreeWidget::customContextMenuRequested, this, &SlnPanel::onFavContextMenu);
	connect(ui.treeContents,  &QTreeWidget::itemDoubleClicked, this, &SlnPanel::onDocDoubleClicked);
	connect(ui.treeResults,	  &QTreeWidget::itemDoubleClicked, this, &SlnPanel::onResDoubleClicked);
	connect(ui.treeFavorites, &QTreeWidget::itemDoubleClicked, this, &SlnPanel::onFavDoubleClicked);
    connect(ui.comboFavRoot,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SlnPanel::onFavRootChanged);

	ui.tabWidget->setCurrentIndex(0);

	ui.treeContents->viewport()->installEventFilter(this);	// event filter - for mouse
	ui.treeContents->installEventFilter(this);				// event filter - for keyboard
	ui.treeFavorites->viewport()->installEventFilter(this);	// event filter - for mouse
	ui.treeFavorites->installEventFilter(this);				// event filter - for keyboard

	// toolbar buttons
	connect(mw->ui.actionTreeMoveUp, &QAction::triggered, this, &SlnPanel::onMoveItemUp);
	connect(mw->ui.actionTreeMoveDown, &QAction::triggered, this, &SlnPanel::onMoveItemDown);
	connect(mw->ui.actionTreeMoveParent, &QAction::triggered, this, &SlnPanel::onMoveItemParent);
	connect(mw->ui.actionTreeMoveChild, &QAction::triggered, this, &SlnPanel::onMoveItemChild);
	connect(mw->ui.actionTreeAddChild, &QAction::triggered, this, &SlnPanel::onAddChildDoc);
	connect(mw->ui.actionTreeAddSibling, &QAction::triggered, this, &SlnPanel::onAddSiblingDoc);
	connect(mw->ui.actionTreeRemoveItem, &QAction::triggered, this, &SlnPanel::onRemoveItem);
    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &SlnPanel::onTabChanged);

	menuPopupDoc = new QMenu(tr("Document"), this);
	submenuOpen0Ext = new QMenu(tr("Doc0"), menuPopupDoc);
	submenuOpen1Ext = new QMenu(tr("Doc1"), menuPopupDoc);

	// doc0
	MakeAction(tr("Open in New Tab"), submenuOpen0Ext, [this]() { onOpenInNewTab(0); });
	MakeAction(tr("Open in External Doc Editor"), submenuOpen0Ext, [this]() { onOpenInExtDocEditor(0); });
	MakeAction(tr("Open in External Browser"), submenuOpen0Ext, [this]() { onOpenInExtBrowser(0); });
	MakeAction(tr("Open in External Text Editor"), submenuOpen0Ext, [this]() { onOpenInExtTextEditor(0); });
	MakeAction(tr("Open Folder"), submenuOpen0Ext, [this]() { onOpenFolder(0); });

	// doc1
	MakeAction(tr("Open in New Tab"), submenuOpen1Ext, [this]() { onOpenInNewTab(1); });
	MakeAction(tr("Open in External Doc Editor"), submenuOpen1Ext, [this]() { onOpenInExtDocEditor(1); });
	MakeAction(tr("Open in External Browser"), submenuOpen1Ext, [this]() { onOpenInExtBrowser(1); });
	MakeAction(tr("Open in External Text Editor"), submenuOpen1Ext, [this]() { onOpenInExtTextEditor(1); });
	MakeAction(tr("Open Folder"), submenuOpen1Ext, [this]() { onOpenFolder(1); });

	// common actions
	QAction *actionOpenFolder = MakeAction(tr("Open Folder"), &SlnPanel::onOpenFolderVmb);
	QAction *actionOpenVmbaseInTextEditor = MakeAction(tr("Open VMB in Text Editor"), &SlnPanel::onOpenVmbaseInExtTextEditor);
	QAction *actionItemMove = MakeAction(tr("Move item..."), &SlnPanel::onMoveItem);

    // shortcuts does not trigger action, see eventFilter()
	QAction *actionItemProperties = MakeAction(tr("Item properties..."), QKeySequence(Qt::ControlModifier + Qt::Key_Space), &SlnPanel::onItemProperties);
	QAction *actionItemAddToFavs = MakeAction(tr("Add to favorites"), &SlnPanel::onAddToFavorites);

	QMenu *submenuItemStatus = new QMenu(tr("Item Status"), this);// menuPopupDoc);
	MakeAction(tr("Ready"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_READY); });
	MakeAction(tr("Almost ready"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_ALMOST); });
	MakeAction(tr("75 %"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_75); });
	MakeAction(tr("50 %"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_50); });
	MakeAction(tr("25 %"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_25); });
	MakeAction(tr("Under construction"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_UNREADY); });
	MakeAction(tr("Locked"), submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_LOCKED); });

	QMenu *submenuNodeStatus = new QMenu(tr("Node Status"), this);//menuPopupDoc);
	MakeAction(tr("Ready"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_READY); });
	MakeAction(tr("Almost ready"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_ALMOST); });
	MakeAction(tr("75 %"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_75); });
	MakeAction(tr("50 %"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_50); });
	MakeAction(tr("25 %"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_25); });
	MakeAction(tr("Under construction"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_UNREADY); });
	MakeAction(tr("Locked"), submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_LOCKED); });

	QMenu *submenuInsert = new QMenu(tr("Insert"), this);//menuPopupDoc);
	MakeAction(tr("New subitem"),      QKeySequence(Qt::Key_Insert), submenuInsert, &SlnPanel::onAddChildDoc);
	MakeAction(tr("New sibling item"), QKeySequence(Qt::Key_Enter), submenuInsert,  &SlnPanel::onAddSiblingDoc);

	QMenu *submenuDelete = new QMenu(tr("Delete"), this);//menuPopupDoc);
	MakeAction(tr("Remove item (do not touch source files)"), QKeySequence(Qt::Key_Delete), submenuDelete, &SlnPanel::onRemoveItem);
	MakeAction(tr("Delete item and source files"), submenuDelete, &SlnPanel::onDeleteItem);
	MakeAction(tr("Delete document file"), submenuDelete, &SlnPanel::onDeleteDoc);

	// formation of a context menu
	menuPopupDoc->addAction(actionItemProperties);
    menuPopupDoc->addSeparator();

    menuPopupDoc->addAction(actionItemAddToFavs);
	menuPopupDoc->addSeparator();

	menuPopupDoc->addMenu(submenuOpen0Ext);
	menuPopupDoc->addMenu(submenuOpen1Ext);
	menuPopupDoc->addAction(actionOpenVmbaseInTextEditor);
	menuPopupDoc->addAction(actionOpenFolder);
	menuPopupDoc->addSeparator();

	menuPopupDoc->addMenu(submenuItemStatus);
	menuPopupDoc->addMenu(submenuNodeStatus);
	menuPopupDoc->addSeparator();

	menuPopupDoc->addMenu(submenuInsert);
	menuPopupDoc->addMenu(submenuDelete);
	menuPopupDoc->addAction(actionItemMove);

	QAction *actionAddSiblingGroup = MakeAction(tr("Add sibling group"), &SlnPanel::onAddSiblingGroup);
	QAction *actionAddChildGroup = MakeAction(tr("Add child group"), &SlnPanel::onAddChildGroup);
	QAction *actionAddSiblingFav = MakeAction(tr("Add sibling favorite"), &SlnPanel::onAddSiblingFav);
	QAction *actionAddChildFav = MakeAction(tr("Add child favorite"), &SlnPanel::onAddChildFav);
	QAction *actionRemoveFromFavs = MakeAction(tr("Remove from favorites"), &SlnPanel::onRemoveFromFavorites);
	QAction *actionEditFavRef = MakeAction(tr("Change fav reference"), &SlnPanel::onEditFavoriteRef);
	QAction *actionEditGroup = MakeAction(tr("Edit group name"), &SlnPanel::onEditGroup);

	// favorites context menus
    menuPopupAllFav = new QMenu(this);
    menuPopupAllFav->addAction(actionAddChildGroup);
    menuPopupAllFav->addAction(actionAddChildFav);

	menuPopupGroup = new QMenu(this);
	menuPopupGroup->addAction(actionAddSiblingGroup);
	menuPopupGroup->addAction(actionAddChildGroup);
	menuPopupGroup->addAction(actionAddSiblingFav);
	menuPopupGroup->addAction(actionAddChildFav);
	menuPopupGroup->addAction(actionRemoveFromFavs);
	menuPopupGroup->addAction(actionEditGroup);

	menuPopupDangling = new QMenu(this);
	menuPopupDangling->addAction(actionEditFavRef);
	menuPopupDangling->addAction(actionRemoveFromFavs);
	
	menuPopupRef = new QMenu(this);
	menuPopupRef->addMenu(menuPopupDoc);
	menuPopupRef->addAction(actionEditFavRef);
	menuPopupRef->addAction(actionRemoveFromFavs);
}

TREEITEM SlnPanel::CurrItem()
{
	TREEITEM item;
	int tab = ui.tabWidget->currentIndex();
	if (tab == 0) {
		item.qitem = ui.treeContents->currentItem();
		if (item.qitem) {
			item.fav = nullptr;
			item.doc = item.qitem->data(0, Qt::UserRole).value<DocItem*>();
		}
	}
	else if (tab == 2) {
		item.qitem = ui.treeFavorites->currentItem();
		if (item.qitem) {
			item.fav = item.qitem->data(0, Qt::UserRole).value<FavItem*>();
			if (item.fav && item.fav->ref)
				item.doc = item.fav->ref;
			else
				item.doc = item.qitem->data(0, Qt::UserRole).value<DocItem*>();
		}
	}
	return item;
}

void SlnPanel::initTree(QTreeWidget *tree)
{
	tree->setRootIsDecorated(false);
	tree->setUniformRowHeights(true);			// all lines are the same height
	
	QHeaderView * header = tree->header();

	header->setStretchLastSection(false);		// the last section is NOT the full available width
//	header->setSectionResizeMode(QHeaderView::ResizeToContents);	// header resizing mode
	header->setSectionResizeMode(QHeaderView::Interactive);
	tree->setColumnWidth(0, 220);
	tree->setColumnWidth(1, 220);
	tree->setIconSize(QSize(20, 20));
}

QAction *SlnPanel::MakeAction(QString text, QMenu *menu, void (SlnPanel::*slot)())
{
	QAction *action = new QAction(this);
	action->setText(text);
	menu->addAction(action);
	connect(action, &QAction::triggered, this, slot);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, QMenu *menu, const std::function<void()> &fn)
{
	QAction *action = new QAction(this);
	action->setText(text);
	menu->addAction(action);
	connect(action, &QAction::triggered, this, fn);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, void (SlnPanel::*slot)())
{
	QAction *action = new QAction(this);
	action->setText(text);
	connect(action, &QAction::triggered, this, slot);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, QKeySequence skey, void (SlnPanel::*slot)())
{
	QAction *action = new QAction(this);
	action->setText(text);
	action->setShortcut(skey);
	connect(action, &QAction::triggered, this, slot);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, QKeySequence skey, QMenu *menu, void (SlnPanel::*slot)())
{
	QAction *action = new QAction(this);
	action->setText(text);
	action->setShortcut(skey);
	menu->addAction(action);
	connect(action, &QAction::triggered, this, slot);
	return action;
}

void SlnPanel::processEvents()
{
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool SlnPanel::eventFilter(QObject * o, QEvent * e)
{
	QString objName = o->objectName();
	int key = static_cast<QKeyEvent*>(e)->key();
	quint32 modifiers = QApplication::keyboardModifiers();
	if (o == ui.treeContents) {
		if (e->type() == QEvent::KeyPress) {
			switch (key) {
			case Qt::Key_Insert:
				if (modifiers == Qt::NoModifier)
					onAddChildDoc();
				break;
			case Qt::Key_Delete:
				if (modifiers == Qt::ShiftModifier)
					onDeleteItem();
				else if (modifiers == Qt::NoModifier)
					onRemoveItem();
				break;
			// open in-plce edit
			//case Qt::Key_F2:
			//	ui.treeContents->openPersistentEditor(ui.treeContents->currentItem(), 0);
			//	break;
			case Qt::Key_Space:
				if (modifiers == Qt::ControlModifier)
					onItemProperties();
				break;
			case Qt::Key_Enter:
			case Qt::Key_Return:
				if (modifiers == Qt::NoModifier)
					onAddSiblingDoc();
				break;
			default:
				break;
			}
		}
	}
	else if (o == ui.treeFavorites) {
		if (e->type() == QEvent::KeyPress) {
			switch (key) {
			case Qt::Key_Insert:
                if (modifiers == Qt::NoModifier) {
                    if(CurrItem().doc)
                        onAddChildDoc();
                    else
                        onAddChildGroup();
                }
				else if (modifiers == Qt::ShiftModifier)
					onAddChildFav();
				break;
			case Qt::Key_Delete:
			//	if (modifiers == Qt::ShiftModifier)
			//		onDeleteItem();
			//	else if (modifiers == Qt::NoModifier)
			//		onRemoveItem();
			//	break;
			case Qt::Key_Enter:
			case Qt::Key_Return:
                if (modifiers == Qt::NoModifier) {
                    if(CurrItem().doc)
                        onAddSiblingDoc();
                    else
                        onAddSiblingGroup();
                }
				else if (modifiers == Qt::ShiftModifier)
					onAddSiblingFav();
				break;
			}
		}
	}
	return QWidget::eventFilter(o, e);
}

void SlnPanel::LoadBookTitles()
{
	QTreeWidgetItem* headerItem;
	int n = theSln.Cols.BCnt();
	ui.treeContents->setColumnCount(n); 
	ui.treeFavorites->setColumnCount(n);

    headerItem  = ui.treeContents->headerItem();
    for(int i=0; i<n; i++) {
        headerItem->setText(i, theSln.Cols.books[i].title);
    }
    headerItem = ui.treeFavorites->headerItem();
    for(int i=0; i<n; i++) {
        headerItem->setText(i, theSln.Cols.books[i].title);
    }

    submenuOpen0Ext->setTitle(tr("Doc0 (%1)").arg(theSln.Cols.books[0].title));
	if(n>=2)
		submenuOpen1Ext->setTitle(tr("Doc1 (%1)").arg(theSln.Cols.books[1].title));
	else
		submenuOpen1Ext->setTitle(tr("Doc1 (---)"));
}

void SlnPanel::LoadDocLevel(DocItem* tposNode, QTreeWidgetItem *parent)
{
	for (auto tpos : tposNode->children)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(parent);
		item->setData(0, Qt::UserRole, QVariant::fromValue(tpos));
        UpdateDocItem(item);
		LoadDocLevel(tpos, item);
	}
}

void SlnPanel::Load()
{
	setCursor(Qt::WaitCursor);
	LoadDocTree();
    LoadFavTree(theSln.Favs.GetRoot());
    LoadFavCombo();
	LoadBookTitles();
	setCursor(Qt::ArrowCursor);
}

void SlnPanel::LoadDocTree()
{
	DocItem* tposRoot = theSln.GetRoot();
	if (!tposRoot)
		return;
	
	ui.treeContents->clear();
	QTreeWidgetItem *root = new QTreeWidgetItem();
	root->setText(0, tposRoot->title[0]);
    root->setData(0, Qt::UserRole, QVariant::fromValue(tposRoot));

	ui.treeContents->addTopLevelItem(root);
    UpdateDocItem(root);

	LoadDocLevel(tposRoot, root);
	root->setExpanded(true);
}

void SlnPanel::LoadFavTree(FavItem* root)
{
    if (!root)
		return;

	ui.treeFavorites->clear();
    QTreeWidgetItem *qroot = new QTreeWidgetItem();
    qroot->setText(0, root->title);
    qroot->setData(0, Qt::UserRole, QVariant::fromValue(root));

    ui.treeFavorites->addTopLevelItem(qroot);
    UpdateFavItem(qroot);
	if (root->type == FavItem::T_REF && root->ref) 
		LoadDocLevel(root->ref, qroot);
	else 
		LoadFavLevel(root, qroot);
	qroot->setExpanded(true);
}

void SlnPanel::LoadFavLevel(FavItem* node, QTreeWidgetItem *parent)
{
	for (auto tpos : node->children)
	{
        QTreeWidgetItem *qitem = new QTreeWidgetItem(parent);
        qitem->setData(0, Qt::UserRole, QVariant::fromValue(tpos));
		if (tpos->type==FavItem::T_GROUP) {
            UpdateFavItem(qitem);
            LoadFavLevel(tpos, qitem);
		}
		else if(tpos->ref) {
            UpdateDocItem(qitem, tpos->ref);
            LoadDocLevel(tpos->ref, qitem);
		}
		else {
            UpdateFavItem(qitem);
		}
	}
}

void SlnPanel::LoadFavCombo()
{
    int sel = 0, i = 1;
    disconnect(ui.comboFavRoot, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SlnPanel::onFavRootChanged);
    FavItem *curr = ui.comboFavRoot->currentData().value<FavItem*>();
    FavItem *root = theSln.Favs.GetRoot();
    ui.comboFavRoot->clear();
    ui.comboFavRoot->addItem("ALL FAVORITES", QVariant::fromValue(root));
	if (root) {
		for (auto fav : root->children) {
			if (fav->ref)
				ui.comboFavRoot->addItem(fav->ref->GetTitle(0), QVariant::fromValue(fav));
			else
				ui.comboFavRoot->addItem(fav->title, QVariant::fromValue(fav));
			if (fav == curr)
				sel = i;
			i++;
		}
	}
        
	// reload 'ALL FAVs' if vis.tree is empty, or if we removed visible root
	if (ui.treeFavorites->topLevelItem(0) == nullptr || curr != ui.comboFavRoot->currentData().value<FavItem*>()) {
		ui.comboFavRoot->setCurrentIndex(0);
		onFavRootChanged(0);
	}
	// don't reload fav.tree, restore old selection
	else {
		ui.comboFavRoot->setCurrentIndex(sel);
	}

	connect(ui.comboFavRoot, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SlnPanel::onFavRootChanged);
}

void SlnPanel::ReloadFavCombo(FavItem *item)
{
    // for root and top-level items
    if(!item->parent || !item->parent->parent)
        LoadFavCombo();
}

void SlnPanel::onTabChanged(int tab)
{
    if(tab==0) {
        if(m_contentsNeedsToRefresh) {
            UpdateDocTree();
            m_contentsNeedsToRefresh = false;
        }
    }
    else if(tab==2) {
        if(m_favoritesNeedsToRefresh) {
            UpdateFavTree();
            m_favoritesNeedsToRefresh = false;
        }
    }
}

void SlnPanel::onDocContextMenu(const QPoint &pos)
{
	QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(sender());
	if (!treeWidget)
		return;
	QTreeWidgetItem *item = treeWidget->itemAt(pos);
	if (!item)
		return;

	menuPopupDoc->exec(treeWidget->viewport()->mapToGlobal(pos));
}

void SlnPanel::onFavContextMenu(const QPoint &pos)
{
//	QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(sender());
//	if (!treeWidget)
//		return;
	QTreeWidgetItem *item = ui.treeFavorites->itemAt(pos);
	if (!item)
		return;
	FavItem* fav = item->data(0, Qt::UserRole).value<FavItem*>();
	if (!fav)
		menuPopupDoc->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
    else if(!fav->parent)
        menuPopupAllFav->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
	else if(fav->type==FavItem::T_GROUP)
		menuPopupGroup->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
	else if(fav->ref)
		menuPopupRef->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
	else
		menuPopupDangling->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
}

void SlnPanel::onResDoubleClicked(QTreeWidgetItem* curItem, int column)
{
    // open document by double click
    onDocDoubleClicked(curItem, column);
    // search first occurence
    onFindNext();
}

void SlnPanel::onDocDoubleClicked(QTreeWidgetItem* qitem, int column)
{
	// open document by double click in Contents and SearchResults
	DocItem *item = qitem->data(0, Qt::UserRole).value<DocItem*>();
	mw->DoOpenDoc(item, column);
	UpdateDocItem(qitem);
}

void SlnPanel::onFavDoubleClicked(QTreeWidgetItem* curItem, int column)
{
    QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
    if (!qitem) return;
    FavItem* fav = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (fav && fav->ref) {
        mw->DoOpenDoc(fav->ref, column);
    }
    else {
        DocItem* doc = qitem->data(0, Qt::UserRole).value<DocItem*>();
        mw->DoOpenDoc(doc, column);
    }
}

void SlnPanel::onFavRootChanged(int index)
{
    FavItem *root = ui.comboFavRoot->currentData().value<FavItem*>();
    LoadFavTree(root);
}

QIcon& SlnPanel::GetTreeItemIcon(ETreeStatus i)
{
	if (i < ETreeStatus::TS_UNKNOWN || i >= ETreeStatus::TS_ITEMS_COUNT)
		i = ETreeStatus::TS_UNKNOWN;
	return m_TreeIcons[(int)i];
}

QIcon& SlnPanel::GetLangItemIcon(ELangStatus i)
{
	if (i < ELangStatus::LS_NONE || i >= ELangStatus::LS_ITEMS_COUNT)
		i = ELangStatus::LS_NONE;
	return m_LangIcons[(int)i];
}

void SlnPanel::RemoveItemDontAsk(bool del_files)
{
    // remove item from tree
    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

    item.doc->p_modify = 0;

    mw->CloseTabs(item.doc, true);
    if (theSln.RemoveNode(item.doc, del_files)) {
        RemoveTreeNode(item.qitem, nullptr);
        Update(nullptr, nullptr);
		LoadFavCombo(); // perhaps we removed local root in favs
        mw->projectModified(true);
		UpdateTreesIfNeeded(true);
    }
}

void SlnPanel::UpdateDocItem(QTreeWidgetItem * qitem)
{
    // set the text and picture of the view node by associated model node
    DocItem* tpos = qitem->data(0, Qt::UserRole).value<DocItem*>();
    UpdateDocItem(qitem, tpos);
}

void SlnPanel::UpdateDocItem(QTreeWidgetItem * qitem, DocItem* tpos)
{
    // set the text and picture of the view node by explicitly specified model node
    for(int i=0; i<theSln.Cols.BCnt(); i++) {
        qitem->setText(i, tpos->GetTitle(i));
        if(theSln.Cols.GetColType(i) == CT_BASE) {
            ETreeStatus im = tpos->GetTreeStatus();
            qitem->setIcon(0, GetTreeItemIcon(im));
        }
        else if(theSln.Cols.GetColType(i) == CT_BOOK) {
            ELangStatus ls = tpos->GetLangStatus(1);
            qitem->setIcon(1, GetLangItemIcon(ls));
        }
    }
}

void SlnPanel::UpdateFavItem(QTreeWidgetItem * item)
{
	// set the text and picture of the node depending on its state and attributes
	FavItem* tpos = item->data(0, Qt::UserRole).value<FavItem*>();

	if (tpos->type == FavItem::T_GROUP) {
		item->setText(0, tpos->title);
		item->setIcon(0, GetTreeItemIcon(ETreeStatus::TS_FOLDER));
	}
	else {
		if(!tpos->ref) {
			item->setText(0, tpos->title);
			item->setIcon(0, GetTreeItemIcon(ETreeStatus::TS_UNKNOWN));
		}
		else {
            UpdateDocItem(item, tpos->ref);
		}
	}
}

int SlnPanel::FindChildItemIndex(QTreeWidgetItem *qnode, DocItem* item, int startIndex)
{
    for(int i = startIndex, n = qnode->childCount(); i<n; i++) {
        QTreeWidgetItem *qchild = qnode->child(i);
        DocItem *doc = qchild->data(0, Qt::UserRole).value<DocItem*>();
        if(doc == item)
            return i;
    }
    return -1;
}

void SlnPanel::UpdateDocNode(QTreeWidgetItem * qnode, DocItem *node)
{
    // update text and icons
    UpdateDocItem(qnode, node);

    // loop by model children; the model is not modified!
    int i = 0;
    QTreeWidgetItem *qprevchild = nullptr;
    for(DocItem* item : node->children) {
        if(i < qnode->childCount()) {
            // get view item and associated model item
            QTreeWidgetItem *qchild = qnode->child(i);
            DocItem *adoc = qchild->data(0, Qt::UserRole).value<DocItem*>();
            // compare pair
            if(adoc == item) {
                // ok, recursive update
                UpdateDocNode(qchild, item);
            }
            else {
                // dismatch, find item next to node
                int j = FindChildItemIndex(qnode, item, i+1);
                // if found - move it to qchild position (exchange view nodes)
                if(j>=0) {
                    // j is always greater than i
                    QTreeWidgetItem *qchild2 = qnode->takeChild(j); // take item at higher (found) index
                    QTreeWidgetItem *qchild1 = qnode->takeChild(i); // take item at lower (current) index
                    qnode->insertChild(i, qchild2);                 // insert found item at lower index
                    qnode->insertChild(j, qchild1);                 // insert this item at higher index
                    // recursive update
                    qchild = qchild2;
                    UpdateDocNode(qchild, item);
                }
                // else create and load from model; because load, no recursive update required
                else {
                    QTreeWidgetItem *qitem = new QTreeWidgetItem(qnode, qprevchild);
                    qitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
                    UpdateDocItem(qitem);
                    LoadDocLevel(item, qitem);
                }
            }
            // prev view item
            qprevchild = qchild;
        }
        else {
            // add new node from model; because load, no recursive update required
            QTreeWidgetItem *qitem = new QTreeWidgetItem(qnode, qprevchild);
            qitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
            UpdateDocItem(qitem);
            LoadDocLevel(item, qitem);
            // prev view item
            qprevchild = qitem;
        }

        // next view item index
        i++;        
    }
    // remove the remaining items in the view
    int n = qnode->childCount();
    for(int j=i; i<n; i++) {
        delete qnode->takeChild(j);
    }
}

void SlnPanel::UpdateDocTree()
{
    UpdateDocNode(ui.treeContents->topLevelItem(0), theSln.GetRoot());
}

void SlnPanel::UpdateFavTree()
{
    QTreeWidgetItem *qroot = ui.treeFavorites->topLevelItem(0);
    if(!qroot)
        return;
    ForEachItem(qroot, [&](QTreeWidgetItem * qitem) {
        FavItem* fav = qitem->data(0, Qt::UserRole).value<FavItem*>();
        if(fav->type == FavItem::T_REF) {
            // check ref validity
            DocItem *doc = theSln.Locate(fav->title);
            if(!doc)
                fav->ref = nullptr;
            else
                UpdateDocNode(qitem, fav->ref);
            // don't deepen recursively
            return true;
        }
        return false;
    });    
}

void SlnPanel::UpdateDocItemsByObj(DocItem* pos)
{
    // for update language status from 'SaveHtml' function
    // single object in Contents
    QTreeWidgetItem *item = FindItem(ui.treeContents->topLevelItem(0), pos);
    if (item)
       UpdateDocItem(item);

    //ForEachItem(ui.treeContents->topLevelItem(0), [&](QTreeWidgetItem *item){
    //    DocItem* doc = item->data(0, Qt::UserRole).value<DocItem*>();
    //    if(doc == pos)
    //        UpdateDocItem(item);
    //});

    // multiple objects in Favorites
    ForEachItem(ui.treeFavorites->topLevelItem(0), [&](QTreeWidgetItem *item){
        DocItem* doc = item->data(0, Qt::UserRole).value<DocItem*>();
        if(doc == pos)
            UpdateDocItem(item);
        FavItem* fav = item->data(0, Qt::UserRole).value<FavItem*>();
        if(fav && fav->ref == pos)
            UpdateDocItem(item, pos);
        return false;
    });
}

void SlnPanel::SetCurrItemStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, false);
    UpdateDocItem(item);

	UpdateTreesIfNeeded(false);
}

void SlnPanel::SetCurrNodeStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, true);
	UpdateTreesIfNeeded(true);
}

void SlnPanel::UpdateTreesIfNeeded(bool all)
{
	// update progress
	int ci = theSln.Cols.ProgressCol();
	if (ci >= 0) {
		theSln.GetRoot()->UpdateProgress(ci);
		all = true;
	}
	if(all) {
		UpdateDocTree();
		UpdateFavTree();
	}
}


void SlnPanel::Update(QTreeWidgetItem *qitem, DocItem *doc)
{
    // qitem: view item to update
    //   doc: model item to update
    int tab = ui.tabWidget->currentIndex();
    if(tab==0) {
        if(qitem)
            UpdateDocItem(qitem);
        m_favoritesNeedsToRefresh = true;
    }
    else if(tab==2) {
        UpdateFavTree();
        m_contentsNeedsToRefresh = true;
    }
    if(doc)
        mw->UpdateTab(doc);
    if(qitem)
        SetCurrentItem(qitem);
}

void SlnPanel::onItemProperties()
{
	// rename item, document file and tree file
	TREEITEM item = CurrItem();
	if (item.badDoc()) return;

	ItemProperties dlg(this);
	if (dlg.DoModal(item.doc) == QDialog::Accepted)	{
        bool changed = false;
        if(item.doc->GetTitle(0) != dlg.m_title0) {
            theSln.RenameTitle(item.doc, dlg.m_title0, 0);
            changed = true;
        }
        if(item.doc->GetTitle(1) != dlg.m_title1) {
            theSln.RenameTitle(item.doc, dlg.m_title1, 1);
            changed = true;
        }
		if (item.doc->GetId() != dlg.m_id) {
            if (!theSln.RenameItem(item.doc, dlg.m_id))
                QMessageBox::warning(this, "Rename error", FailMsg);
		}
        if(changed) {
            Update(item.qitem, item.doc);
        }
	}
}

void SlnPanel::onOpenInNewTab(int di)
{
	TREEITEM item = CurrItem();
	if (item.badDoc()) return;
	mw->DoOpenDoc(item.doc, di);
}

void SlnPanel::onOpenInExtBrowser(int di)
{
	OpenInExtProgram(U16(INI::BrowserPath), di);
}

void SlnPanel::onOpenInExtDocEditor(int di)
{
	OpenInExtProgram(U16(INI::VisEditPath), di);
}

void SlnPanel::onOpenInExtTextEditor(int di)
{
	OpenInExtProgram(U16(INI::HtmEditPath), di);
}

void SlnPanel::onDeleteDoc()
{
	onDeleteDoc(-1);
}

void SlnPanel::onDeleteDoc(int di)
{
	int ret = QMessageBox::warning(this, AppTitle, tr("Remove doc? Source file will be untouched."),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes) {
		TREEITEM item = CurrItem();
		if (item.badDoc()) return;
		if (di < 0)
			di = ui.treeContents->currentColumn();
		theSln.RemoveNodeDoc(item.doc, di);
        UpdateDocItem(item.qitem);
		mw->projectModified(true);
	}
}

void SlnPanel::onOpenVmbaseInExtTextEditor()
{
	TREEITEM item = CurrItem();
	if (item.badDoc()) return;

	QString path;
	path = item.doc->GetVmbAbsPath();
	if (!QFileInfo(path).isFile())
		QMessageBox::warning(this, AppTitle, tr("VMBase file not found"), QMessageBox::Ok);
	else
		OpenInExternalApplication(this, codecUtf8->toUnicode(INI::HtmEditPath.c_str()), path);
}

void SlnPanel::onOpenFolderVmb()
{
	onOpenFolder(-1);
}

void SlnPanel::onOpenFolder(int bi)
{
	if (INI::ExplorePath.empty()) {
		QMessageBox::warning(this, "Error", "ExplorePath is not set");
		return;
	}

    TREEITEM item = CurrItem();
    if (item.badDoc()) return;

    QString path = item.doc->GetAbsDir(bi);
	path += "/";
	path = QDir::toNativeSeparators(path);
	if (QFileInfo(path).exists())
		OpenInExternalApplication(this, U16(INI::ExplorePath), path);
	else
		QMessageBox::warning(this, "Warning", "Path is not exist\r\n" + path);
}

void SlnPanel::OpenInExtProgram(const QString& program, int di)
{
	TREEITEM item = CurrItem();
	if (item.badDoc()) return;

	QString path;
	path = item.doc->GetDocAbsPath(di);
	if (!QFileInfo(path).isFile())
		QMessageBox::warning(this, AppTitle, tr("Document %1 not found").arg(path), QMessageBox::Ok);
	else
		OpenInExternalApplication(this, program, path);
}

//////////////////////////////////////////////////////////////////////////
// insert & delete

void SlnPanel::onAddChildDoc()
{
	// insert the blank into the tree
	NewItemDlg dlg(m_TreeIcons);
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");
    dlg.m_open = INI::OpenNewDoc;
	dlg.m_status = INI::DefItemStatus;

    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

	// prepare workpiece
    QTreeWidgetItem *nqitem = AddTreeItem(item.qitem, nullptr, dlg.m_title, GetTreeItemIcon(ETreeStatus::TS_UNREADY));

	if (dlg.DoModal() == QDialog::Accepted) {
		INI::OpenNewDoc = dlg.m_open;
		INI::DefItemStatus = dlg.m_status;

		// ok - insert the element into the base and connect the workpiece
        DocItem* tpNew = theSln.AddItem(item.doc, nullptr, dlg.m_title, dlg.m_id);
		if (tpNew) {
            nqitem->setData(0, Qt::UserRole, QVariant::fromValue(tpNew));
            Update(nqitem, nullptr);
            if (dlg.m_open)
				mw->DoOpenDoc(tpNew, 0);
			UpdateTreesIfNeeded(true);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating node"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	// cancel - remove the workpiece
    RemoveTreeNode(nqitem, item.qitem);
}

void SlnPanel::onAddSiblingDoc()
{
	// insert a new item after the given one
	NewItemDlg dlg(m_TreeIcons);
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");
    dlg.m_open = INI::OpenNewDoc;
	dlg.m_status = INI::DefItemStatus;

    TREEITEM item = CurrItem();
    if(item.badDoc() || !item.doc->parent || item.fav) return;

	// prepare workpiece
    QTreeWidgetItem *nqitem = AddTreeItem(item.qitem->parent(), item.qitem, dlg.m_title, GetTreeItemIcon(ETreeStatus::TS_UNREADY));
		
	if (dlg.DoModal() == QDialog::Accepted) {
		INI::OpenNewDoc = dlg.m_open;
		INI::DefItemStatus = dlg.m_status;

		// ok - insert the element into the base and connect the workpiece
        DocItem* tpNew = theSln.AddItem(item.doc->parent, item.doc, dlg.m_title, dlg.m_id);
		if (tpNew) {
            nqitem->setData(0, Qt::UserRole, QVariant::fromValue(tpNew));
            Update(nqitem, nullptr);
            if(dlg.m_open)
				mw->DoOpenDoc(tpNew, 0);
			UpdateTreesIfNeeded(true);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating node"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	// cancel - remove the workpiece
    RemoveTreeNode(nqitem, item.qitem);
}

void SlnPanel::onRemoveItem()
{
	// remove item from tree without deleting files
	int ret = QMessageBox::warning(this, AppTitle, tr("Remove node? Source file will be untouched."),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes)
	{
		RemoveItemDontAsk(false);
	}
}

void SlnPanel::onDeleteItem()
{
	// completely remove the element
	int ret = QMessageBox::warning(this, AppTitle, tr("Delete node? All source files will be deleted!"),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes)
	{
		RemoveItemDontAsk(true);
	}
}

void SlnPanel::onMoveItemUp()
{
    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

    if (theSln.MoveUp(item.doc)) {
		// move node up: move PREVIOUS DOWN
        QTreeWidgetItem *prev = GetPrevSibling(item.qitem);
        QTreeWidgetItem *parent = item.qitem->parent();
        MoveItem(prev, parent, item.qitem);
        Update(item.qitem, item.doc);
	}
}

void SlnPanel::onMoveItemDown()
{
    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

    if (theSln.MoveDown(item.doc)) {
		// move node down: move THIS DOWN
        QTreeWidgetItem *next = GetNextSibling(item.qitem);
        QTreeWidgetItem *parent = item.qitem->parent();
        MoveItem(item.qitem, parent, next);
        Update(item.qitem, item.doc);
	}
}

void SlnPanel::onMoveItemParent()
{
	// move node left: make it next after parent
    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

    if (theSln.MoveParent(item.doc))
	{
        QTreeWidgetItem *parent = item.qitem->parent();
        QTreeWidgetItem *parpar = parent->parent();
        MoveItem(item.qitem, parpar, parent);
        Update(item.qitem, item.doc);
		UpdateTreesIfNeeded(true);
	}
}

void SlnPanel::onMoveItemChild()
{
	// move node to the right: make it the last child of its previous one
    TREEITEM item = CurrItem();
    if(item.badDoc()) return;

    if (theSln.MoveChild(item.doc))
	{
        QTreeWidgetItem *prev = GetPrevSibling(item.qitem);
        MoveItem(item.qitem, prev, 0);
        Update(item.qitem, item.doc);
		UpdateTreesIfNeeded(true);
	}
}

void SlnPanel::onMoveItem()
{
    TREEITEM item = CurrItem();
    if (item.badDoc()) return;

    TopicChooser dlg(this, "Select new parent item");
    if (dlg.DoModal()) {
        if (!theSln.Move(item.doc, dlg.m_posSelected, NULL)) {
            QMessageBox::warning(this, "Move error", FailMsg, QMessageBox::Ok);
        }
        else {
            QTreeWidgetItem *qnpar = FindItem(ui.treeContents->topLevelItem(0), dlg.m_posSelected);
            if (qnpar) {
                MoveItem(item.qitem, qnpar, 0);
                SetCurrentItem(item.qitem);
            }
            Update(item.qitem, item.doc);
			UpdateTreesIfNeeded(true);
        }
    }
}

int SlnPanel::onDropping(QTreeWidgetItem *drag, QTreeWidgetItem *drop, int m)
{
	// drag event
	// Qt::KeyboardModifiers
	// return Qt::IgnoreAction or Qt::MoveAction
	if (!drag || !drop)
		return 0;
	DocItem* tpDrag = drag->data(0, Qt::UserRole).value<DocItem*>();
	DocItem* tpDrop = drop->data(0, Qt::UserRole).value<DocItem*>();

	int r = QMessageBox::warning(this, AppTitle,
		tr("Are you sure want to move node '%1' to node '%2'").arg(tpDrag->GetTitle(0)).arg(tpDrop->GetTitle(0)),
		QMessageBox::Yes | QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		if (theSln.Move(tpDrag, tpDrop, NULL))
			return 1;
	}

	return 0;
}

void SlnPanel::ForEachItem(QTreeWidgetItem *par, const std::function<bool(QTreeWidgetItem *)> fn)
{
    if(fn(par))
        return; // don't deepen recursively
    int n = par->childCount();
    for (int i = 0; i < n; i++) {
        ForEachItem(par->child(i), fn);
    }
}

QTreeWidgetItem* SlnPanel::FindItem(QTreeWidgetItem *par, DocItem* mtpos)
{
	// recursive search for an element with a given identifier
	// checking this item
	DocItem* pos = par->data(0, Qt::UserRole).value<DocItem*>();
	if (pos == mtpos)
		return par;

	// recursive traversal of the rest
	int n = par->childCount();
    for (int i = 0; i < n; i++) {
		QTreeWidgetItem* found = FindItem(par->child(i), mtpos);
		if (found)
			return found;
	}
	return 0;
}

void SlnPanel::EnsureVisible(DocItem* node)
{
	QTreeWidgetItem *item = FindItem(ui.treeContents->topLevelItem(0), node);
	if (item)
	{
        ui.tabWidget->setCurrentIndex(0);
		ui.treeContents->scrollToItem(item);
		ui.treeContents->setCurrentItem(item);
	}
}

void SlnPanel::Search(const QString &text)
{
    ui.tabWidget->setCurrentIndex(1);
    ui.lineSearch->setText(text);
    onSearch();
}

void SlnPanel::onSearch()
{
    // searching...
	std::list<DocItem*> results;
	setCursor(QCursor(Qt::WaitCursor));
    QString text = ui.lineSearch->text().toHtmlEscaped();
	unsigned int scope = 0;
	if (actionCheckTree->isChecked())
		scope |= ESM_TREE;
	if (actionCheckIds->isChecked())
		scope |= ESM_ID;
	if (actionCheckText->isChecked())
		scope |= ESM_TEXT;
	if (actionCheckTags->isChecked())
		scope |= ESM_TAG;
	if (actionCheckAttrs->isChecked())
		scope |= ESM_ATTR;
	if (!scope) {
		QMessageBox::warning(this, "Search", "Search scope not defined!");
		return;
	}
	DocItem *root = theSln.Locate(searchRoot);
	if (!root)
		root = theSln.GetRoot();
    theSln.Search(text, scope, root, results);
    ui.treeResults->clear();
    for(auto pos : results) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setData(0, Qt::UserRole, QVariant::fromValue(pos));
        ui.treeResults->addTopLevelItem(item);
        UpdateDocItem(item);
    }
	setCursor(QCursor(Qt::ArrowCursor));
}

void SlnPanel::onFindNext()
{
	WebEditView *wnd = mw->GetActiveMdiChild();
	if (wnd)
		wnd->Find(ui.lineSearch->text(), false);
}

void SlnPanel::onFindPrev()
{
	WebEditView *wnd = mw->GetActiveMdiChild();
	if (wnd)
		wnd->Find(ui.lineSearch->text(), true);
}

void SlnPanel::onSelNode()
{
	TopicChooser dlg(this, "Select node to search");
	if (!dlg.DoModal()) {
		searchRoot = "";
		ui.lineNode->setText("");
		return;
	}
	searchRoot = dlg.m_posSelected->GetGuid();
	if (dlg.m_posSelected)
		ui.lineNode->setText(dlg.m_posSelected->GetTitles(0));
	else
		ui.lineNode->setText("");
}

void SlnPanel::onAddToFavorites()
{
	TREEITEM item = CurrItem();
	if (item.badDoc()) return;
	
    FavItem *root = ui.comboFavRoot->currentData().value<FavItem*>();
    if(!root) return;
    FavItem* fav = theSln.Favs.AddRef(root, nullptr, item.doc);
    if (fav) {
        QTreeWidgetItem *qpar = ui.treeFavorites->topLevelItem(0);
        QTreeWidgetItem *qnewitem = AddTreeItem(qpar, nullptr, "", GetTreeItemIcon(ETreeStatus::TS_UNREADY));
        qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(fav));
        UpdateFavItem(qnewitem);
        LoadDocLevel(fav->ref, qnewitem);
        ReloadFavCombo(fav);
        return; // ok
    }
    else {
        QMessageBox::warning(this, AppTitle, tr("Error adding reference"), QMessageBox::Ok, QMessageBox::Ok);
    }
}

void SlnPanel::onRemoveFromFavorites()
{
    QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
    if (!qitem) return;
    FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (!sel) return;
    bool rc = !sel->parent || !sel->parent->parent; // local root or first-level item
	int ret = QMessageBox::question(this, AppTitle, tr("Remove favorite reference? Document will be untouched."),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes) {
        if(theSln.Favs.RemoveNode(sel)) {
            RemoveTreeNode(qitem, nullptr);
            if(rc) {
                LoadFavCombo();
            }
        }
	}
}

void SlnPanel::onEditFavoriteRef()
{
    QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
    if (!qitem) return;
    FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (!sel) return;
	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {
        theSln.Favs.ChangeRef(sel, dlg.m_posSelected);
        UpdateFavItem(qitem);
        LoadDocLevel(sel->ref, qitem);
        ReloadFavCombo(sel);
	}
}

void SlnPanel::onAddSiblingGroup()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
	if (!sel) return;
	QTreeWidgetItem *qpar = qitem->parent();
	if (!qpar) return;
	FavItem* par = qpar->data(0, Qt::UserRole).value<FavItem*>();
	if (!par) return;

	// prepare workpiece
    QTreeWidgetItem *qnewitem = AddTreeItem(qpar, qitem, "NEW GROUP", GetTreeItemIcon(ETreeStatus::TS_FOLDER));

	QString s = QInputDialog::getText(this, "Add group", "Input group name", QLineEdit::Normal, "NEW GROUP");
	if (!s.isEmpty()) {
		// ok - insert folder
		FavItem* item = theSln.Favs.AddGroup(par, sel, s);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
            UpdateFavItem(qnewitem);
            ReloadFavCombo(item);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating group"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
        RemoveTreeNode(qnewitem, qitem);
	}
}

void SlnPanel::onAddChildGroup()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
	if (!sel) return;

	// prepare workpiece
    QTreeWidgetItem *qnewitem = AddTreeItem(qitem, nullptr, "NEW GROUP", GetTreeItemIcon(ETreeStatus::TS_FOLDER));

	QString s = QInputDialog::getText(this, "Add group", "Input group name", QLineEdit::Normal, "NEW GROUP");
	if (!s.isEmpty()) {
		// ok - insert folder
		FavItem* item = theSln.Favs.AddGroup(sel, nullptr, s);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
            UpdateFavItem(qnewitem);
            ReloadFavCombo(item);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating group"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
        RemoveTreeNode(qnewitem, qitem);
	}
}

void SlnPanel::onAddSiblingFav()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (!sel || sel->type!=FavItem::T_GROUP) return;
	QTreeWidgetItem *qpar = qitem->parent();
	if (!qpar) return;
	FavItem* par = qpar->data(0, Qt::UserRole).value<FavItem*>();
	if (!par) return;

	// prepare workpiece
    QTreeWidgetItem *qnewitem = AddTreeItem(qpar, qitem, "NEW REF", GetTreeItemIcon(ETreeStatus::TS_UNREADY));

	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {
		// ok - insert subtree
		FavItem* item = theSln.Favs.AddRef(par, sel, dlg.m_posSelected);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
            UpdateFavItem(qnewitem);
			LoadDocLevel(item->ref, qnewitem);
            ReloadFavCombo(item);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error adding reference"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
        RemoveTreeNode(qnewitem, qitem);
	}
}

void SlnPanel::onAddChildFav()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (!sel || sel->type!=FavItem::T_GROUP) return;

	// prepare workpiece
    QTreeWidgetItem *qnewitem = AddTreeItem(qitem, nullptr, "NEW REF", GetTreeItemIcon(ETreeStatus::TS_UNREADY));
	
	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {
		// ok - insert subtree
		FavItem* item = theSln.Favs.AddRef(sel, nullptr, dlg.m_posSelected);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
            UpdateFavItem(qnewitem);
			LoadDocLevel(item->ref, qnewitem);
            qnewitem->setSelected(true);
            ReloadFavCombo(item);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error adding reference"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
        RemoveTreeNode(qnewitem, qitem);
	}
}

void SlnPanel::onEditGroup()
{
    QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
    if (!qitem) return;
    FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
    if (!sel) return;
    QString s = QInputDialog::getText(this, "Rename group", "Change group name", QLineEdit::Normal, sel->title);
	if (!s.isEmpty()) {
        theSln.Favs.ChangeTitle(sel, s);
        UpdateFavItem(qitem);
        qitem->setSelected(true);
        ReloadFavCombo(sel);    // this causes the entire list to be reloaded, and therefore the tree! and qitem becomes invalid
	}
}
