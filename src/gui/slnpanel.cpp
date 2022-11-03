#include "slnpanel.h"
#include "topicchooser.h"
#include "mainwindow.h"
#include "itemproperty.h"
#include "newitemdlg.h"
#include "existitemdlg.h"

#include <QtGui>
#include <QtDebug>
#include <QFile>   		
#include <QTextStream>	
#include <QTreeWidgetItem>

#include <stdlib.h>
#include <limits.h>

#include "../core/vmbsrv.h"
#include "../core/ini.h"
#include "../service/tools.h"
#include "../service/search.h"

extern QTextCodec *codecUtf8;



//==================== class HelpDialog ====================
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
	
	QHeaderView * header = ui.treeContents->header();
	header->setSectionResizeMode(QHeaderView::Interactive);
	ui.treeContents->setColumnWidth(0, 220);
	ui.treeContents->setColumnWidth(1, 220);
	ui.treeContents->setIconSize(QSize(20, 20));

	connect(ui.treeContents, SIGNAL(dropping(QTreeWidgetItem*, QTreeWidgetItem*, int)), this, SLOT(onDropping(QTreeWidgetItem*, QTreeWidgetItem*, int)));
    connect(ui.pushSearch, &QPushButton::clicked, this, &SlnPanel::onSearch);
	connect(ui.pushFindNext, &QPushButton::clicked, this, &SlnPanel::onFindNext);
	connect(ui.pushFindPrev, &QPushButton::clicked, this, &SlnPanel::onFindPrev);

	m_TreeIcons[(int)ETreeStatus::TS_UNKNOWN] = QIcon(":/treeicons/images/ti-unknown.png");
	m_TreeIcons[(int)ETreeStatus::TS_READY] = QIcon(":/treeicons/images/ti-html.png");
	m_TreeIcons[(int)ETreeStatus::TS_ALMOST] = QIcon(":/treeicons/images/ti-htmlx.png");
	m_TreeIcons[(int)ETreeStatus::TS_75] = QIcon(":/treeicons/images/ti-folder75.png");
	m_TreeIcons[(int)ETreeStatus::TS_50] = QIcon(":/treeicons/images/ti-folder50.png");
	m_TreeIcons[(int)ETreeStatus::TS_25] = QIcon(":/treeicons/images/ti-folder25.png");
	m_TreeIcons[(int)ETreeStatus::TS_UNREADY] = QIcon(":/treeicons/images/ti-underconstr.png");
	m_TreeIcons[(int)ETreeStatus::TS_LOCKED] = QIcon(":/treeicons/images/ti-locked.png");

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
}

QAction *SlnPanel::MakeAction(QString text, QMenu *menu, const char *slot)
{
	QAction *action = new QAction(this);
	action->setText(text);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()), this, slot);
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

QAction *SlnPanel::MakeAction(QString text, const char *slot)
{
	QAction *action = new QAction(this);
	action->setText(text);
	connect(action, SIGNAL(triggered()), this, slot);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, QKeySequence skey, const char *slot)
{
	QAction *action = new QAction(this);
	action->setText(text);
	action->setShortcut(skey);
	connect(action, SIGNAL(triggered()), this, slot);
	return action;
}

QAction *SlnPanel::MakeAction(QString text, QKeySequence skey, QMenu *menu, const char *slot)
{
	QAction *action = new QAction(this);
	action->setText(text);
	action->setShortcut(skey);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()), this, slot);
	return action;
}


//-------------------------------------------------
void SlnPanel::initialize()
{
	ui.tabWidget->setElideMode(Qt::ElideNone);	//show tab titles, do not elide them by placing "..."

	connect(ui.treeContents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onShowContentsMenu(QPoint)));
	connect(ui.treeContents, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(ui.treeResults, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onResDoubleClicked(QTreeWidgetItem*, int)));

    ui.tabWidget->setCurrentIndex(0);

	ui.treeContents->viewport()->installEventFilter(this);	// event filter - for mouse
	ui.treeContents->installEventFilter(this);				// event filter - for keyboard

	connect(mw->ui.actionTreeMoveUp, SIGNAL(triggered()), this, SLOT(onMoveItemUp()));
	connect(mw->ui.actionTreeMoveDown, SIGNAL(triggered()), this, SLOT(onMoveItemDown()));
	connect(mw->ui.actionTreeMoveParent, SIGNAL(triggered()), this, SLOT(onMoveItemParent()));
	connect(mw->ui.actionTreeMoveChild, SIGNAL(triggered()), this, SLOT(onMoveItemChild()));
	connect(mw->ui.actionTreeAddChild, SIGNAL(triggered()), this, SLOT(onInsertNewChild()));
	connect(mw->ui.actionTreeAddSibling, SIGNAL(triggered()), this, SLOT(onInsertNewSibling()));
	connect(mw->ui.actionTreeRemoveItem, SIGNAL(triggered()), this, SLOT(onRemoveItem()));

	menuPopupContents = new QMenu(this);  //for Contents tab
	// formation of actions for the context menu
	submenuOpen0Ext = new QMenu(tr("Doc0"), menuPopupContents);
	submenuOpen1Ext = new QMenu(tr("Doc1"), menuPopupContents);
	
	// doc0
	MakeAction(tr("Open in New Tab"), submenuOpen0Ext, [this](){ onOpenInNewTab(0); });
	MakeAction(tr("Open in External Doc Editor"), submenuOpen0Ext, [this](){ onOpenInExtDocEditor(0); });
	MakeAction(tr("Open in External Browser"), submenuOpen0Ext, [this](){ onOpenInExtBrowser(0); });
	MakeAction(tr("Open in External Text Editor"), submenuOpen0Ext, [this](){ onOpenInExtTextEditor(0); });
	MakeAction(tr("Open Folder"), submenuOpen0Ext, [this](){ onOpenFolder(0); });

	// doc1
	MakeAction(tr("Open in New Tab"), submenuOpen1Ext, [this](){ onOpenInNewTab(1); });
	MakeAction(tr("Open in External Doc Editor"), submenuOpen1Ext, [this](){ onOpenInExtDocEditor(1); });
	MakeAction(tr("Open in External Browser"), submenuOpen1Ext, [this](){ onOpenInExtBrowser(1); });
	MakeAction(tr("Open in External Text Editor"), submenuOpen1Ext, [this](){ onOpenInExtTextEditor(1); });
	MakeAction(tr("Open Folder"), submenuOpen1Ext, [this](){ onOpenFolder(1); });
	
	// common actions
	actionOpenFolder = MakeAction(tr("Open Folder"), SLOT(onOpenFolderVmb()));
	actionOpenVmbaseInTextEditor = MakeAction(tr("Open VMB in Text Editor"), SLOT(onOpenVmbaseInExtTextEditor()));
	actionItemMove = MakeAction(tr("Move item..."), SLOT(onMoveItem()));

	//!+! shortcuts does not trigger action, see eventFilter()
	actionItemProperties = MakeAction(tr("Item properties..."), QKeySequence(Qt::ControlModifier + Qt::Key_Space), SLOT(onItemProperties()));

	QMenu *submenuItemStatus = new QMenu(tr("Item Status"), menuPopupContents);
	MakeAction(tr("Ready"),				submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_READY); });
	MakeAction(tr("Almost ready"),		submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_ALMOST); });
	MakeAction(tr("75 %"),				submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_75); });
	MakeAction(tr("50 %"),				submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_50); });
	MakeAction(tr("25 %"),				submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_25); });
	MakeAction(tr("Under construction"),submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_UNREADY); });
	MakeAction(tr("Locked"),			submenuItemStatus, [this]() { SetCurrItemStatus(ETreeStatus::TS_LOCKED); });

	QMenu *submenuNodeStatus = new QMenu(tr("Node Status"), menuPopupContents);
	MakeAction(tr("Ready"),				submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_READY); });
	MakeAction(tr("Almost ready"),		submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_ALMOST); });
	MakeAction(tr("75 %"),				submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_75); });
	MakeAction(tr("50 %"),				submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_50); });
	MakeAction(tr("25 %"),				submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_25); });
	MakeAction(tr("Under construction"),submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_UNREADY); });
	MakeAction(tr("Locked"),			submenuNodeStatus, [this]() { SetCurrNodeStatus(ETreeStatus::TS_LOCKED); });
	
	QMenu *submenuInsert = new QMenu(tr("Insert"), menuPopupContents);
	MakeAction(tr("New subitem"), QKeySequence(Qt::Key_Insert), submenuInsert, SLOT(onInsertNewChild()));
	MakeAction(tr("New sibling item"), QKeySequence(Qt::Key_Enter), submenuInsert, SLOT(onInsertNewSibling()));
	
	QMenu *submenuDelete = new QMenu(tr("Delete"), menuPopupContents);
	MakeAction(tr("Remove item (do not touch source files)"), QKeySequence(Qt::Key_Delete), submenuDelete, SLOT(onRemoveItem()));
	MakeAction(tr("Delete item and source files"), submenuDelete, SLOT(onDeleteItem()));
	MakeAction(tr("Delete document file"), submenuDelete, SLOT(onDeleteDoc()));

	// formation of a context menu
	menuPopupContents->addAction(actionItemProperties);

	menuPopupContents->addSeparator();
	menuPopupContents->addMenu(submenuOpen0Ext);
	menuPopupContents->addMenu(submenuOpen1Ext);
	menuPopupContents->addAction(actionOpenVmbaseInTextEditor);
	menuPopupContents->addAction(actionOpenFolder);

	menuPopupContents->addSeparator();
	menuPopupContents->addMenu(submenuItemStatus);
	menuPopupContents->addMenu(submenuNodeStatus);

	menuPopupContents->addSeparator();
	menuPopupContents->addMenu(submenuInsert);
	menuPopupContents->addMenu(submenuDelete);
	menuPopupContents->addAction(actionItemMove);
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
					onInsertNewChild();
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
					onInsertNewSibling();
				break;
			default:
				break;
			}
		}
	}
	return QWidget::eventFilter(o, e);
}

void SlnPanel::LoadTreeLevel(MTPOS tposNode, QTreeWidgetItem *parent)
{
	for (MTPOS tpos : tposNode->children)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(parent);
		item->setData(0, Qt::UserRole, qVariantFromValue((void*)tpos));
    //    item->setBackgroundColor(0,  QColor(255,255,0));
		UpdateItem(item);
		LoadTreeLevel(tpos, item);
	}
}

void SlnPanel::UpdateBases()
{
	QTreeWidgetItem* headerItem = ui.treeContents->headerItem();
	headerItem->setText(0, theSln.m_Bases[0].title);
	headerItem->setText(1, theSln.m_Bases[1].title);
}

void SlnPanel::LoadTree()
{
	MTPOS tposRoot = theSln.GetRoot();
	if (!tposRoot)
		return;

	setCursor(Qt::WaitCursor);

	ui.treeContents->clear();
	QTreeWidgetItem *root = new QTreeWidgetItem();
	root->setText(0, tposRoot->title[0]);
	root->setData(0, Qt::UserRole, qVariantFromValue((void*)tposRoot));

	ui.treeContents->addTopLevelItem(root);
	UpdateItem(root);

	LoadTreeLevel(tposRoot, root);
	root->setExpanded(true);

	UpdateBases();

	submenuOpen0Ext->setTitle(tr("Doc0 (%1)").arg(theSln.m_Bases[0].title));
	submenuOpen1Ext->setTitle(tr("Doc1 (%1)").arg(theSln.m_Bases[1].title));

	setCursor(Qt::ArrowCursor);
	showInitDoneMessage();
}

//-------------------------------------------------
void SlnPanel::onShowContentsMenu(const QPoint &pos)
{
	QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(sender());
	if (!treeWidget)
		return;
	QTreeWidgetItem *item = treeWidget->itemAt(pos);
	if (!item)
		return;

	QAction *action;
	action = menuPopupContents->exec(treeWidget->viewport()->mapToGlobal(pos)); //ContCur == ContTreeView
}

//-------------------------------------------------
void SlnPanel::RemoveItemDontAsk(bool del_files)
{
	// remove item from tree
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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

void SlnPanel::OpenDoc(QTreeWidgetItem *item, int di)
{
	if (!item)
		item = ui.treeContents->currentItem();
	if (!item)
		return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	mw->DoOpenDoc(tpos, di);
	UpdateItem(item);
}

void SlnPanel::onResDoubleClicked(QTreeWidgetItem* curItem, int column)
{
    // open document by double click
    onItemDoubleClicked(curItem, column);
    // search first occurence
    onFindNext();
}

void SlnPanel::onItemDoubleClicked(QTreeWidgetItem* curItem, int column)
{
	// open document by double click
	OpenDoc(curItem, column);
}

void SlnPanel::DoChangeTreeItemStatus(ETreeStatus status, bool rec)
{
	// change the status of the current tree item (and possibly all children)
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, rec);
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


void SlnPanel::UpdateItem(QTreeWidgetItem * item)
{
	// set the text and picture of the node depending on its state and attributes
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();

	item->setText(0, tpos->GetTitle(0));
	ETreeStatus im = tpos->GetTreeStatus();
	item->setIcon(0, GetTreeItemIcon(im));

	item->setText(1, tpos->GetTitle(1));
	ELangStatus ls = tpos->GetLangStatus(1);
	item->setIcon(1, GetLangItemIcon(ls));
}

void SlnPanel::UpdateNode(QTreeWidgetItem * item)
{
	// recursively update all pictures of the whole tree
	UpdateItem(item);
	int n = item->childCount();
	for (int i = 0; i < n; i++)
	{
		QTreeWidgetItem *child = item->child(i);
		UpdateNode(child);
	}
}

void SlnPanel::UpdateTreeItem(MTPOS pos)
{
	QTreeWidgetItem *item = FindItem(ui.treeContents->topLevelItem(0), pos);
	if (item)
		UpdateItem(item);
}

void SlnPanel::UpdateTree()
{
	UpdateNode(ui.treeContents->topLevelItem(0));
}

void SlnPanel::SetCurrItemStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, false);
	UpdateItem(item);
}

void SlnPanel::SetCurrNodeStatus(ETreeStatus status)
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return;

	theSln.SetStatus(tpos, status, true);
	UpdateNode(item);
}

//////////////////////////////////////////////////////////////////////////
// item context menu

void SlnPanel::onItemProperties()
{
	// rename item, document file and tree file
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return;

	ItemProperties dlg(this);
	if (dlg.DoModal(tpos) == QDialog::Accepted)	// inside the check for the correctness of renaming
	{
		theSln.RenameTitle(tpos, dlg.m_title0, 0);
		theSln.RenameTitle(tpos, dlg.m_title1, 1);
		if (tpos->GetId() != dlg.m_id) {
			if (!theSln.RenameItem(tpos, dlg.m_id))
				QMessageBox::warning(this, "Rename error", FailMsg);
		}
		UpdateItem(item);
		mw->UpdateTab(tpos);
	}
}

//////////////////////////////////////////////////////////////////////////
// external programs

void SlnPanel::onOpenInNewTab(int di)
{
	OpenDoc(nullptr, di);
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
	if (ret == QMessageBox::Yes)
	{
		QTreeWidgetItem *item = ui.treeContents->currentItem();
		if (di < 0)
			di = ui.treeContents->currentColumn();
		MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
		theSln.RemoveNodeDoc(tpos, di);
		UpdateItem(item);
		mw->projectModified(true);
	}
}

void SlnPanel::onOpenVmbaseInExtTextEditor()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
	MTPOS tpDrag = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpDrag) return;

	TopicChooser dlg(this, "Select new parent item");
	if (dlg.DoModal())
	{
		if (!theSln.Move(tpDrag, dlg.m_posSelected, NULL))
		{
			QMessageBox::warning(this, "Move error", FailMsg, QMessageBox::Ok);
		}
		else
		{
			QTreeWidgetItem *npar = FindItem(ui.treeContents->topLevelItem(0), dlg.m_posSelected);
			if (npar)
			{
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

	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
void SlnPanel::onInsertNewChild()
{
	// insert the blank into the tree
	NewItemDlg dlg;
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");

	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) 
		return;
	MTPOS tpPar = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpPar) 
		return;

	//dlg.m_title = dlg.m_id = GenerateUniqueFTitle(theSln.Get

	QTreeWidgetItem *newitem = new QTreeWidgetItem(item);
	newitem->setText(0, dlg.m_title);
	newitem->setIcon(0, GetTreeItemIcon(ETreeStatus::TS_UNREADY));
	item->setExpanded(true);

	if (dlg.DoModal() == QDialog::Accepted)
	{
		// ok - insert the element into the base and connect the workpiece
		MTPOS tpNew = theSln.AddItem(tpPar, nullptr, dlg.m_title, dlg.m_id);
		if (tpNew)
		{
			newitem->setData(0, Qt::UserRole, qVariantFromValue((void*)tpNew));
			UpdateItem(newitem);
			ui.treeContents->setCurrentItem(newitem);
			if (dlg.m_open) {
				OpenDoc(newitem, 0);
			}
			return;
		}
	}

	// cancel - remove the workpiece
	int index = item->indexOfChild(newitem);
	delete item->takeChild(index);
}

void SlnPanel::onInsertNewSibling()
{
	// insert a new item after the given one
	NewItemDlg dlg;
	dlg.m_title = dlg.m_id = theSln.m_fnum.GenNewName("doc");

	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return;
	MTPOS tpAfter = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpAfter) return;
	QTreeWidgetItem *par = item->parent();
	if (!par) return;
	MTPOS tpPar = (MTPOS)par->data(0, Qt::UserRole).value<void*>();
	if (!tpPar) return;

	QTreeWidgetItem *newitem = new QTreeWidgetItem(par, item);
	newitem->setText(0, dlg.m_title);
	newitem->setIcon(0, GetTreeItemIcon(ETreeStatus::TS_UNREADY));
	item->setExpanded(true);

	if (dlg.DoModal() == QDialog::Accepted)
	{
		// ok - insert the element into the base and connect the workpiece
		MTPOS tpNew = theSln.AddItem(tpPar, tpAfter, dlg.m_title, dlg.m_id);
		if (tpNew)
		{
			newitem->setData(0, Qt::UserRole, qVariantFromValue((void*)tpNew));
			UpdateItem(newitem);
			ui.treeContents->setCurrentItem(newitem);
			if(dlg.m_open) {
				OpenDoc(newitem, 0);
			}
			return;
		}
		else {
			QMessageBox::warning(this, AppTitle, tr("Error creating node"), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	// cancel - remove the workpiece
	int index = par->indexOfChild(newitem);
	delete par->takeChild(index);
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
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
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
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return;

	if (theSln.MoveChild(tpos))
	{
		QTreeWidgetItem *prev = ui.treeContents->GetPrevSibling(item);
		ui.treeContents->MoveItem(item, prev, 0);
		ui.treeContents->setCurrentItem(item);
	}
}
MTPOS SlnPanel::GetMtposFromRes()
{
	QTreeWidgetItem *item = ui.treeContents->currentItem();
	if (!item) return 0;
	MTPOS tpos = (MTPOS)item->data(0, Qt::UserRole).value<void*>();
	if (!tpos) return 0;

	if (tpos != theSln.GetRoot())
	{
		QMessageBox::warning(this, AppTitle, tr("Resourse settings are available only from root item"), QMessageBox::Ok);
		return 0;
	}
	return tpos;
}

int SlnPanel::onDropping(QTreeWidgetItem *drag, QTreeWidgetItem *drop, int m)
{
	// drag event
	// Qt::KeyboardModifiers
	// return Qt::IgnoreAction or Qt::MoveAction
	if (!drag || !drop)
		return 0;
	MTPOS tpDrag = (MTPOS)drag->data(0, Qt::UserRole).value<void*>();
	MTPOS tpDrop = (MTPOS)drop->data(0, Qt::UserRole).value<void*>();

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

QTreeWidgetItem* SlnPanel::FindItem(QTreeWidgetItem *par, MTPOS mtpos)
{
	// recursive search for an element with a given identifier
	// checking this item
	MTPOS pos = (MTPOS)par->data(0, Qt::UserRole).value<void*>();
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

void SlnPanel::EnsureVisible(MTPOS node)
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
    CMtposList results;
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
    theSln.Search(text, scope, results);
    ui.treeResults->clear();
    for(auto pos : results) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setData(0, Qt::UserRole, qVariantFromValue((void*)pos));
        ui.treeResults->addTopLevelItem(item);
        UpdateItem(item);
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

