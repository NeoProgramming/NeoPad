#include "mainwindow.h"

#include <QAbstractTextDocumentLayout>
#include <QDir>
#include <QDockWidget>
#include <QEventLoop>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPainter>
#include <QShortcut>
#include <QStatusBar>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextObject>
#include <QTimer>
#include <QWhatsThis>
#include <QtDebug> //to use qWarning and qDebug messages
#include <QtEvents>
#include <QFontDatabase>
#include <QToolButton>
#include <QWebFrame>

#include "tablemenu.h"
#include "slnpanel.h"
#include "quickstartdlg.h"
#include "selectdocdlg.h"
#include "savealldlg.h"
#include "newprojectdlg.h"
#include "snippetsdlg.h"
#include "prjpropsdlg.h"
#include "topicchooser.h"
#include "multieditdlg.h"
#include "passworddlg.h"

#include "../core/ini.h"
#include "../core/vmbsrv.h"
#include "../core/PrjStat.h"
#include "../core/Cryptor.h"
#include "../service/tools.h"

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)
char BUILD_DATE_TIME_STR[] = 
#include "../datetime.gen"

#if defined(Q_WS_WIN)
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

extern QTextCodec *codecUtf8;

//-------------------------------------------------
MainWindow::MainWindow()
{
	theSln.m_pCB = this;

	setUnifiedTitleAndToolBarOnMac(true);
    ui.setupUi(this);

#if defined(Q_WS_WIN)
    // Workaround for QMimeSourceFactory failing in QFileInfo::isReadable() for
    // certain user configs. See task: 34372
    qt_ntfs_permission_lookup = 0;
#endif
    m_doCommit = true;

    QSize tbSize = QSize( INI::IconSize, INI::IconSize );
    ui.toolBarEdit->setIconSize( tbSize );
    ui.toolBarInsert->setIconSize( tbSize );
    ui.toolBarPara->setIconSize( tbSize );
    ui.toolBarProject->setIconSize( tbSize );
    ui.toolBarText->setIconSize( tbSize );
    ui.toolBarTree->setIconSize( tbSize );
	ui.toolBarTools->setIconSize( tbSize );
	ui.toolBarTable->setIconSize( tbSize );
	ui.toolBarMark->setIconSize(tbSize);
		
	m_wArea = new QMdiArea;
	m_wArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_wArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_wArea->setViewMode(QMdiArea::TabbedView);
	m_wArea->setTabsClosable(true);
	m_wArea->setTabsMovable(true);

	QTabWidget *tabWidget = m_wArea->findChild<QTabWidget*>();

	QList<QTabBar *> tabBarList = m_wArea->findChildren<QTabBar*>();
	m_tabBar = tabBarList.at(0);
	if (m_tabBar) {
		m_tabBar->setExpanding(false);
		m_tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
	}

	setCentralWidget(m_wArea);

	createSpecialToolWidgets();

    m_wDock = new QDockWidget(this);
    m_wDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_wDock->setWindowTitle(tr("Outline Explorer"));
    m_wDock->setObjectName(QLatin1String("sidebar"));

    m_wSln = new SlnPanel(m_wDock, this);
    m_wDock->setWidget(m_wSln);
    addDockWidget(Qt::LeftDockWidgetArea, m_wDock);

	//m_wDock->setTitleBarWidget( m_wSln );
	
	m_iconHtm = QIcon(":/treeicons/images/treeicon-doc.png");
	m_iconVmb = QIcon(":/treeicons/images/treeicon-xml.png");

    // read geometry configuration
	loadSettings();

    if(!IsBlank(INI::TitleRedef.c_str()))
        setWindowTitle(codecUtf8->toUnicode(INI::TitleRedef.c_str()));
	SetAutoRaiseToolBar(ui.toolBarEdit, false);
	SetAutoRaiseToolBar(ui.toolBarInsert, false);
	SetAutoRaiseToolBar(ui.toolBarPara, false);
	SetAutoRaiseToolBar(ui.toolBarProject, false);
	SetAutoRaiseToolBar(ui.toolBarText, false);
	SetAutoRaiseToolBar(ui.toolBarTree, false);
	SetAutoRaiseToolBar(ui.toolBarTools, false);
	SetAutoRaiseToolBar(ui.toolBarTable, false);
	SetAutoRaiseToolBar(ui.toolBarMark, false);
	
	// read script
	loadScripts();
    
	connect(ui.actionWindowClose,	&QAction::triggered, m_wArea, &QMdiArea::closeActiveSubWindow);
	connect(ui.actionWindowCloseAll,&QAction::triggered, m_wArea, &QMdiArea::closeAllSubWindows);
	connect(ui.actionWindowTile,	&QAction::triggered, m_wArea, &QMdiArea::tileSubWindows);
	connect(ui.actionWindowCascade,	&QAction::triggered, m_wArea, &QMdiArea::cascadeSubWindows);

	connect(ui.actionWindowHTile,	&QAction::triggered, this, &MainWindow::onTileSubWindowsHorizontally);
	connect(ui.actionWindowVTile,	&QAction::triggered, this, &MainWindow::onTileSubWindowsVertically);
	connect(ui.actionAboutNeopad,	&QAction::triggered, this, &MainWindow::onAppAbout);
	connect(ui.actionAboutQt,		&QAction::triggered, this, &MainWindow::onAppAboutQt);

	
#define CONN_CHILD(act, meth) connect(act, &QAction::triggered, this, [this]() { onChild(&WebEditView::meth); } )

	CONN_CHILD(ui.actionZoomIn,		onZoomIn);
	CONN_CHILD(ui.actionZoomOut,	onZoomOut);
	CONN_CHILD(ui.actionZoomNormal, onZoomNormal);

	CONN_CHILD(ui.actionClearDoc,		onEditClearDoc);
	CONN_CHILD(ui.actionCorrectCssPath,	onEditFixCssPath);

	CONN_CHILD(ui.actionEditCut,			onEditCut);
	CONN_CHILD(ui.actionEditCutText,		onEditCutText);
	CONN_CHILD(ui.actionEditCopy,			onEditCopy);
	CONN_CHILD(ui.actionEditCopyText,		onEditCopyText);
	CONN_CHILD(ui.actionEditPaste,			onEditPaste);
	CONN_CHILD(ui.actionEditPasteText,		onEditPasteText);
	CONN_CHILD(ui.actionEditPasteImage,     onEditPasteImage);
	CONN_CHILD(ui.actionEditPasteAsTable,	onEditPasteAsTable);
	CONN_CHILD(ui.actionEditPasteCell,      onEditPasteCell);
	CONN_CHILD(ui.actionEditUndo,			onEditUndo);
	CONN_CHILD(ui.actionEditRedo,			onEditRedo);
	CONN_CHILD(ui.actionEditRedo,			onEditRedo);
	CONN_CHILD(ui.actionEditUntag,			onEditUntag);
	CONN_CHILD(ui.actionEditOutside,		onEditOutside);
	CONN_CHILD(ui.actionEditInfo,			onEditTagInfo);
	CONN_CHILD(ui.actionEditUntable,		onEditUntable);	

	CONN_CHILD(ui.actionTextBold,		onTextBold);
	CONN_CHILD(ui.actionTextItalic,		onTextItalic);
	CONN_CHILD(ui.actionTextUnderline,	onTextUnderline);
	CONN_CHILD(ui.actionTextStrike,		onTextStrike);
	CONN_CHILD(ui.actionTextCode,		onTextCode);
	CONN_CHILD(ui.actionTextSubscript,	onTextSubscript);
	CONN_CHILD(ui.actionTextSuperscript,onTextSuperscript);
	CONN_CHILD(ui.actionTextStrong,		onTextStrong);
	CONN_CHILD(ui.actionTextEm,			onTextEm);
	CONN_CHILD(ui.actionTextIns,		onTextIns);
	CONN_CHILD(ui.actionTextDel,		onTextDel);
	CONN_CHILD(ui.actionTextSamp,		onTextSamp);
	CONN_CHILD(ui.actionTextVar,		onTextVar);
	CONN_CHILD(ui.actionTextKbd,		onTextKbd);

	CONN_CHILD(ui.actionTextMark,		onTextMark);
	CONN_CHILD(ui.actionTextMark1,		onTextMark1);
	CONN_CHILD(ui.actionTextMark2,		onTextMark2);
	CONN_CHILD(ui.actionTextMark3,		onTextMark3);
	CONN_CHILD(ui.actionTextMark4,		onTextMark4);
	CONN_CHILD(ui.actionTextMark5,		onTextMark5);
	CONN_CHILD(ui.actionTextMark6,		onTextMark6);
	CONN_CHILD(ui.actionTextMark7,		onTextMark7);

	CONN_CHILD(ui.actionInsertTable,    onInsertTable);
	CONN_CHILD(ui.actionInsertHorzline,	onInsertHorzLine);
	CONN_CHILD(ui.actionInsertImage,	onInsertImage);
	CONN_CHILD(ui.actionInsertHyperlink,onInsertHyperlink);
	CONN_CHILD(ui.actionInsertDatetime,	onInsertDateTime);
	CONN_CHILD(ui.actionInsertSnippet,	onInsertSnippet);
	CONN_CHILD(ui.actionInsertSymbol,	onInsertSymbol);

	CONN_CHILD(ui.actionTableAppendData,   onTableAppendData);
	CONN_CHILD(ui.actionTableInsertData,   onTableInsertData);
	CONN_CHILD(ui.actionTableExpand,       onTableExpand);
	CONN_CHILD(ui.actionTableCollapse,     onTableCollapse);
	CONN_CHILD(ui.actionTableDeleteRow,    onTableDeleteRow);
	CONN_CHILD(ui.actionTableMoveRowAbove, onTableMoveRowAbove);
	CONN_CHILD(ui.actionTableMoveRowBelow, onTableMoveRowBelow);
		
	CONN_CHILD(ui.actionParaMarkList,	onInsertBulList);
	CONN_CHILD(ui.actionParaNumList,	onInsertNumList);

	CONN_CHILD(ui.actionFileReload,	onFileReload);
	CONN_CHILD(ui.actionFileSave,	onFileSave);
	
	CONN_CHILD(ui.actionParaH1,			onParaHeading1);
	CONN_CHILD(ui.actionParaH2,			onParaHeading2);
	CONN_CHILD(ui.actionParaH3,			onParaHeading3);
	CONN_CHILD(ui.actionParaH4,			onParaHeading4);
	CONN_CHILD(ui.actionParaH5,			onParaHeading5);
	CONN_CHILD(ui.actionParaH6,			onParaHeading6);
	CONN_CHILD(ui.actionParaDiv,        onParaDiv);
	CONN_CHILD(ui.actionParaPara,		onParaPara);
	CONN_CHILD(ui.actionParaComment,	onParaComment);
	CONN_CHILD(ui.actionParaSource,		onParaSource);
	CONN_CHILD(ui.actionParaQuestion,	onParaQuestion);
	CONN_CHILD(ui.actionParaImportant,	onParaImportant);
	CONN_CHILD(ui.actionParaFeature,	onParaFeature);
	CONN_CHILD(ui.actionParaQuote,		onParaQuote);
	CONN_CHILD(ui.actionParaAnn,		onParaAnn);
	CONN_CHILD(ui.actionParaTerm,		onParaTerm);
	CONN_CHILD(ui.actionParaNote,		onParaNote);

	CONN_CHILD(ui.actionToolsLink,      onToolsLink);
	CONN_CHILD(ui.actionToolsSearch,    onToolsSearch);
	CONN_CHILD(ui.actionToolsTranslate, onToolsTranslate);

#undef CONN_CHILD

	connect(ui.actionQuickStart, &QAction::triggered, this, &MainWindow::onProjectQuickStart);
	connect(ui.actionProjectNew, &QAction::triggered, this, &MainWindow::onProjectNew);
	connect(ui.actionProjectSave, &QAction::triggered, this, &MainWindow::onProjectSave);
	//	connect(ui.actionProjectOpen,		&QAction::triggered, this, &MainWindow::onProjectOpen);
	connect(ui.actionFileSaveAll, &QAction::triggered, this, &MainWindow::onProjectSaveAll);
	connect(ui.actionProjectProperties, &QAction::triggered, this, &MainWindow::onProjectProperties);
	connect(ui.actionProjectStatistics, &QAction::triggered, this, &MainWindow::onProjectStatistics);
	connect(ui.actionProjectPrinfPdf, &QAction::triggered, this, &MainWindow::onProjectPrintPdfBundle);
	connect(ui.actionExportPdfFiles, &QAction::triggered, this, &MainWindow::onProjectPrintPdfFiles);

	connect(ui.actionGenContents1, &QAction::triggered, this, [this]() { GenContents(0); });
	connect(ui.actionGenContents2, &QAction::triggered, this, [this]() { GenContents(1); });
	connect(ui.actionEditCss1, &QAction::triggered, this, [this]() { EditCss(0); });
	connect(ui.actionEditCss2, &QAction::triggered, this, [this]() { EditCss(1); });
	connect(ui.actionCryptBase, &QAction::triggered, this, &MainWindow::onProjectEncrypt);

	connect(ui.actionAppExit,			&QAction::triggered, this, &MainWindow::onAppExit);
	connect(ui.actionAppForceExit,		&QAction::triggered, this, &MainWindow::onAppForceExit);
	
    connect(ui.actionTreeSync,			&QAction::triggered, this, &MainWindow::onTreeSync);
	

	QAction *viewsAction = createPopupMenu()->menuAction();
	viewsAction->setText(tr("Toolbars & panels"));
	ui.viewMenu->addAction(viewsAction);

	connect(ui.actionToolsSnippets, &QAction::triggered, this, &MainWindow::onToolsSnippets);
	connect(ui.actionToolsOptions,  &QAction::triggered, this, &MainWindow::onToolsOptions);
    connect(ui.actionEditConfig,	&QAction::triggered, this, &MainWindow::onToolsEditConfig);
	connect(ui.actionEditScript,    &QAction::triggered, this, &MainWindow::onToolsEditScript);
	connect(ui.actionReloadScript,  &QAction::triggered, this, &MainWindow::onToolsReloadScript);	

	UpdateZoom(100);

	QTimer::singleShot(0, this, SLOT(onPostInit()));
}

void MainWindow::onChild(void (WebEditView::*pHandler)())
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd) {
		(wnd->*pHandler)();
	}
}

void MainWindow::onPostInit()
{
	onProjectQuickStart();
}

//-------------------------------------------------
MainWindow::~MainWindow()
{
}

//-------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *e)
{
	if (DoSaveAll())
	{
		saveSettings();
		if(theSln.m_bModify && !m_doCommit)
		{
			QString bdir = QDir(U16(INI::CurrProjectPath)).path();
			if(!IsBlank(INI::CommitPath.c_str()))
				StartExternalApplication(codecUtf8->toUnicode(INI::CommitPath.c_str()), "", bdir);
			QCoreApplication::processEvents();
			if(!IsBlank(INI::SyncPath.c_str()))
				StartExternalApplication(codecUtf8->toUnicode(INI::SyncPath.c_str()), "", bdir);
		}
		
		e->accept();
	}
	else
	{
		e->ignore();
	}
}


//-------------------------------------------------
void MainWindow::onAppAbout()
{
    QMessageBox box(this);
    QString release = tr("build %1<br>%2<br>script %3<br>%4<br>")
		.arg(codecUtf8->toUnicode(BUILD_DATE_TIME_STR))
        .arg(theSln.m_sPargPath)
		.arg(m_jsTime.toString("dd.MM.yyyy hh:mm"))
		.arg(m_jsPath);

    QString info = tr("NeoPad is powered by Qt Open Source Edition x%1").arg(sizeof(void*)*8);
    QString warranty = tr("The program is provided AS IS with NO WARRANTY OF ANY KIND.");
    box.setText(QString::fromLatin1("<p align=\"center\"><h3>NeoPad</h3></p>"
                                    "<p align=\"center\">%1</p>"
                                    "<p>%2</p>"
                                    "<p>%3<p/>"
                                    "<p>Copyright (C) NeoCode</p>"
                                    "<p align=\"center\">e-mail:<a href=\"mailto:neoprogamming@gmail.com\">neoprogamming@gmail.com</a> </p>"
    //                                "<p align=\"center\">web: <a href=\"http://neoprogramming.com/\">http://neoprogramming.com</a></p>"
	)
                                    .arg(release).arg(info).arg(warranty));
    box.setWindowTitle(tr("About NeoPad"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::onAppAboutQt()
{
    QMessageBox::aboutQt(this);
}

//-------------------------------------------------
MainWindow* MainWindow::newWindow()
{
	qDebug()<< "newWindow()";
    saveSettings();
    MainWindow *mw = new MainWindow;
    mw->move(geometry().topLeft());
    if (isMaximized())
        mw->showMaximized();
    else
        mw->show();
    return mw;
}

//-------------------------------------------------
void MainWindow::saveSettings()
{
	QByteArray ba, ha;
	ba = saveGeometry();
	ha = ba.toHex();
	INI::WinGeometry = ha.data();

	ba = saveState();
	ha = ba.toHex();
	INI::WinState = ha.data();
}

void MainWindow::loadSettings()
{
	QByteArray ba, ha;
	ha = INI::WinGeometry.c_str();
	ba = QByteArray::fromHex(ha);
	restoreGeometry( ba );
	ha = INI::WinState.c_str();
	ba = QByteArray::fromHex(ha);
	restoreState(ba);
}

void MainWindow::loadScripts()
{
     if(IsBlank(INI::ScriptsDir.c_str())) {
        m_jsPath = theSln.m_sProgDir + "/js";
    }
    else {
        m_jsPath = codecUtf8->toUnicode(INI::ScriptsDir.c_str());
        if(QDir(m_jsPath).isRelative())
            m_jsPath = theSln.m_sProgDir + "/" + m_jsPath;
    }
    m_jsPath += "/neopad.js";
    QFile file(m_jsPath);
	if(!file.open(QIODevice::ReadOnly)) {
        QString msg = QString("error opening file neopad.js (code %1) from %2").arg(file.error()).arg(m_jsPath);
		QMessageBox::warning(this, "Error", msg);
		return;
	}
	QTextStream in(&file);
	m_js = in.readAll();
	
	const QFileInfo info(file);
	m_jsTime = info.lastModified();

	file.close();
}


//-------------------------------------------------
SlnPanel* MainWindow::getSln() const
{
    return m_wSln;
}

//-------------------------------------------------
void MainWindow::onProjectOpen(QString fileName)
{
	if (!fileName.isEmpty())
	{
		m_wSln->Load();
    	projectModified(false);
	}
}

void MainWindow::onProjectEncrypt()
{
	// 1 save all


	// 2 enter password
	PasswordDlg dlg;
	if (theSln.m_Password.isEmpty()) {
		// encrypt
		//if (QMessageBox::Yes == QMessageBox::question(this, "Cryptography", "Encrypt all database?")) {
			if (dlg.DoSetPassword() == QDialog::Accepted) {
				// htmls
				theSln.EncryptDocs(theSln.GetRoot(), "", dlg.m_psw1);
				// xmls
				theSln.m_Password = dlg.m_psw1;
                theSln.SaveProject(true);
			}
		//}
	}
	else {
		// decrypt/recrypt
		dlg.m_psw0 = theSln.m_Password;
		//if (QMessageBox::Yes == QMessageBox::question(this, "Cryptography", "Recrypt all database?")) {
			if (dlg.DoChangePassword() == QDialog::Accepted) {
				// htmls
				theSln.EncryptDocs(theSln.GetRoot(), theSln.m_Password, dlg.m_psw2);
				// xmls
				theSln.m_Password = dlg.m_psw2;
                theSln.SaveProject(true);
			}
		//}
	}
}

//-------------------------------------------------
void MainWindow::onAppExit()
{
	m_doCommit = true;
	qApp->closeAllWindows();	
}

void MainWindow::onAppForceExit()
{
	m_doCommit = false;
	qApp->closeAllWindows();
}

void MainWindow::onTreeSync()
{
    WebEditView *wnd = GetActiveMdiChild();
    if(wnd) {
        m_wSln->EnsureVisible(wnd->m_Item);
    }
}

//-------------------------------------------------
void MainWindow::onProjectNew()
{
	NewProjectDlg dlg;
	if(dlg.DoModal() == QDialog::Accepted)
	{
		if(!QDir::isAbsolutePath(dlg.m_base))
			dlg.m_base = theSln.m_sProgDir + "/" + dlg.m_base;
        bool res = theSln.MakeProject(dlg.m_name, dlg.m_base, dlg.m_suffixes[0], dlg.m_suffixes[0]);
		if (res)
			theSln.addProjectToRecent( theSln.GetRoot()->GetVmbAbsPath() );
		m_wSln->Load();
		UpdateTitle();
	}
}

void MainWindow::onProjectSave()
{
	if(QMessageBox::Yes == QMessageBox::question(this, "Save all vmbases?", "Save all vmbases?")) {
        theSln.SaveProject(true);
	}
}

void MainWindow::onProjectSaveAll()
{
	QList<QMdiSubWindow *> wl = m_wArea->subWindowList();	
	QList<QMdiSubWindow *>::Iterator i = wl.begin();
	QList<QMdiSubWindow *>::Iterator e = wl.end();
	while(i != e)
	{
		QMdiSubWindow *subwnd = *i;
		WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
		if(view->isWindowModified())
		{
			view->SaveHtml(false);
		}
		++i;
	}
    getSln()->UpdateTree();
}

void MainWindow::onProjectProperties()
{
	PrjPropsDlg dlg;

	dlg.m_images   = theSln.m_ImageDir;
	dlg.m_snippets = theSln.m_Snippets.m_SnippDir;
	dlg.m_bases[0] = theSln.Books.books[0];
	dlg.m_bases[1] = theSln.Books.books[1];
		
	if(dlg.DoModal() == QDialog::Accepted)
	{
		// global paths
        theSln.m_ImageDir = dlg.m_images;
		theSln.m_Snippets.m_SnippDir = dlg.m_snippets;
		// bases
		int cnt = std::max(1, !dlg.m_bases[0].suffix.isEmpty() + !dlg.m_bases[1].suffix.isEmpty());
		theSln.Books.booksCnt = cnt;
		for (int bi = 0; bi < BCNT; bi++) {
			theSln.Books.books[bi] = dlg.m_bases[bi];
			if (theSln.Books.books[bi].load_prefix != theSln.Books.books[bi].save_prefix) {
				setCursor(Qt::WaitCursor);
				theSln.TransformDocs(bi);
				setCursor(Qt::ArrowCursor);
			}
		}
		m_wSln->LoadBookTitles();
	}
}

void MainWindow::onProjectStatistics()
{
    QString str;
    NEOPAD_STAT s, p;    

    int total = s.CalcStatistics(theSln.GetRoot());

    if(total == 0) {
        str.sprintf("BASE IS EMPTY");
    }
    else {
        for(int i=0; i<sizeof(s)/sizeof(int); i++)
            p.arr[i] = 100 * s.arr[i] / total;

        str.sprintf("TOTAL: %d pages\r\nUNREADY: %d (%d%%)\r\n25%%: %d (%d%%)\r\n50%%: %d (%d%%)\r\n75%%: %d (%d%%)\r\nALMOST: %d (%d%%)\r\nREADY: %d (%d%%)\r\nLOCKED: %d (%d%%)\r\nOTHER: %d (%d%%)\r\n"
                    "-------\r\n"
                    "NONE %d (%d%%)\r\nOK %d (%d%%)\r\nOLD %d (%d%%)\r\nQOK %d (%d%%)\r\nQOLD %d (%d%%)\r\nUND %d(%d%%)",
           total, s.x0, p.x0, s.x25, p.x25, s.x50, p.x50, s.x75, p.x75, s.x99, p.x99, s.x100, p.x100, s.xLock, p.xLock, s.xUnd, p.xUnd,
           s.tNone, p.tNone, s.tOk, p.tOk, s.tOld, p.tOld, s.tQok, p.tQok, s.tQold, p.tQold, s.tUnd, p.tUnd
        );
    }
    QMessageBox::information(this, tr("Statistics"), str );
}

//-------------------------------------------------
void MainWindow::projectModified(bool modified)
{
	ui.actionFileSave->setEnabled(modified);
}

QMenu *MainWindow::createTableMenu(const char *slot)
{
	TableMenu *tableMenu = new TableMenu(0, 10, 10);
	connect(tableMenu, SIGNAL(selected(int,int)), this, slot);	
	return tableMenu;
}

QToolButton* MainWindow::createMenuButton(const char *res, QMenu *menu, const QString &tooltip)
{
	QToolButton *btn = new QToolButton;
	btn->setPopupMode(QToolButton::MenuButtonPopup);
	btn->setMenu(menu);
	btn->setIcon(QIcon(res));
	btn->setAutoFillBackground(true);
	btn->setToolTip(tooltip);
	return btn;
}

void MainWindow::createSpecialToolWidgets()
{
	// insert table menu
	tbtnInsertTable = createMenuButton(":/insert/images/insert-table.png", createTableMenu(SLOT(onInsertTableChanged(int,int))), 
		tr("Insert table"));
	connect(tbtnInsertTable, &QToolButton::clicked, this, [this]() { onChild(&WebEditView::onInsertTable); } );
	ui.toolBarInsert->insertWidget(ui.actionInsertImage, tbtnInsertTable);
}

void MainWindow::onProjectQuickStart()
{
	QuickStartDlg dlg(this);
	if (dlg.DoModal() == QDialog::Accepted) {
		
		DoQuickStart(INI::QSModeNew);
	}
}

qreal MainWindow::UpdateZoom(int percent)
{
	ui.actionZoomOut->setEnabled(percent > 25);
	ui.actionZoomIn->setEnabled(percent < 400);
	qreal factor = static_cast<qreal>(percent) / 100;
	//@ui->webView->setZoomFactor(factor);

	return factor;
}

bool MainWindow::DoSaveAll()
{
	SaveAllDlg dlg(this);	
	theSln.MakeUnsavedList(dlg.mpl);
	dlg.wl = m_wArea->subWindowList();
	if(dlg.wl.count() <= 0)
		return true;
	return dlg.DoModal(this) == QDialog::Accepted;
}

void MainWindow::DoQuickStart(int code)
{
	switch(code)
	{
	case 0:
		DoPrjOpen(U16(INI::CurrProjectPath));
		break;
	case 1:
		onProjectNew();
		break;
	}
}

bool MainWindow::DoPrjOpen(const QString& fpath)
{
	// is encrypted
	if (isEncrypted(fpath)) {
		PasswordDlg dlg;
		if (dlg.DoEnterPassword() == QDialog::Rejected)
			return false;
		theSln.m_Password = dlg.m_psw1;
	}
	else {
		theSln.m_Password = "";
	}

    bool res = theSln.LoadProject(fpath);
	m_wSln->Load();
	if(!res) {
		QMessageBox::warning(this, "Error", 
			QString("Error loading file %1\r\n%2").arg(fpath).arg(FailMsg));
		return false;
	}
	UpdateTitle();
	return res;
}

void MainWindow::UpdateTitle()
{
	setWindowTitle(theSln.GetPrjTitle() + " - Neopad");
}

void MainWindow::DoOpenDoc(DocItem* tpos, int di)
{
	if(!tpos)
		return;
	QString path;
	path = tpos->GetDocAbsPath(di);
	if( !QFileInfo(path).isFile() )
	{
		if(!DoSelectDoc(tpos, di))
			return;
	}

	qApp->setOverrideCursor(Qt::WaitCursor);
	OpenDoc(tpos, di);
	qApp->restoreOverrideCursor();
}

bool MainWindow::DoSelectDoc(DocItem* tpos, int bi)
{
	// open 'SelectDoc' dialog
	SelectDocDlg dlg(this);
	dlg.m_DocPath = tpos->GetDocAbsPath(0); // not 'bi'
	if(dlg.DoModal() != QDialog::Accepted)
		return false;

	// make sure the folder exists for the document
	QString path = tpos->GetAbsDir(bi);
	QDir().mkpath(path);
	
	// create copy and use path of copy as existing doc
	if (dlg.m_Mode == SelectDocDlg::MODE_COPY)
		QFile::copy(dlg.m_DocPath, tpos->GetDocAbsPath(bi));
	else if (dlg.m_Mode == SelectDocDlg::MODE_MOVE)
		QFile::rename(dlg.m_DocPath, tpos->GetDocAbsPath(bi));
	else
		theSln.MakeDoc(tpos, bi);
	
	if (!QFileInfo(tpos->GetDocAbsPath(bi)).isFile())
		return false;

	if (tpos->GetTitle(bi).isEmpty())
		theSln.RenameTitle(tpos, tpos->GetId(), bi);

	tpos->SetDocTime(bi);
	
	return true;
}

void MainWindow::OpenDoc(DocItem* mtPos, int bi)
{
	if(!OpenExistingDoc(mtPos, bi))
	{
		if(!INI::OutlinerMode)
			CreateNewDoc(mtPos, bi);
		else
			LoadToCurrentDoc(mtPos, bi);
	}
}

void MainWindow::LoadToCurrentDoc(DocItem* mtPos, int di)
{

}

void MainWindow::CreateNewDoc(DocItem* mtPos, int di)
{
	WebEditView *child = new WebEditView(this, mtPos, di);
	m_wArea->addSubWindow(child);
			
	child->LoadHtml(mtPos, di);
	child->show();
}

QMdiSubWindow * MainWindow::FindTab(DocItem* mtPos, int di)
{
	QList<QMdiSubWindow *> wl = m_wArea->subWindowList();
	QList<QMdiSubWindow *>::Iterator i = wl.begin();
	QList<QMdiSubWindow *>::Iterator e = wl.end();
	while (i != e)
	{
		QMdiSubWindow *subwnd = *i;
		WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
		if (view->m_Item == mtPos && view->m_di == di)
		{
			return subwnd;
		}
		++i;
	}
	return 0;
}

bool MainWindow::OpenExistingDoc(DocItem* mtPos, int di)
{
	QMdiSubWindow *subwnd = FindTab(mtPos, di);
	if (subwnd)
	{
		m_wArea->setActiveSubWindow(subwnd);
		return 1;
	}
	return 0;
}

WebEditView *MainWindow::GetActiveMdiChild()
{
	if (QMdiSubWindow *activeSubWindow = m_wArea->activeSubWindow())
		return qobject_cast<WebEditView *>(activeSubWindow->widget());
	return 0;
}

void MainWindow::onZoomChange(int percent)
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onZoomChange(percent);
}

void MainWindow::onToolsEditScript()
{
	QString cmd = U16(INI::HtmEditPath);
	if (!QFileInfo(m_jsPath).isFile())
		QMessageBox::warning(this, "NeoPad", tr("neopad.js file not found!"));
	else
		OpenInExternalApplication(this, cmd, m_jsPath);
}

void MainWindow::onToolsReloadScript()
{
	loadScripts();
}


//////////////////////////////////////////////////////////////////////////
// insert

void MainWindow::onInsertTableChanged(int cols, int rows)
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertTable(cols, rows);
}

void MainWindow::onToolsSnippets()
{
	SnippetsDlg dlg;
	dlg.DoModal();
}

void MainWindow::onToolsOptions()
{
	
}

void MainWindow::onToolsEditConfig()
{
    theSln.saveSettings();
	QString s = theSln.m_sProgDir + "/settings.xml";

	if (!QFileInfo(s).isFile())
		QMessageBox::warning(this, tr("Config not found!"), s);

    QFile file(s);
    file.open(QFile::ReadOnly | QFile::Text );
    MultiEditDlg dlg;
    dlg.m_title = s;
    if(dlg.DoModal(codecUtf8->toUnicode(file.readAll())) == QDialog::Accepted) {
        file.close();
        file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate);
        file.write(codecUtf8->fromUnicode(dlg.m_text));
        file.close();
        theSln.loadSettings();
    }
    else {
        file.close();
    }
}

void MainWindow::onTileSubWindowsHorizontally()
{
	if (m_wArea->subWindowList().isEmpty())
		return;
	m_wArea->tileSubWindows();
	QPoint position(0, 0);
	int h = m_tabBar ? m_tabBar->height() : 0;
	foreach (QMdiSubWindow *window, m_wArea->subWindowList()) 
	{
		QRect rect(0, 0, m_wArea->width(), 
			(m_wArea->height()-h) / m_wArea->subWindowList().count());
		window->setGeometry(rect);
		window->move(position);
		position.setY(position.y() + window->height());
	}
}

void MainWindow::onTileSubWindowsVertically()
{
	if (m_wArea->subWindowList().isEmpty())
		return;
	m_wArea->tileSubWindows();
	QPoint position(0, 0);
	int h = m_tabBar ? m_tabBar->height() : 0;
	foreach (QMdiSubWindow *window, m_wArea->subWindowList()) 
	{
		QRect rect(0, 0, m_wArea->width() / m_wArea->subWindowList().count(), 
			m_wArea->height()-h);
		window->setGeometry(rect);
		window->move(position);
		position.setX(position.x() + window->width());
	}	
}

void MainWindow::onProjectPrintPdfBundle()
{
	if (!QFileInfo(U16(INI::PdfgenPath)).isFile())
	{
		QMessageBox::warning(this, tr("Error"), tr("pdf generation software (wkhtmltopdf) not found; path: ") + U16(INI::PdfgenPath));
		return;
	}

	TopicChooser dlg(this, "Select items to print");
	if (!dlg.DoModal(true) || !dlg.m_posSelected)
		return;
	
	QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", QString(), "*.pdf");
	if (fileName.isEmpty())
		return;

	setCursor(Qt::WaitCursor);
	QApplication::processEvents();

	if (QFileInfo(fileName).suffix().isEmpty())
		fileName.append(".pdf");
		
	QString toc;
	QStringList args;
	int page = 1;
	MakePagesListForPdfPrinting(dlg.m_posSelected, 0, page, args, toc);

	// save toc
	QFile file(fileName + ".txt");
	if(file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		file.write(toc.toUtf8());
		file.close();
	}

	// make pdf
	args << fileName;
	int c = StartExternalApplication(U16(INI::PdfgenPath), args, "");
	
	this->setCursor(Qt::ArrowCursor);

	QString str;
	str.sprintf("%d", c);
	QMessageBox::information(this, tr("Exit code"), str );
	return;
}

void MainWindow::onProjectPrintPdfFiles()
{
	if (!QFileInfo(codecUtf8->toUnicode(INI::PdfgenPath.c_str())).isFile())
	{
		QMessageBox::warning(this, tr("Error"), tr("pdf generation software (wkhtmltopdf) not found; path: ") + U16(INI::PdfgenPath));
		return;
	}

	TopicChooser dlg(this, "Select items to print");
	if (!dlg.DoModal(true) || !dlg.m_posSelected)
		return;

	QString filePath = QFileDialog::getExistingDirectory(this, tr("Select Dir"), QString());
	if (filePath.isEmpty())
		return;

	setCursor(Qt::WaitCursor);
	QApplication::processEvents();
	
	QString toc;
	QStringList args;
	int page = 1;

	MakePagesListForPdfPrinting(dlg.m_posSelected, 0, page, args, toc);

	// save toc
	QFile fileToc(filePath + "/__toc.txt");
	if (fileToc.open(QIODevice::WriteOnly | QIODevice::Text)) {
		fileToc.write(toc.toUtf8());
	}
	fileToc.close();

	// make pdf files from each page
	int n = 0, ok = 0, er = 0;
	QFile fileLog(filePath + "/__log.txt");
	bool log = fileLog.open(QIODevice::WriteOnly | QIODevice::Text);
	if (log)
		fileLog.write("errors log\n\n");
	for (QStringList::iterator i = args.begin(), e = args.end(); i != e; ++i, ++n)
	{
		// generate name
		QString fname;
		fname.sprintf("/%08d.pdf", n);
		QString fpath = filePath + fname;
		QStringList sl;
		sl.push_back(*i);
		sl.push_back(fpath);
		int c = StartExternalApplication(U16(INI::PdfgenPath), sl, "");
		if (!c) ok++;
		else {
			er++;
			if (log) {
				fileLog.write(i->toUtf8());
				fileLog.write("\n");
			}
		}
	}
	if (log)
		fileLog.close();
	this->setCursor(Qt::ArrowCursor);

	QString str;
	str.sprintf("ok %d, errors %d", ok, er);
	QMessageBox::information(this, tr("Exit code"), str);
	return;
}

void MainWindow::MakePagesListForPdfPrinting(DocItem* tpos, int level, int &page, QStringList &args, QString &toc)
{
	if (!tpos->GetCheck())
		return;

	QString s, path = tpos->GetDocAbsPath(0);
	path.replace('\\','/');
	args << path;

	for (int i = 0; i < level; i++)
		toc += "\t";
	toc += tpos->GetTitle(0);
	s.sprintf("\t%d\r\n", page);
	toc += s;
	page++;
		
	for(auto tchild : tpos->children)
	{
		MakePagesListForPdfPrinting(tchild, level + 1, page, args, toc);
	}
}

void MainWindow::UpdateTab(DocItem* tpos)
{
	QMdiSubWindow * subwnd;
	subwnd = FindTab(tpos, 0);
	if (subwnd)
		subwnd->setWindowTitle(tpos->GetTitle(0));
	subwnd = FindTab(tpos, 1);
	if (subwnd)
		subwnd->setWindowTitle(tpos->GetTitle(1));
}

void MainWindow::CloseTab(DocItem* tpos, bool clear_modify)
{
    QMdiSubWindow * subwnd;
    subwnd = FindTab(tpos, 0);
    if (subwnd) {
        if(clear_modify) {
            WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
            view->setWindowModified(false);
        }
        subwnd->close();
    }
    subwnd = FindTab(tpos, 1);
    if (subwnd) {
        if(clear_modify) {
            WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
            view->setWindowModified(false);
        }
        subwnd->close();
    }
}

void MainWindow::GenContents(int bi)
{
	bool ok = true;
	QString base = QInputDialog::getText(this, "Input base URL", "URL:", QLineEdit::Normal, "", &ok);
	if (ok) {
		if (!base.isEmpty() && !base.endsWith('/'))
			base += '/';
		QString ctxName = QFileDialog::getSaveFileName(this, tr("Select contents file"),
			theSln.GetBookDir(bi) + "/contents.html", "HTML files (*.html)");
		if (!ctxName.isEmpty())
			theSln.GenContents(bi, ctxName, base);
	}
}

void MainWindow::EditCss(int bi)
{
	QString cssPath = theSln.GetCssAbsPath(bi);
	QString cmd = U16(INI::HtmEditPath);
	if (!QFileInfo(cssPath).exists())
		QMessageBox::warning(this, "NeoPad", tr("CSS file not found!"));
	else
		OpenInExternalApplication(this, cmd, cssPath);
}

void* MainWindow::FindOpenedDoc(DocItem* pos, int di)
{
	QList<QMdiSubWindow *> wl = m_wArea->subWindowList();
	QList<QMdiSubWindow *>::Iterator i = wl.begin();
	QList<QMdiSubWindow *>::Iterator e = wl.end();
	while (i != e)
	{
		QMdiSubWindow *subwnd = *i;
		WebEditView *view = qobject_cast<WebEditView *>(subwnd->widget());
		if (view->m_Item == pos && view->m_di == di)
			return view;
		++i;
	}
	return nullptr;
}

void MainWindow::GetDocData(void* wnd, QString &html)
{
	WebEditView *view = (WebEditView*)wnd;
	html = view->page()->mainFrame()->toHtml();
}

void MainWindow::OpenLocalLink(const QString &url, int di)
{
	// extract '?' part
	int i = url.indexOf('?');
	if (i < 0)
		return;
	QString guid = url.mid(i + 1);
	DocItem* pos = theSln.Locate(guid);
	if (!pos)
		return;
	OpenDoc(pos, di);
}

void MainWindow::Search(const QString &text)
{
    // Search
    m_wSln->Search(text);
}

void MainWindow::setStatus(const QString &str)
{
    statusBar()->showMessage(str);
}

