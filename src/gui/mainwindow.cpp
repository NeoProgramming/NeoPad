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
//	tabWidget->setTabBar(new CustomTabBar(tabWidget));
//	m_wArea->setTabBar(new TabBar(m_wArea));

	QList<QTabBar *> tabBarList = m_wArea->findChildren<QTabBar*>();
	m_tabBar = tabBarList.at(0);
	if (m_tabBar) {
		m_tabBar->setExpanding(false);
		m_tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
	}

	setCentralWidget(m_wArea);

	windowMapper = new QSignalMapper(this);
	connect(windowMapper, SIGNAL(mapped(QWidget*)),  this, SLOT(setActiveSubWindow(QWidget*)));

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

#define CHILD(meth)	[this]() { onChild(&WebEditView::meth); }
    
	connect(ui.actionWindowClose,		&QAction::triggered, m_wArea, &QMdiArea::closeActiveSubWindow);
	connect(ui.actionWindowCloseAll,	&QAction::triggered, m_wArea, &QMdiArea::closeAllSubWindows);
	connect(ui.actionWindowTile,		&QAction::triggered, m_wArea, &QMdiArea::tileSubWindows);
	connect(ui.actionWindowCascade,		&QAction::triggered, m_wArea, &QMdiArea::cascadeSubWindows);

	connect(ui.actionWindowHTile,		&QAction::triggered, this, &MainWindow::onTileSubWindowsHorizontally);
	connect(ui.actionWindowVTile,		&QAction::triggered, this, &MainWindow::onTileSubWindowsVertically);
	connect(ui.actionAboutNeopad,		&QAction::triggered, this, &MainWindow::onAppAbout);
	connect(ui.actionAboutQt,			&QAction::triggered, this, &MainWindow::onAppAboutQt);
	
	connect(ui.actionClearDoc,			&QAction::triggered, this, &MainWindow::onEditClearDoc);
	connect(ui.actionCorrectCssPath,	&QAction::triggered, this, &MainWindow::onEditFixCssPath);

	connect(ui.actionEditCut,			&QAction::triggered, this, &MainWindow::onEditCut);
	connect(ui.actionEditCutText,		&QAction::triggered, this, &MainWindow::onEditCutText);
	connect(ui.actionEditCopy,			&QAction::triggered, this, &MainWindow::onEditCopy);
    connect(ui.actionEditCopyText,		&QAction::triggered, this, &MainWindow::onEditCopyText);
	connect(ui.actionEditPaste,			&QAction::triggered, this, &MainWindow::onEditPaste);
	connect(ui.actionEditPasteText,		&QAction::triggered, this, &MainWindow::onEditPasteText);
	connect(ui.actionEditPasteImage,    &QAction::triggered, this, &MainWindow::onEditPasteImage);
    connect(ui.actionEditPasteAsTable,	&QAction::triggered, this, &MainWindow::onEditPasteAsTable);
	connect(ui.actionEditPasteCell,     &QAction::triggered, this, &MainWindow::onEditPasteCell);
	connect(ui.actionEditUndo,			&QAction::triggered, this, &MainWindow::onEditUndo);
	connect(ui.actionEditRedo,			&QAction::triggered, this, &MainWindow::onEditRedo);
	connect(ui.actionEditRedo,			&QAction::triggered, this, &MainWindow::onEditRedo);
	connect(ui.actionEditUntag,			&QAction::triggered, this, &MainWindow::onEditUntag);
	connect(ui.actionEditOutside,		&QAction::triggered, this, &MainWindow::onEditOutside);
	connect(ui.actionEditInfo,			&QAction::triggered, this, &MainWindow::onEditTagInfo);
	connect(ui.actionEditUntable,		&QAction::triggered, this, &MainWindow::onEditUntable);

	connect(ui.actionZoomIn,			&QAction::triggered, this, &MainWindow::onZoomIn);
	connect(ui.actionZoomOut,			&QAction::triggered, this, &MainWindow::onZoomOut);
	connect(ui.actionZoomNormal,		&QAction::triggered, this, &MainWindow::onZoomNormal);

	//connect(ui.actionTextBold,			&QAction::triggered, this, &MainWindow::onTextBold);
	connect(ui.actionTextBold,			&QAction::triggered, this, CHILD(onTextBold));

	connect(ui.actionTextItalic,		&QAction::triggered, this, &MainWindow::onTextItalic);
	connect(ui.actionTextUnderline,		&QAction::triggered, this, &MainWindow::onTextUnderline);
	connect(ui.actionTextStrike,		&QAction::triggered, this, &MainWindow::onTextStrike);
	connect(ui.actionTextCode,			&QAction::triggered, this, &MainWindow::onTextCode);
	connect(ui.actionTextSubscript,		&QAction::triggered, this, &MainWindow::onTextSubscript);
	connect(ui.actionTextSuperscript,	&QAction::triggered, this, &MainWindow::onTextSuperscript);
	connect(ui.actionTextStrong,		&QAction::triggered, this, &MainWindow::onTextStrong);
	connect(ui.actionTextEm,			&QAction::triggered, this, &MainWindow::onTextEm);
	connect(ui.actionTextIns,			&QAction::triggered, this, &MainWindow::onTextIns);
	connect(ui.actionTextDel,			&QAction::triggered, this, &MainWindow::onTextDel);
	connect(ui.actionTextSamp,			&QAction::triggered, this, &MainWindow::onTextSamp);
	connect(ui.actionTextVar,			&QAction::triggered, this, &MainWindow::onTextVar);
	connect(ui.actionTextKbd,			&QAction::triggered, this, &MainWindow::onTextKbd);

	connect(ui.actionTextMark,			&QAction::triggered, this, &MainWindow::onTextMark);
	connect(ui.actionTextMark1,			&QAction::triggered, this, &MainWindow::onTextMark1);
	connect(ui.actionTextMark2,			&QAction::triggered, this, &MainWindow::onTextMark2);
	connect(ui.actionTextMark3,			&QAction::triggered, this, &MainWindow::onTextMark3);
	connect(ui.actionTextMark4,			&QAction::triggered, this, &MainWindow::onTextMark4);
	connect(ui.actionTextMark5,			&QAction::triggered, this, &MainWindow::onTextMark5);
	connect(ui.actionTextMark6,			&QAction::triggered, this, &MainWindow::onTextMark6);
	connect(ui.actionTextMark7,			&QAction::triggered, this, &MainWindow::onTextMark7);

	connect(ui.actionInsertHorzline,	&QAction::triggered, this, &MainWindow::onInsertHorzLine);
	connect(ui.actionInsertTable,		&QAction::triggered, this, &MainWindow::onInsertTableClicked);
	connect(ui.actionInsertImage,		&QAction::triggered, this, &MainWindow::onInsertImage);
	connect(ui.actionInsertHyperlink,	&QAction::triggered, this, &MainWindow::onInsertHyperlink);
	connect(ui.actionInsertDatetime,	&QAction::triggered, this, &MainWindow::onInsertDateTime);
	connect(ui.actionInsertSnippet,		&QAction::triggered, this, &MainWindow::onInsertSnippet);
	connect(ui.actionInsertSymbol,		&QAction::triggered, this, &MainWindow::onInsertSymbol);

	connect(ui.actionTableAppendData,   &QAction::triggered, this, &MainWindow::onTableAppendData);
	connect(ui.actionTableInsertData,   &QAction::triggered, this, &MainWindow::onTableInsertData);
	connect(ui.actionTableExpand,       &QAction::triggered, this, &MainWindow::onTableExpand);
	connect(ui.actionTableCollapse,     &QAction::triggered, this, &MainWindow::onTableCollapse);
	connect(ui.actionTableDeleteRow,    &QAction::triggered, this, &MainWindow::onTableDeleteRow);
    connect(ui.actionTableMoveRowAbove, &QAction::triggered, this, &MainWindow::onTableMoveRowAbove);
    connect(ui.actionTableMoveRowBelow, &QAction::triggered, this, &MainWindow::onTableMoveRowBelow);

	
	connect(ui.actionParaMarkList,		&QAction::triggered, this, &MainWindow::onInsertBulList);
	connect(ui.actionParaNumList,		&QAction::triggered, this, &MainWindow::onInsertNumList);

	connect(ui.actionQuickStart,		&QAction::triggered, this, &MainWindow::onProjectQuickStart);
	connect(ui.actionProjectNew,		&QAction::triggered, this, &MainWindow::onProjectNew);
	connect(ui.actionProjectSave,		&QAction::triggered, this, &MainWindow::onProjectSave);
//	connect(ui.actionProjectOpen,		&QAction::triggered, this, &MainWindow::onProjectOpen);
	connect(ui.actionFileReload,		&QAction::triggered, this, &MainWindow::onFileReload);
	connect(ui.actionFileSave,			&QAction::triggered, this, &MainWindow::onFileSave);
	connect(ui.actionFileSaveAll,		&QAction::triggered, this, &MainWindow::onFileSaveAll);
	connect(ui.actionProjectProperties,	&QAction::triggered, this, &MainWindow::onProjectProperties);
    connect(ui.actionProjectStatistics,	&QAction::triggered, this, &MainWindow::onProjectStatistics);
	connect(ui.actionProjectPrinfPdf,	&QAction::triggered, this, &MainWindow::onProjectPrintPdfBundle);
	connect(ui.actionExportPdfFiles,	&QAction::triggered, this, &MainWindow::onProjectPrintPdfFiles);

	connect(ui.actionGenContents1,      &QAction::triggered, this, [this](){ GenContents(0); });
	connect(ui.actionGenContents2,      &QAction::triggered, this, [this](){ GenContents(1); });
	connect(ui.actionEditCss1,          &QAction::triggered, this, [this](){ EditCss(0); });
	connect(ui.actionEditCss2,          &QAction::triggered, this, [this](){ EditCss(1); });
	connect(ui.actionCryptBase,         &QAction::triggered, this, &MainWindow::onProjectEncrypt);
	
	connect(ui.actionParaH1,			&QAction::triggered, this, &MainWindow::onParaH1);
	connect(ui.actionParaH2,			&QAction::triggered, this, &MainWindow::onParaH2);
	connect(ui.actionParaH3,			&QAction::triggered, this, &MainWindow::onParaH3);
	connect(ui.actionParaH4,			&QAction::triggered, this, &MainWindow::onParaH4);
	connect(ui.actionParaH5,			&QAction::triggered, this, &MainWindow::onParaH5);
	connect(ui.actionParaH6,			&QAction::triggered, this, &MainWindow::onParaH6);
	connect(ui.actionParaDiv,           &QAction::triggered, this, &MainWindow::onParaDiv);
	connect(ui.actionParaPara,			&QAction::triggered, this, &MainWindow::onParaPara);
	connect(ui.actionParaComment,		&QAction::triggered, this, &MainWindow::onParaComment);
	connect(ui.actionParaSource,		&QAction::triggered, this, &MainWindow::onParaSource);
	connect(ui.actionParaQuestion,		&QAction::triggered, this, &MainWindow::onParaQuestion);
	connect(ui.actionParaImportant,		&QAction::triggered, this, &MainWindow::onParaImportant);
	connect(ui.actionParaFeature,		&QAction::triggered, this, &MainWindow::onParaFeature);
	connect(ui.actionParaQuote,			&QAction::triggered, this, &MainWindow::onParaQuote);
	connect(ui.actionParaAnn,			&QAction::triggered, this, &MainWindow::onParaAnn);
	connect(ui.actionParaTerm,			&QAction::triggered, this, &MainWindow::onParaTerm);
	connect(ui.actionParaNote,			&QAction::triggered, this, &MainWindow::onParaNote);

	connect(ui.actionAppExit,			&QAction::triggered, this, &MainWindow::onAppExit);
	connect(ui.actionAppForceExit,		&QAction::triggered, this, &MainWindow::onAppForceExit);
	
    connect(ui.actionTreeSync,			&QAction::triggered, this, &MainWindow::onTreeSync);
	connect(ui.actionToolsLink,         &QAction::triggered, this, &MainWindow::onToolsLink);
	connect(ui.actionToolsSearch,		&QAction::triggered, this, &MainWindow::onToolsSearch);
	connect(ui.actionToolsTranslate,	&QAction::triggered, this, &MainWindow::onToolsTranslate);

	QAction *viewsAction = createPopupMenu()->menuAction();
	viewsAction->setText(tr("Toolbars & panels"));
	ui.viewMenu->addAction(viewsAction);

	connect(ui.actionToolsSnippets, &QAction::triggered, this, &MainWindow::onToolsSnippets);
	connect(ui.actionToolsOptions,  &QAction::triggered, this, &MainWindow::onToolsOptions);
    connect(ui.actionEditConfig,	&QAction::triggered, this, &MainWindow::onToolsEditConfig);
	connect(ui.actionEditScript,    &QAction::triggered, this, &MainWindow::onToolsEditScript);
	connect(ui.actionReloadScript,  &QAction::triggered, this, &MainWindow::onToolsReloadScript);	

	m_wSln->initialize();
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
		m_wSln->LoadTree();
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

void MainWindow::onToolsLink()
{
	// following a link
    WebEditView *wnd = GetActiveMdiChild();
    if(wnd) {
		wnd->onToolsLink();
    }
}

void MainWindow::onToolsSearch()
{
	// following a link
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd) {
		wnd->onToolsSearch();
	}
}

void MainWindow::onToolsTranslate()
{
	// following a link
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd) {
		wnd->onToolsTranslate();
	}
}
//-------------------------------------------------
void MainWindow::onProjectNew()
{
	NewProjectDlg dlg;
	if(dlg.DoModal() == QDialog::Accepted)
	{
		theSln.CreateProject(dlg.m_name, dlg.m_base, dlg.m_suffixes[0], dlg.m_suffixes[0]);
		m_wSln->LoadTree();
		UpdateTitle();
	}
}

void MainWindow::onProjectSave()
{
	if(QMessageBox::Yes == QMessageBox::question(this, "Save all vmbases?", "Save all vmbases?")) {
		theSln.SaveProject(true);
	}
}

void MainWindow::onFileSave()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->OnFileSave();
}

void MainWindow::onFileReload()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->ReloadHtml();
}

void MainWindow::onEditFixCssPath()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->FixCssPath();
}

void MainWindow::onFileSaveAll()
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
	dlg.m_bases[0] = theSln.m_Books.books[0];
	dlg.m_bases[1] = theSln.m_Books.books[1];
		
	if(dlg.DoModal() == QDialog::Accepted)
	{
		// global paths
        theSln.m_ImageDir = dlg.m_images;
		theSln.m_Snippets.m_SnippDir = dlg.m_snippets;
		// bases
		int cnt = std::max(1, !dlg.m_bases[0].suffix.isEmpty() + !dlg.m_bases[1].suffix.isEmpty());
		theSln.m_Books.booksCnt = cnt;
		for (int bi = 0; bi < BCNT; bi++) {
			theSln.m_Books.books[bi] = dlg.m_bases[bi];
			if (theSln.m_Books.books[bi].load_prefix != theSln.m_Books.books[bi].save_prefix) {
				setCursor(Qt::WaitCursor);
				theSln.TransformDocs(bi);
				setCursor(Qt::ArrowCursor);
			}
		}
		m_wSln->UpdateBases();
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
	connect(tbtnInsertTable, SIGNAL(clicked()), this, SLOT(onInsertTableClicked()));
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
	m_wSln->LoadTree();
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

void MainWindow::onZoomIn()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onZoomIn();
}

void MainWindow::onZoomOut()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onZoomOut();
}

void MainWindow::onZoomNormal()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onZoomChange(100);
}

void MainWindow::onZoomChange(int percent)
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onZoomChange(percent);
}

void MainWindow::onEditCut()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditCut();
}

void MainWindow::onEditCutText()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditCutText();
}

void MainWindow::onEditCopy()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditCopy();
}

void MainWindow::onEditCopyText()
{
    WebEditView *wnd = GetActiveMdiChild();
    if(wnd)
        wnd->onEditCopyText();
}

void MainWindow::onEditPaste()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditPaste();
}

void MainWindow::onEditPasteText()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onEditPasteText();
}

void MainWindow::onEditPasteImage()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onEditPasteImage();
}

void MainWindow::onEditPasteAsTable()
{
    WebEditView *wnd = GetActiveMdiChild();
    if (wnd)
        wnd->onEditPasteAsTable();
}

void MainWindow::onEditPasteCell()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onEditPasteCell();
}

void MainWindow::onEditUndo()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditUndo();
}

void MainWindow::onEditRedo()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditRedo();
}

void MainWindow::onEditUntag()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditUntag();
}

void MainWindow::onEditOutside()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditOutside();
}

void MainWindow::onEditTagInfo()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditTagInfo();
}

void MainWindow::onEditUntable()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditUntable();

}
//////////////////////////////////////////////////////////////////////////
// text 
void MainWindow::onTextBold()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextBold();
}
void MainWindow::onTextItalic()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextItalic();
}
void MainWindow::onTextUnderline()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextUnderline();
}
void MainWindow::onTextStrike()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextStrike();
}
void MainWindow::onTextSubscript()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextSubscript();
}
void MainWindow::onTextSuperscript()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextSuperscript();
}
void MainWindow::onTextCode()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextCode();
}
void MainWindow::onTextMark()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextMark();
}
void MainWindow::onTextMark1()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark1();
}
void MainWindow::onTextMark2()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark2();
}
void MainWindow::onTextMark3()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark3();
}
void MainWindow::onTextMark4()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark4();
}
void MainWindow::onTextMark5()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark5();
}
void MainWindow::onTextMark6()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark6();
}
void MainWindow::onTextMark7()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTextMark7();
}

void MainWindow::onTextStrong()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextStrong();
}
void MainWindow::onTextEm()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextEm();
}
void MainWindow::onTextIns()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextIns();
}
void MainWindow::onTextDel()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextDel();
}
void MainWindow::onTextSamp()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextSamp();
}
void MainWindow::onTextVar()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextVar();
}
void MainWindow::onTextKbd()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onTextKbd();
}

void MainWindow::onEditClearDoc()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onEditClearDoc();
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

void MainWindow::onInsertTableClicked()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertTable();
}

void MainWindow::onInsertHorzLine()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertHorzLine();
}
void MainWindow::onInsertImage()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertImage();
}
void MainWindow::onInsertHyperlink()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertHyperlink();
}
void MainWindow::onInsertDateTime()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertDateTime();
}
void MainWindow::onInsertSnippet()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertSnippet();
}
void MainWindow::onInsertSymbol()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertSymbol();
}

void MainWindow::onInsertNumList()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertNumList();
}

void MainWindow::onInsertBulList()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onInsertBulList();
}

void MainWindow::onParaH1()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
		wnd->onParaHeading1();
}
void MainWindow::onParaH2()
{
	WebEditView *wnd = GetActiveMdiChild();
	if(wnd)
wnd->onParaHeading2();
}
void MainWindow::onParaH3()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaHeading3();
}
void MainWindow::onParaH4()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaHeading4();
}
void MainWindow::onParaH5()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaHeading5();
}
void MainWindow::onParaH6()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaHeading6();
}

void MainWindow::onParaDiv()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaDiv();
}
void MainWindow::onParaPara()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaPara();
}
void MainWindow::onParaComment()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaComment();
}
void MainWindow::onParaSource()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaSource();
}
void MainWindow::onParaQuestion()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaQuestion();
}
void MainWindow::onParaImportant()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaImportant();
}
void MainWindow::onParaFeature()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaFeature();
}
void MainWindow::onParaQuote()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaQuote();
}
void MainWindow::onParaAnn()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaAnn();
}
void MainWindow::onParaTerm()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaTerm();
}
void MainWindow::onParaNote()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onParaNote();
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
    theSln.SaveSettings();
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
        theSln.LoadSettings();
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
		
	for(MTPOS tchild : tpos->children)
	{
		MakePagesListForPdfPrinting(tchild->This<DocItem>(), level + 1, page, args, toc);
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

void* MainWindow::FindOpenedDoc(MTPOS pos, int di)
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

void MainWindow::onTableAppendData()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTableAppendData();
}

void MainWindow::onTableInsertData()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTablePasteData();
}

void MainWindow::onTableExpand()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTableExpand();
}

void MainWindow::onTableCollapse()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTableCollapse();
}

void MainWindow::onTableDeleteRow()
{
	WebEditView *wnd = GetActiveMdiChild();
	if (wnd)
		wnd->onTableDelRow();
}

void MainWindow::onTableMoveRowAbove()
{
    WebEditView *wnd = GetActiveMdiChild();
    if (wnd)
        wnd->onTableMoveRowAbove();
}

void MainWindow::onTableMoveRowBelow()
{
    WebEditView *wnd = GetActiveMdiChild();
    if (wnd)
        wnd->onTableMoveRowBelow();
}

