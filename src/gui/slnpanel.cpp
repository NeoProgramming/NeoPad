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

	ui.treeContents->setRootIsDecorated(false);
	ui.treeContents->setUniformRowHeights(true);			// all lines are the same height
	ui.treeContents->header()->setStretchLastSection(false);// the last section is NOT the full available width
	ui.treeContents->header()->setSectionResizeMode(QHeaderView::ResizeToContents);	// header resizing mode

	QString fmt;
	fmt.sprintf("background-color: #%06X; alternate-background-color: #%06X", INI::BackColor1, INI::BackColor2);
	ui.treeContents->setStyleSheet(fmt);//"background-color: #FFC782; alternate-background-color: #F0CF72");

	ui.treeContents->setColumnCount(2);
	QTreeWidgetItem* headerItem = ui.treeContents->headerItem();
	headerItem->setText(0, "Base 1");
	headerItem->setText(1, "Base 2");
	
	initColumns(ui.treeContents);
	initColumns(ui.treeFavorites);

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
	actionCheckTree = menu->addAction("Search in Tree");
	actionCheckText = menu->addAction("Search in Text");
	actionCheckTags = menu->addAction("Search in HTML Tags");
	actionCheckAttrs= menu->addAction("Search in HTML Attributes");
//	menu->addSeparator();
//	menu->addAction("Match case");
//	menu->addAction("Whole words");

	actionCheckTree->setCheckable(true);
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

	//!+! shortcuts does not trigger action, see eventFilter()
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

void SlnPanel::initColumns(QTreeWidget *tree)
{
	QHeaderView * header = tree->header();
	header->setSectionResizeMode(QHeaderView::Interactive);
	tree->setColumnWidth(0, 220);
	tree->setColumnWidth(1, 220);
	tree->setIconSize(QSize(20, 20));
}

DocItem* SlnPanel::currDoc()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item)
		return nullptr;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	return tpos;
}

FavItem* SlnPanel::currFav()
{
	QTreeWidgetItem *item = ui.treeFavorites->currentItem();
	if (!item)
		return nullptr;
	FavItem* tpos = item->data(0, Qt::UserRole).value<FavItem*>();
	return tpos;
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

void SlnPanel::showInitDoneMessage()
{
	if (initDoneMsgShown)
		return;
	initDoneMsgShown = true;
	mw->statusBar()->showMessage(tr("Done"), 3000);
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
			case Qt::Key_F2:
				ui.treeContents->openPersistentEditor(ui.treeContents->currentItem(), 0);
				break;
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
				if (modifiers == Qt::NoModifier)
					onAddChildGroup();
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
				if (modifiers == Qt::NoModifier)
					onAddSiblingGroup();
				else if (modifiers == Qt::ShiftModifier)
					onAddSiblingFav();
				break;
			}
		}
	}
	return QWidget::eventFilter(o, e);
}

void SlnPanel::UpdateBookTitles()
{
	QTreeWidgetItem* headerItem = ui.treeContents->headerItem();
	headerItem->setText(0, theSln.Books.books[0].title);
	headerItem->setText(1, theSln.Books.books[1].title);

    headerItem = ui.treeFavorites->headerItem();
    headerItem->setText(0, theSln.Books.books[0].title);
    headerItem->setText(1, theSln.Books.books[1].title);

    submenuOpen0Ext->setTitle(tr("Doc0 (%1)").arg(theSln.Books.books[0].title));
    submenuOpen1Ext->setTitle(tr("Doc1 (%1)").arg(theSln.Books.books[1].title));
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
	LoadFavTree();
	UpdateBookTitles();
	setCursor(Qt::ArrowCursor);
	showInitDoneMessage();
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

void SlnPanel::LoadFavTree()
{
	FavItem* tposRoot = theSln.Favs.GetRoot();
	if (!tposRoot)
		return;

	ui.treeFavorites->clear();
	QTreeWidgetItem *root = new QTreeWidgetItem();
	root->setText(0, tposRoot->title);
	root->setData(0, Qt::UserRole, QVariant::fromValue(tposRoot));

	ui.treeFavorites->addTopLevelItem(root);
	UpdateFavItem(root);

	LoadFavLevel(tposRoot, root);
	root->setExpanded(true);
}

void SlnPanel::LoadFavLevel(FavItem* node, QTreeWidgetItem *parent)
{
	for (auto tpos : node->children)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(parent);
		item->setData(0, Qt::UserRole, QVariant::fromValue(tpos));
		if (tpos->type==FavItem::T_GROUP) {
			UpdateFavItem(item);
			LoadFavLevel(tpos, item);
		}
		else if(tpos->ref) {
			UpdateDocItem(tpos->ref);
			LoadDocLevel(tpos->ref, item);
		}
		else {
			UpdateFavItem(item);
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
	else if(fav->type==FavItem::T_GROUP)
		menuPopupGroup->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
	else if(fav->ref)
		menuPopupRef->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
	else
		menuPopupDangling->exec(ui.treeFavorites->viewport()->mapToGlobal(pos));
}

void SlnPanel::RemoveItemDontAsk(bool del_files)
{
	// remove item from tree
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (theSln.RemoveNode(tpos, del_files))
	{
		QTreeWidgetItem *parent = item->parent();
		int index;
		if (parent)
		{
			index = parent->indexOfChild(item);
			delete parent->takeChild(index);
		}
		else
		{
			index = ui.treeContents->indexOfTopLevelItem(item);
			delete ui.treeContents->takeTopLevelItem(index);
		}
		mw->projectModified(true);
	}
}

void SlnPanel::onResDoubleClicked(QTreeWidgetItem* curItem, int column)
{
    // open document by double click
    onDocDoubleClicked(curItem, column);
    // search first occurence
    onFindNext();
}

void SlnPanel::onDocDoubleClicked(QTreeWidgetItem* curItem, int column)
{
	// open document by double click
	DocItem *item = currDoc();
	mw->DoOpenDoc(item, column);
}

void SlnPanel::onFavDoubleClicked(QTreeWidgetItem* curItem, int column)
{
	FavItem *item = currFav();
	if(item && item->ref)
		mw->DoOpenDoc(item->ref, column);
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


void SlnPanel::UpdateDocItem(QTreeWidgetItem * item)
{
	// set the text and picture of the node depending on its state and attributes
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	UpdateDocItem(item, tpos);
}

void SlnPanel::UpdateDocItem(QTreeWidgetItem * item, DocItem* tpos)
{
	item->setText(0, tpos->GetTitle(0));
	ETreeStatus im = tpos->GetTreeStatus();
	item->setIcon(0, GetTreeItemIcon(im));

	item->setText(1, tpos->GetTitle(1));
	ELangStatus ls = tpos->GetLangStatus(1);
	item->setIcon(1, GetLangItemIcon(ls));
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

void SlnPanel::UpdateDocNode(QTreeWidgetItem * node, DocItem *inode)
{

}

void SlnPanel::UpdateNode(QTreeWidgetItem * item)
{
	// recursively update all pictures of the whole tree
	UpdateDocItem(item);
	int n = item->childCount();
	for (int i = 0; i < n; i++)
	{
		QTreeWidgetItem *child = item->child(i);
		UpdateNode(child);
	}
}

void SlnPanel::UpdateDocItem(DocItem* pos)
{
	QTreeWidgetItem *item = FindItem(ui.treeContents->topLevelItem(0), pos);
	if (item)
		UpdateDocItem(item);
}

void SlnPanel::UpdateTree()
{
	UpdateNode(ui.treeContents->topLevelItem(0));
}

void SlnPanel::SetCurrItemStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, false);
	UpdateDocItem(item);
}

void SlnPanel::SetCurrNodeStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, true);
	UpdateNode(item);
}

void SlnPanel::onItemProperties()
{
	// rename item, document file and tree file
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	ItemProperties dlg(this);
	if (dlg.DoModal(tpos) == QDialog::Accepted)	{
		theSln.RenameTitle(tpos, dlg.m_title0, 0);
		theSln.RenameTitle(tpos, dlg.m_title1, 1);
		if (tpos->GetId() != dlg.m_id) {
			if (!theSln.RenameItem(tpos, dlg.m_id))
				QMessageBox::warning(this, "Rename error", FailMsg);
		}
		UpdateDocItem(item);
		mw->UpdateTab(tpos);
	}
}

void SlnPanel::onOpenInNewTab(int di)
{
	DocItem *item = currDoc();
	mw->DoOpenDoc(item, di);
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
		QTreeWidgetItem *item = ui.treeContents->currentItem();
		if (di < 0)
			di = ui.treeContents->currentColumn();
		DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
		theSln.RemoveNodeDoc(tpos, di);
		UpdateDocItem(item);
		mw->projectModified(true);
	}
}

void SlnPanel::onOpenVmbaseInExtTextEditor()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	QString path;
	path = tpos->GetVmbAbsPath();
	if (!QFileInfo(path).isFile())
		QMessageBox::warning(this, AppTitle, tr("VMBase file not found"), QMessageBox::Ok);
	else
		OpenInExternalApplication(this, codecUtf8->toUnicode(INI::HtmEditPath.c_str()), path);
}

void SlnPanel::onMoveItem()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpDrag = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpDrag) return;

	TopicChooser dlg(this, "Select new parent item");
	if (dlg.DoModal()) {
		if (!theSln.Move(tpDrag, dlg.m_posSelected, NULL)) {
			QMessageBox::warning(this, "Move error", FailMsg, QMessageBox::Ok);
		}
		else {
			QTreeWidgetItem *npar = FindItem(ui.treeContents->topLevelItem(0), dlg.m_posSelected);
			if (npar) {
				ui.treeContents->MoveItem(item, npar, 0);
				ui.treeContents->setCurrentItem(item);
			}
		}
	}
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

	DocItem* tpos = currDoc();
	if (!tpos) return;

	QString path = tpos->GetAbsDir(bi);
	path += "/";
	path = QDir::toNativeSeparators(path);
	if (QFileInfo(path).exists())
		OpenInExternalApplication(this, U16(INI::ExplorePath), path);
	else
		QMessageBox::warning(this, "Warning", "Path is not exist\r\n" + path);
}

void SlnPanel::OpenInExtProgram(const QString& program, int di)
{
	DocItem* tpos = currDoc();
	if (!tpos) return;

	QString path;
	path = tpos->GetDocAbsPath(di);
	if (!QFileInfo(path).isFile())
		QMessageBox::warning(this, AppTitle, tr("Document %1 not found").arg(path), QMessageBox::Ok);
	else
		OpenInExternalApplication(this, program, path);
}

//////////////////////////////////////////////////////////////////////////
// insert & delete
QTreeWidgetItem *SlnPanel::AddWorkpieceItem(QTreeWidgetItem *par, QTreeWidgetItem *after, const QString &title, ETreeStatus st)
{
	QTreeWidgetItem *newitem = after ? new QTreeWidgetItem(par, after) : new QTreeWidgetItem(par);
	newitem->setText(0, title);
	newitem->setIcon(0, GetTreeItemIcon(st));
	par->setExpanded(true);
	QTreeWidget *tree = par->treeWidget();
	tree->setCurrentItem(newitem);
	return newitem;
}

void SlnPanel::RemoveWorkpieceItem(QTreeWidgetItem *newitem, QTreeWidgetItem *olditem)
{
	QTreeWidget *tree = olditem->treeWidget();
	tree->setCurrentItem(olditem);
	QTreeWidgetItem *par = newitem->parent();
	int index = par->indexOfChild(newitem);
	delete par->takeChild(index);
}

void SlnPanel::onAddChildDoc()
{
	// insert the blank into the tree
	NewItemDlg dlg;
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");
    dlg.m_open = INI::OpenNewDoc;

	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpPar = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpPar) return;

	// prepare workpiece
	QTreeWidgetItem *newitem = AddWorkpieceItem(item, nullptr, dlg.m_title, ETreeStatus::TS_UNREADY);

	if (dlg.DoModal() == QDialog::Accepted) {
		// ok - insert the element into the base and connect the workpiece
		DocItem* tpNew = theSln.AddItem(tpPar, nullptr, dlg.m_title, dlg.m_id);
		if (tpNew) {
			newitem->setData(0, Qt::UserRole, QVariant::fromValue(tpNew));
			UpdateDocItem(newitem);
			if (dlg.m_open) {
				mw->DoOpenDoc(tpNew, 0);
			}
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating node"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	// cancel - remove the workpiece
	RemoveWorkpieceItem(newitem, item);
}

void SlnPanel::onAddSiblingDoc()
{
	// insert a new item after the given one
	NewItemDlg dlg;
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");
    dlg.m_open = INI::OpenNewDoc;

	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpAfter = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpAfter) return;
	QTreeWidgetItem *par = item->parent();
	if (!par) return;
	DocItem* tpPar = par->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpPar) return;

	// prepare workpiece
	QTreeWidgetItem *newitem = AddWorkpieceItem(par, item, dlg.m_title, ETreeStatus::TS_UNREADY);
		
	if (dlg.DoModal() == QDialog::Accepted) {
		// ok - insert the element into the base and connect the workpiece
		DocItem* tpNew = theSln.AddItem(tpPar, tpAfter, dlg.m_title, dlg.m_id);
		if (tpNew) {
			newitem->setData(0, Qt::UserRole, QVariant::fromValue(tpNew));
			UpdateDocItem(newitem);
			ui.treeContents->setCurrentItem(newitem);
			if(dlg.m_open) {
				mw->DoOpenDoc(tpNew, 0);
			}
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating node"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	// cancel - remove the workpiece
	RemoveWorkpieceItem(newitem, item);
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
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	if (theSln.MoveUp(tpos))
	{
		// move node up: move PREVIOUS DOWN
		QTreeWidgetItem *prev = ui.treeContents->GetPrevSibling(item);
		QTreeWidgetItem *parent = item->parent();
		ui.treeContents->MoveItem(prev, parent, item);
		ui.treeContents->setCurrentItem(item);
	}
}

void SlnPanel::onMoveItemDown()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	if (theSln.MoveDown(tpos))
	{
		// move node down: move THIS DOWN
		QTreeWidgetItem *next = ui.treeContents->GetNextSibling(item);
		QTreeWidgetItem *parent = item->parent();
		ui.treeContents->MoveItem(item, parent, next);
		ui.treeContents->setCurrentItem(item);
	}
}

void SlnPanel::onMoveItemParent()
{
	// move node left: make it next after parent
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	if (theSln.MoveParent(tpos))
	{
		QTreeWidgetItem *parent = item->parent();
		QTreeWidgetItem *parpar = parent->parent();
		ui.treeContents->MoveItem(item, parpar, parent);
		ui.treeContents->setCurrentItem(item);
	}
}

void SlnPanel::onMoveItemChild()
{
	// move node to the right: make it the last child of its previous one
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	DocItem* tpos = item->data(0, Qt::UserRole).value<DocItem*>();
	if (!tpos) return;

	if (theSln.MoveChild(tpos))
	{
		QTreeWidgetItem *prev = ui.treeContents->GetPrevSibling(item);
		ui.treeContents->MoveItem(item, prev, 0);
		ui.treeContents->setCurrentItem(item);
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

QTreeWidgetItem* SlnPanel::FindItem(QTreeWidgetItem *par, DocItem* mtpos)
{
	// recursive search for an element with a given identifier
	// checking this item
	DocItem* pos = par->data(0, Qt::UserRole).value<DocItem*>();
	if (pos == mtpos)
		return par;

	// recursive traversal of the rest
	int n = par->childCount();
	for (int i = 0; i < n; i++)
	{
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
	if (!dlg.DoModal())
		return;
	searchRoot = dlg.m_posSelected->GetGuid();
	if (dlg.m_posSelected)
		ui.lineNode->setText(dlg.m_posSelected->GetTitles(0));
	else
		ui.lineNode->setText("");
}

void SlnPanel::onAddToFavorites()
{
	DocItem* doc = currDoc();
	if (!doc) return;
}

void SlnPanel::onRemoveFromFavorites()
{
	FavItem* fav = currFav();
	if (!fav) return;
	int ret = QMessageBox::question(this, AppTitle, tr("Remove favorite reference? Document will be untouched."),
		QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes) {

	}
}

void SlnPanel::onEditFavoriteRef()
{
	FavItem* fav = currFav();
	if (!fav) return;
	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {

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
	QTreeWidgetItem *qnewitem = AddWorkpieceItem(qpar, qitem, "NEW GROUP", ETreeStatus::TS_FOLDER);

	QString s = QInputDialog::getText(this, "Add group", "Input group name", QLineEdit::Normal, "NEW GROUP");
	if (!s.isEmpty()) {
		// ok - insert folder
		FavItem* item = theSln.Favs.AddGroup(par, sel, s);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
			UpdateFavItem(qnewitem);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating group"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
		RemoveWorkpieceItem(qnewitem, qitem);
	}
}

void SlnPanel::onAddChildGroup()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
	if (!sel) return;

	// prepare workpiece
	QTreeWidgetItem *qnewitem = AddWorkpieceItem(qitem, nullptr, "NEW GROUP", ETreeStatus::TS_FOLDER);

	QString s = QInputDialog::getText(this, "Add group", "Input group name", QLineEdit::Normal, "NEW GROUP");
	if (!s.isEmpty()) {
		// ok - insert folder
		FavItem* item = theSln.Favs.AddGroup(sel, nullptr, s);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
			UpdateFavItem(qnewitem);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating group"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
		RemoveWorkpieceItem(qnewitem, qitem);
	}
}

void SlnPanel::onAddSiblingFav()
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
	QTreeWidgetItem *qnewitem = AddWorkpieceItem(qpar, qitem, "NEW REF", ETreeStatus::TS_UNREADY);

	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {
		// ok - insert subtree
		FavItem* item = theSln.Favs.AddRef(par, sel, dlg.m_posSelected);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
			UpdateFavItem(qnewitem);
			LoadDocLevel(item->ref, qnewitem);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error adding reference"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
		RemoveWorkpieceItem(qnewitem, qitem);
	}
}

void SlnPanel::onAddChildFav()
{
	QTreeWidgetItem *qitem = ui.treeFavorites->currentItem();
	if (!qitem) return;
	FavItem* sel = qitem->data(0, Qt::UserRole).value<FavItem*>();
	if (!sel) return;

	// prepare workpiece
	QTreeWidgetItem *qnewitem = AddWorkpieceItem(qitem, nullptr, "NEW REF", ETreeStatus::TS_UNREADY);
	
	TopicChooser dlg(this, "Select favorite item");
	if (dlg.DoModal()) {
		// ok - insert subtree
		FavItem* item = theSln.Favs.AddRef(sel, nullptr, dlg.m_posSelected);
		if (item) {
			qnewitem->setData(0, Qt::UserRole, QVariant::fromValue(item));
			UpdateFavItem(qnewitem);
			LoadDocLevel(item->ref, qnewitem);
			return; // ok
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error adding reference"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		// cancel - remove workpiece
		RemoveWorkpieceItem(qnewitem, qitem);
	}
}

void SlnPanel::onEditGroup()
{
	FavItem* fav = currFav();
	if (!fav) return;
	QString s = QInputDialog::getText(this, "Rename group", "Change group name", QLineEdit::Normal, fav->title);
	if (!s.isEmpty()) {
		fav->SetTitle(s);
		theSln.Favs.m_bModify = true;
	}
}
