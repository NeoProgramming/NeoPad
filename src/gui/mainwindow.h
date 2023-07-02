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
	void  UpdateTab(DocItem* tpos);
    void  CloseTab(DocItem* tpos, bool clear_modify);
    void  CloseTabs(DocItem* tpos, bool clear_modify);
    qreal UpdateZoom(int percent);
	void  OpenLocalLink(const QString &url, int di);
	void saveSettings();
	void loadSettings();
	void loadScripts();
	void projectModified(bool modified);
    void Search(const QString &text);
	MainWindow *newWindow();
    void setStatus(const QString &str);
public:
	void* FindOpenedDoc(DocItem* pos, int di) override;
	void  GetDocData(void* wnd, QString &html) override;
	
public slots:

	void onProjectSaveAll();
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

	void onZoomChange(int);

	void onInsertTableChanged(int cols, int rows);
		    
	void onAppAbout();
	void onAppAboutQt();
	void onAppExit();
	void onAppForceExit();

    void onTreeSync();
	
protected:
    void closeEvent(QCloseEvent *);
    
private:
	void QuitProject();
	void OpenTabs();
	void SaveTabs();
	WebEditView *GetActiveMdiChild();
    void onChild(void (WebEditView::*pHandler)());
	void GenContents(int bi);
	void EditCss(int bi);
	bool DoSaveAll();
	void DoQuickStart(int code);
	bool DoPrjOpen(const QString& fpath);
	void DoOpenDoc(DocItem* mtPos, int di);
	bool DoSelectDoc(DocItem* tpos, int di);
	void OpenDoc(DocItem* mtPos, int di);
	void LoadToCurrentDoc(DocItem* mtPos, int di);
	void CreateNewDoc(DocItem* mtPos, int di);
	bool OpenExistingDoc(DocItem* mtPos, int di);
	QMdiSubWindow * FindTab(DocItem* mtPos, int di);
	void MakePagesListForPdfPrinting(DocItem* mtPos, int level, int &page, QStringList &args, QString &toc);
	void UpdateTitle();
	void createSpecialToolWidgets();
	
	QToolButton* createMenuButton(const char *res, QMenu *menu, const QString &tooltip);
	QMenu* createTableMenu(const char *slot);
private:	
	QMdiArea      *m_wArea;
	QTabBar       *m_tabBar;			// extract from m_wArea
	SlnPanel	  *m_wSln;
	QDockWidget   *m_wDock;
	QToolButton	  *tbtnInsertTable;	// table picker
};

#endif // MAINWINDOW_H
