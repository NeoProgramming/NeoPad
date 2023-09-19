#ifndef WEBEDITVIEW_H
#define WEBEDITVIEW_H

#include <QtWidgets>
#include <QtGui>
#include <QtWebKit>
#include <QWebView>
#include "../core/DocItem.h"

class MainWindow;

class WebEditView : public QWebView
{
	Q_OBJECT

public:
	WebEditView(MainWindow *mw, DocItem* tpos, int di);
	~WebEditView();
	DocItem* m_Item;
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

	void onFileSave();
	void onFileSaveAs();
	void onFileReload();
		
	void onEditCut();
	void onEditCutText();
	void onEditCopy();
    void onEditCopyText();
	void onEditPaste();
	void onEditPasteText();
    void onEditPasteAsTable();
	void onEditPasteCell();
	void onEditPasteSpecial();

	void onEditUntag();
	void onEditOutside();
	void onEditTagInfo();
	void onEditUntable();
	bool onEditDeflist();
	void onEditClearDoc();
	void onEditFixCssPath();
	void onEditUndo();
	void onEditRedo();

	void onTextBold();
	void onTextItalic();
	void onTextUnderline();
	void onTextStrike();
	void onTextSubscript();
	void onTextSuperscript();
	void onTextCode();
	void onTextStrong();
	void onTextEm();
	void onTextIns();
	void onTextDel();
	void onTextSamp();
	void onTextVar();
	void onTextKbd();

	void onTextMark();
	void onTextMark1();
	void onTextMark2();
	void onTextMark3();
	void onTextMark4();
	void onTextMark5();
	void onTextMark6();
	void onTextMark7();

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
	void onZoomNormal();

	void onZoomChange(int);

	void onAdjustSource();

	void onTableAppendData();
	void onTableProperties();
	void onTableInsAbove();
	void onTableInsBelow();
	void onTableNormalizeRow();
	void onTableRemoveEmptyRows();
	void onTableInsLeft();
	void onTableInsRight();
	void onTableDeleteColumn();
	void onTableDeleteRow();
	void onTableInsertData();
	void onTableExpand();
	void onTableCollapse();
    void onTableMoveRowAbove();
    void onTableMoveRowBelow();
	
	void onImageProperties();

	void onLinkProperties();
	
	void onLinkClicked(const QUrl & url);
	void onToolsLink();
	void onToolsSearch();
	void onToolsTranslate();	
    
public:
	bool LoadHtml(DocItem* tpos, int di);
	bool SaveHtml(bool update_tree);
	void Find(const QString &text, bool backward);
protected:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void closeEvent(QCloseEvent *e);
	virtual void contextMenuEvent (QContextMenuEvent * event);
	
private:
	QWebElement getTag();
	bool maybeSave();
	void execCommand(const QString&);
	void execCommand(const QString &cmd, const QString &arg);
//	bool queryCommandState(const QString&);
	bool execScript(const QString &jsfn, QString *outRes = nullptr);
	int  GetHitContext(QWebHitTestResult &r);
	int  GetElementContext(const QWebElement &el);
	bool GetCaretContext();
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
	QAction *actionTableOptTable;
	QAction *actionTableInsLeft;
	QAction *actionTableInsRight;
	QAction *actionTableDelRow;
	QAction *actionTableDelCol;
	QAction *actionTablePasteData;
	QAction *actionImageProps;
	QAction *actionLinkProps;
    
	QWebElement m_elTable;
	QWebElement m_elTR;
	QWebElement m_elTD;
	QWebElement m_elImage;
	QWebElement m_elLink;
};

#endif // WEBEDITVIEW_H
