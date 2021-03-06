#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "webeditview.h"

#include <QPointer>
#include <QMap>
#include <QTreeWidget>
#include <QFontComboBox>
#include <QSlider>
#include <QMdiArea>

#include "../core/Solution.h"

class SlnPanel;
class QMenu;
class QDockWidget;

class MainWindow : public QMainWindow, public NeopadCallback
{
    Q_OBJECT
	friend class SlnPanel;
public:
    MainWindow();
    virtual ~MainWindow();
	Ui::MainWindow ui;

	QIcon        m_iconHtm;	
	QIcon        m_iconVmb;
	QMenu        m_menuText;	// editor context menu
	QMenu        m_menuImage;
	QMenu        m_menuTable;
	bool		 m_doCommit;
	QString      m_js;			// working script
	QDateTime	 m_jsTime;		// script modification time
	QString		 m_jsPath;

	SlnPanel *getSln() const;
	void  UpdateTab(MTPOS tpos);
    qreal UpdateZoom(int percent);
	void  OpenLocalLink(const QString &url, int di);
	void saveSettings();
	void loadSettings();
	void loadScripts();
	void projectModified(bool modified);
	
	MainWindow *newWindow();
public:
	void* FindOpenedDoc(MTPOS pos, int di) override;
	void  GetDocData(void* wnd, QString &html) override;
	
public slots:

	void onFileReload();
	void onFileSave();
	void onFileSaveAll();
	
    void onProjectOpen(QString fileName);
    void onProjectQuickStart();
	void onProjectNew();
	void onProjectSave();
	void onProjectProperties();
    void onProjectStatistics();
	void onProjectPrintPdfBundle();
	void onProjectPrintPdfFiles();
	void onProjectEncrypt();
		
	void onToolsSnippets();
	void onToolsOptions();
	void onToolsEditConfig();
	void onToolsEditScript();
	void onToolsReloadScript();

private slots:
	void onPostInit();

	void onTileSubWindowsHorizontally();
	void onTileSubWindowsVertically();
	
	void onZoomIn();
	void onZoomOut();
	void onZoomNormal();
	void onZoomChange(int);

	void onEditClearDoc();
	void onEditFixCssPath();
	void onEditCut();
	void onEditCutText();
	void onEditCopy();
    void onEditCopyText();
	void onEditPaste();
	void onEditPasteText();
	void onEditUndo();
	void onEditRedo();
	void onEditUntag();
	void onEditOutside();
	void onEditTagInfo();
	void onEditUntable();

	void onTextBold();
	void onTextItalic();
	void onTextUnderline();
	void onTextStrike();
	void onTextSubscript();
	void onTextSuperscript();
	void onTextCode();
	void onTextMark();
	void onTextStrong();
	void onTextEm();
	void onTextIns();
	void onTextDel();
	void onTextSamp();
	void onTextVar();
	void onTextKbd();

	void onParaH1();
	void onParaH2();
	void onParaH3();
	void onParaH4();
	void onParaH5();
	void onParaH6();
	void onParaDiv();
	void onParaPara();
	void onParaComment();
	void onParaSource();
	void onParaQuestion();
	void onParaImportant();
	void onParaFeature();
	void onParaQuote();
	void onParaAnn();
	void onParaTerm();
	void onParaNote();

	void onInsertTableChanged(int cols, int rows);
	void onInsertTableClicked();
	void onInsertHorzLine();
	void onInsertImage();
	void onInsertHyperlink();
	void onInsertDateTime();
	void onInsertSnippet();
	void onInsertSymbol();
	void onInsertNumList();
	void onInsertBulList();
	    
	void onAppAbout();
	void onAppAboutQt();
	void onAppExit();
	void onAppForceExit();

    void onTreeSync();
	void onLinkFollow();
protected:
    void closeEvent(QCloseEvent *);
    
private:
	WebEditView *GetActiveMdiChild();

	void GenContents(int bi);
	void EditCss(int bi);
	bool DoSaveAll();
	void DoQuickStart(int code);
	bool DoPrjOpen(const QString& fpath);
	void DoOpenDoc(MTPOS mtPos, int di);
	bool DoSelectDoc(MTPOS tpos, int di);

	void OpenDoc(MTPOS mtPos, int di);
	void LoadToCurrentDoc(MTPOS mtPos, int di);
	void CreateNewDoc(MTPOS mtPos, int di);
	bool OpenExistingDoc(MTPOS mtPos, int di);
	QMdiSubWindow * FindTab(MTPOS mtPos, int di);
	void MakePagesListForPdfPrinting(MTPOS mtPos, int level, int &page, QStringList &args, QString &toc);
	void UpdateTitle();
	void createSpecialToolWidgets();
	
	QToolButton* createMenuButton(const char *res, QMenu *menu, const QString &tooltip);
	QMenu* createTableMenu(const char *slot);
private:	
	QMdiArea      *m_wArea;
	QTabBar       *m_tabBar;
	QSignalMapper *windowMapper;
	
	SlnPanel	  *m_wSln;
	QDockWidget   *m_wDock;

	QToolButton	  *tbtnInsertTable;	// table picker
};

#endif // MAINWINDOW_H
