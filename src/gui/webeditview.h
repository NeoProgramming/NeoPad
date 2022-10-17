#ifndef WEBEDITVIEW_H
#define WEBEDITVIEW_H

#include <QtWidgets>
#include <QtGui>
#include <QtWebKit>
#include <QWebView>
#include "../core/PrjItem.h"

class MainWindow;

class WebEditView : public QWebView
{
	Q_OBJECT

public:
	WebEditView(MainWindow *mw, MTPOS tpos, int di);
	~WebEditView();
	MTPOS m_Item;
	int   m_di;
private:
    static QString jsGetSelBounds();
    static QString jsSetSelAttr();
	static QString jsRemoveSelAttr();
    static QString jsDoClearClass();
	static QString jsDoSetClass(const QString &s);
	static QString jsDoClearStyle();
	static QString jsDoSetStyle(const QString &s);
	static QString jsDoClearTags();
public slots:

	bool OnFileSave();
		
	void onEditCut();
	void onEditCutText();
	void onEditCopy();
    void onEditCopyText();
	void onEditPaste();
	void onEditPasteText();
    void onEditPasteTable();
	void onEditPasteSpecial();

	void onEditUntag();
	void onEditOutside();
	void onEditTagInfo();
	void onEditUntable();
	bool onEditDeflist();
	void onEditClearDoc();
	void onEditUndo();
	void onEditRedo();

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

	void onParaHeading1();
	void onParaHeading2();
	void onParaHeading3();
	void onParaHeading4();
	void onParaHeading5();
	void onParaHeading6();
	void onParaDiv();
	void onParaPara();
	void onParaComment();
	void onParaQuestion();
	void onParaImportant();
	void onParaFeature();
	void onParaQuote();
	void onParaSource();
	void onParaAnn();
	void onParaTerm();
	void onParaNote();
	
	void onInsertHorzLine();
	void onInsertTable();
	void onInsertTable(int cols, int rows);
	void onInsertImage();
	void onInsertHyperlink();
	void onInsertDateTime();
	void onInsertSnippet();
	void onInsertSymbol();
	void onInsertNumList();
	void onInsertBulList();

	void onZoomOut();
	void onZoomIn();
	void onZoomChange(int);

	void onAdjustSource();

	void onTableProperties();
	void onTableInsAbove();
	void onTableInsBelow();
	void onTableNormalizeRow();
	void onTableInsLeft();
	void onTableInsRight();
	void onTableDelColumn();
	void onTableDelRow();

	void onImageProperties();

	void onLinkProperties();
	
	void onLinkClicked(const QUrl & url);
	void onToolsLink();
	void onToolsSearch();
	void onToolsTranslate();	
    
public:
	bool LoadHtml(MTPOS tpos, int di);
	bool ReloadHtml();
	void FixCssPath();
	bool SaveHtml(bool update_tree);
	void Find(const QString &text, bool backward);
protected:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void closeEvent(QCloseEvent *e);
	virtual void contextMenuEvent ( QContextMenuEvent * event );

private:
	bool maybeSave();
	void execCommand(const QString&);
	void execCommand(const QString &cmd, const QString &arg);
//	bool queryCommandState(const QString&);
	bool execScript(const QString &jsfn, QString *outRes = nullptr);
	int  GetHitContext(QWebHitTestResult &r);
	int  GetElementContext(const QWebElement &el);
	void OpenLink(const QUrl&);
	void InsertHtml(QString html);
	QString PrepareImage(ImageAction action, const QString &fpath);
	void InsertImage(const QString &fpath, int w, int h);

private:
	
	MainWindow *m_wMain;
	QMenu m_menuContext;
	QMenu m_menuTable;
	QAction *actionTableProps;
	QAction *actionTableInsAbove;
	QAction *actionTableInsBelow;
	QAction *actionTableNormRow;
	QAction *actionTableInsLeft;
	QAction *actionTableInsRight;
	QAction *actionTableDelRow;
	QAction *actionTableDelCol;
	QAction *actionImageProps;
	QAction *actionLinkProps;
    
	QWebElement m_elTable;
	QWebElement m_elTR;
	QWebElement m_elTD;
	QWebElement m_elImage;
	QWebElement m_elLink;
};

#endif // WEBEDITVIEW_H
