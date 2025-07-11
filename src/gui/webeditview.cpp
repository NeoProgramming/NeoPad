#include <QtCore>
#include <QtGui>
#include <QWebFrame>
#include <QRegularExpression>
#include "webeditview.h"
#include "mainwindow.h"
#include "slnpanel.h"

#include "tableproperty.h"
#include "imageproperty.h"
#include "linkproperty.h"
#include "snippetsdlg.h"
#include "symbolsdlg.h"
#include "html.h"
#include "htmltable.h"
#include "htmlimage.h"
#include "htmllink.h"

#include "../service/tools.h"
#include "../core/vmbsrv.h"
#include "../core/ini.h"
#include "../core/Cryptor.h"

extern QTextCodec *codecUtf8;

enum HtmlCtx {
	CTX_TABLE = 0x1,
	CTX_TR = 0x2,
	CTX_TD = 0x4,
	CTX_IMAGE = 0x8,
	CTX_LINK = 0x10
};

WebEditView::WebEditView(MainWindow *mw, DocItem* tpos, int di)
	: QWebView()
{
	setAttribute(Qt::WA_DeleteOnClose);
	m_wMain = mw;
	m_Item = tpos;
	m_di = di;
	page()->setContentEditable(true);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	
	page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	// necessary to sync our actions
	connect(page(), SIGNAL(contentsChanged()),  SLOT(onAdjustSource()));

	// actions
	actionTableProps = new QAction(tr("Table properties"), 0);
	actionImageProps = new QAction(tr("Image properties"), 0);
	actionLinkProps  = new QAction(tr("Link properties"), 0);

	connect(actionTableProps, SIGNAL(triggered()), this, SLOT(onTableProperties()));
	connect(actionImageProps, SIGNAL(triggered()), this, SLOT(onImageProperties()));
	connect(actionLinkProps, SIGNAL(triggered()), this, SLOT(onLinkProperties()));

	actionTableInsAbove = new QAction(tr("Insert row above"), 0);
	actionTableInsBelow = new QAction(tr("Insert row below"), 0);
	actionTableNormRow  = new QAction(tr("Normalize row"), 0);
	actionTableOptTable = new QAction(tr("Remove empty rows"), 0);
	actionTableInsLeft  = new QAction(tr("Insert column left"), 0);
	actionTableInsRight = new QAction(tr("Insert column right"), 0);
	actionTableDelRow   = new QAction(tr("Delete row"), 0);
	actionTableDelCol   = new QAction(tr("Delete column"), 0);
	actionTablePasteData= new QAction(tr("Paste Data"), 0);
	actionTableClearCol = new QAction(tr("Clear column"), 0);
	actionTableClearRow = new QAction(tr("Clear row"), 0);

	connect(actionTableInsAbove, &QAction::triggered, this, &WebEditView::onTableInsAbove);
	connect(actionTableInsBelow, &QAction::triggered, this, &WebEditView::onTableInsBelow);
	connect(actionTableNormRow, &QAction::triggered, this, &WebEditView::onTableNormalizeRow);
	connect(actionTableOptTable, &QAction::triggered, this, &WebEditView::onTableRemoveEmptyRows);
	connect(actionTableInsLeft, &QAction::triggered, this, &WebEditView::onTableInsLeft);
	connect(actionTableInsRight, &QAction::triggered, this, &WebEditView::onTableInsRight);
	connect(actionTableClearRow, &QAction::triggered, this, &WebEditView::onTableClearRow);
	connect(actionTableClearCol, &QAction::triggered, this, &WebEditView::onTableClearColumn);
	connect(actionTableDelRow, &QAction::triggered, this, &WebEditView::onTableDeleteRow);
	connect(actionTableDelCol, &QAction::triggered, this, &WebEditView::onTableDeleteColumn);
	connect(actionTablePasteData, &QAction::triggered, this, &WebEditView::onTableInsertData);
	
	// table menu
	m_menuTable.setTitle("Table");
	m_menuTable.addAction(actionTableInsAbove);
	m_menuTable.addAction(actionTableInsBelow);
	m_menuTable.addAction(actionTableNormRow);
	m_menuTable.addAction(actionTableOptTable);
	m_menuTable.addAction(actionTableInsLeft);
	m_menuTable.addAction(actionTableInsRight);
	m_menuTable.addSeparator();
	m_menuTable.addAction(actionTablePasteData);
	m_menuTable.addAction(actionTableClearCol);
	m_menuTable.addAction(actionTableClearRow);
	m_menuTable.addSeparator();
	m_menuTable.addAction(actionTableDelRow);
	m_menuTable.addAction(actionTableDelCol);

	// paste menu
	m_menuPaste.setTitle("Special Paste");
	m_menuPaste.addAction(mw->ui.actionEditPasteAsTable);
	m_menuPaste.addAction(mw->ui.actionEditPasteAsCode);
	m_menuPaste.addAction(mw->ui.actionEditPasteAsBilingua);
	m_menuPaste.addAction(mw->ui.actionEditPasteAsLWText);

	// context menu
	m_menuContext.addAction(mw->ui.actionEditCut);
	m_menuContext.addAction(mw->ui.actionEditCutText);
	m_menuContext.addAction(mw->ui.actionEditCopy);
    m_menuContext.addAction(mw->ui.actionEditCopyText);
	m_menuContext.addAction(mw->ui.actionEditPaste);
	m_menuContext.addAction(mw->ui.actionEditPasteText);
    m_menuContext.addMenu( &m_menuPaste );
	m_menuContext.addSeparator();
	m_menuContext.addAction(mw->ui.actionToolsLink);
	m_menuContext.addAction(mw->ui.actionToolsSearch);
	m_menuContext.addAction(mw->ui.actionToolsTranslate);
	m_menuContext.addSeparator();
	m_menuContext.addAction(actionTableProps);
	m_menuContext.addMenu( &m_menuTable );
	m_menuContext.addAction(actionImageProps);
	m_menuContext.addAction(actionLinkProps);
}

WebEditView::~WebEditView()
{

}

void WebEditView::onAdjustSource()
{
	// source change
	setWindowModified(true);
}

bool WebEditView::maybeSave()
{
	if (m_DontSaveBeforeClosing || !isWindowModified())
		return true;

	QMessageBox::StandardButton ret;
	ret = QMessageBox::warning(this, tr("HTML Editor"),
		tr("The document has been modified.\n"
		"Do you want to save your changes?"),
        QMessageBox::Save | QMessageBox::No
		| QMessageBox::Cancel);
	if (ret == QMessageBox::Save)
		return SaveHtml(true);
	else if (ret == QMessageBox::Cancel)
		return false;
	return true;
}

bool WebEditView::SaveHtml(bool update_tree)
{
	QString content = this->page()->mainFrame()->toHtml();
	bool success = theSln.SaveDoc(m_Item, m_di, content);
	if (success) 
		setWindowModified(false);
	
    // update language status
    if(update_tree) {
        m_wMain->getSln()->UpdateDocItemsByObj(m_Item);
    }
	return success;
}

void WebEditView::onFileSave()
{
	SaveHtml(true);
}

void WebEditView::onFileSaveAs()
{
	QString fpath = QFileDialog::getSaveFileName(this, tr("Save File As..."),
		m_Item->GetId(),
		tr("Documents (*.htm *.html);;All Files (*)"));
	if (!fpath.isEmpty())
	{
		QString content = this->page()->mainFrame()->toHtml();
		QByteArray s = content.toUtf8();
		QFile file(fpath);
		if (file.open(QIODevice::WriteOnly))
			file.write(s);
	}
}

void WebEditView::onZoomOut()
{
	int percent = static_cast<int>(this->zoomFactor() * 100);
	if (percent > 25) 
	{
		percent -= 25;
		percent = 25 * (int((percent + 25 - 1) / 25));
		qreal factor = static_cast<qreal>(percent) / 100;
		this->setZoomFactor(factor);

		m_wMain->UpdateZoom(percent);
	}
}

void WebEditView::onZoomIn()
{
	int percent = static_cast<int>(this->zoomFactor() * 100);
	if (percent < 400) 
	{
		percent += 25;
		percent = 25 * (int(percent / 25));
		qreal factor = static_cast<qreal>(percent) / 100;
		this->setZoomFactor(factor);

		m_wMain->UpdateZoom(percent);
	}
}

void WebEditView::onZoomNormal()
{
	onZoomChange(100);
}

void WebEditView::execCommand(const QString &cmd)
{
	QWebFrame *frame = this->page()->mainFrame();
	QString js = QString("document.execCommand(\"%1\", false, null)").arg(cmd);
	frame->evaluateJavaScript(js);
}

void WebEditView::execCommand(const QString &cmd, const QString &arg)
{
	QWebFrame *frame = this->page()->mainFrame();
	QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg);
	frame->evaluateJavaScript(js);
}

/*
bool WebEditView::queryCommandState(const QString &cmd)
{
	QWebFrame *frame = this->page()->mainFrame();
	QString js = QString("document.queryCommandState(\"%1\", false, null)").arg(cmd);
	QVariant result = frame->evaluateJavaScript(js);
	return result.toString().simplified().toLower() == "true";
}*/

bool WebEditView::execScript(const QString &jsfn, QString *outRes)
{
	QWebFrame *frame = this->page()->mainFrame();
	QVariant result = frame->evaluateJavaScript(m_wMain->m_js + " " + jsfn);

	QVariant::Type t = result.type();
	if(t == QVariant::Bool) {
		bool r = result.toBool();
		if(r)
			setWindowModified(true);
		return r;
	}
	else {
		if(outRes)
			*outRes = result.toString();
		else
			QMessageBox::information(this, "Script", result.toString());
	}
	return false;
}

void WebEditView::OpenLink(const QUrl &url)
{
	QString msg = QString(tr("Open %1 ?")).arg(url.toString());
	if (QMessageBox::question(this, tr("Open link"), msg,
		QMessageBox::Open | QMessageBox::Cancel) ==
		QMessageBox::Open)
		QDesktopServices::openUrl(url);
}

void WebEditView::onZoomChange(int percent)
{
	m_wMain->ui.actionZoomOut->setEnabled(percent > 25);
	m_wMain->ui.actionZoomIn->setEnabled(percent < 400);
	qreal factor = static_cast<qreal>(percent) / 100;
	this->setZoomFactor(factor);
}

void WebEditView::keyPressEvent(QKeyEvent * e)
{
	// this is how our own hotkeys are implemented
	int	key = e->key();
	Qt::KeyboardModifiers m = e->modifiers();
	if ((key == Qt::Key_Return) && (m & Qt::ControlModifier))
		onEditOutside();
	else if ((key == Qt::Key_Return) && (m == 0)) 
		QWebView::keyPressEvent(e);
	
	else if ((key == Qt::Key_C) && (m & Qt::ControlModifier))
		onEditCopy();
	else if ((key == Qt::Key_Insert) && (m & Qt::ControlModifier))
		onEditCopy();
	else if ((key == Qt::Key_X) && (m & Qt::ControlModifier))
		onEditCut();
	else if ((key == Qt::Key_Delete) && (m & Qt::ShiftModifier))
		onEditCut();
	else if ((key == Qt::Key_V) && (m & Qt::ControlModifier))
		onEditPasteSpecial();
	else if ((key == Qt::Key_Insert) && (m & Qt::ShiftModifier))
		onEditPasteSpecial();
	
	else
		QWebView::keyPressEvent(e);
}

void WebEditView::closeEvent(QCloseEvent *e)
{
	if (maybeSave())
		e->accept();
	else
		e->ignore();
}

int WebEditView::GetElementContext(const QWebElement &elem)
{
	// go up the tree of elements, fill in m_el ***
	int ctx = 0;
    QWebElement el = elem;
	while(!el.isNull())
	{
		QString tag = el.localName();
		if(tag == "table")// || tag == "tr" || tag == "td" || tag == "th")
		{
			if(!(ctx & CTX_TABLE))
				m_elTable = el;
			ctx |= CTX_TABLE;
		}
		if (tag == "tr")
		{
			if (!(ctx & CTX_TR))
				m_elTR = el;
			ctx |= CTX_TR;
		}
		if (tag == "td")
		{
			if (!(ctx & CTX_TD))
				m_elTD = el;
			ctx |= CTX_TD;
		}
		if(tag == "img")
		{
			if(!(ctx & CTX_IMAGE))
				m_elImage = el;
			ctx |= CTX_IMAGE;
		}
		if(tag == "a")
		{
			if(!(ctx & CTX_LINK))
				m_elLink = el;
			ctx |= CTX_LINK;
		}
		el = el.parent();
	}
	return ctx;
}

int WebEditView::GetHitContext(QWebHitTestResult &hitTestResult)
{
 	// by the result of the hit test, determine where we are and which context menu to display
	// go up the tags
	m_elTable = QWebElement();
	m_elTR = QWebElement();
	m_elTD = QWebElement();
	m_elImage = QWebElement();
	m_elLink = QWebElement();

	int ctx = GetElementContext(hitTestResult.element()); 
	ctx |= GetElementContext(hitTestResult.enclosingBlockElement());
 	return ctx;
}

bool WebEditView::GetCaretContext()
{
	// get coordinates of text cursor
	
	QString res;
	QPoint pos;
	execScript("getCaretPosition();", &res); // "100;200"
	int i = res.indexOf(';');
	if (i < 0)
		return false;
	pos.setX(res.left(i).toDouble()+1);
	pos.setY(res.mid(i + 1).toDouble());

	QWebFrame *frame = page()->currentFrame();
	QWebHitTestResult hitTestResult = frame->hitTestContent(pos);
	GetHitContext(hitTestResult);
	return true;
}

void WebEditView::contextMenuEvent ( QContextMenuEvent * e )
{
	// open this to activate the menu with inspect
	//QWebView::contextMenuEvent(e);
	//return;

	QPoint pos = mapFromGlobal( e->globalPos() );
	QWebFrame *frame = page()->frameAt(e->pos());
	if (frame!=NULL)
	{
		QWebHitTestResult hitTestResult = frame->hitTestContent(pos);
		int ctx = GetHitContext(hitTestResult);
		actionTableProps->setEnabled(ctx & CTX_TABLE);
		actionImageProps->setEnabled(ctx & CTX_IMAGE);
		actionLinkProps->setEnabled(ctx & CTX_LINK);
		
		actionTableInsAbove->setEnabled(ctx & CTX_TABLE);
		actionTableInsBelow->setEnabled(ctx & CTX_TABLE);
		actionTableNormRow->setEnabled(ctx & CTX_TABLE);
		actionTableOptTable->setEnabled(ctx & CTX_TABLE);
		actionTableInsLeft->setEnabled(ctx & CTX_TABLE);
		actionTableInsRight->setEnabled(ctx & CTX_TABLE);
		actionTableDelRow->setEnabled(ctx & CTX_TABLE);
		actionTableDelCol->setEnabled(ctx & CTX_TABLE);
		actionTablePasteData->setEnabled(ctx & CTX_TABLE);
		
		m_menuContext.exec(e->globalPos());
	}
}

void WebEditView::onFileReload()
{
    if(!maybeSave())
        return;
	LoadHtml(m_Item, m_di);
}

void WebEditView::onEditFixCssPath()
{
	// find out the relative path of THIS file relative to the BASE, where the css is located
	QString cp = m_Item->GetCssRelPath(m_di);
	QString cmd = "setDocCSS('" + cp + "')";
	execScript(cmd);
}

bool WebEditView::LoadHtml(DocItem* tpos, int bi)
{
	QString content;

	this->settings()->setObjectCacheCapacities(0, 0, 0);

	m_Item = tpos;
	m_di = bi;
	bool success = theSln.LoadDoc(m_Item, m_di, content);
	if (!success)
		return false;
	QWebView::setHtml(content, QUrl::fromLocalFile(m_Item->GetDocAbsPath(m_di)));
	settings()->setAttribute(QWebSettings::AutoLoadImages, true);
	settings()->setUserStyleSheetUrl(QUrl::fromLocalFile(theSln.GetCssAbsPath(bi)));
	page()->setContentEditable(true);
	connect(page(), &QWebPage::linkClicked, this, &WebEditView::onLinkClicked);
	
	QWidget* parent = parentWidget();
	while (parent && !qobject_cast<QMdiSubWindow*>(parent)) {
		parent = parent->parentWidget();
	}
	QMdiSubWindow *subwnd = qobject_cast<QMdiSubWindow*>(parent);
	subwnd->setWindowTitle(m_Item->GetTitle(m_di));
	subwnd->setWindowIcon(m_wMain->m_wSln->GetTreeItemIcon(m_Item->status));

	setWindowModified(false);

	return true;
}


//////////////////////////////////////////////////////////////////////////
// edit

void WebEditView::onEditCut()
{
	triggerPageAction(QWebPage::Cut);
	QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *mimeData = clipboard->mimeData();
	if (mimeData->hasHtml()) {
		theSln.m_RecentClipboard = mimeData->html();
	}
}

void WebEditView::onEditCutText()
{
	triggerPageAction(QWebPage::Cut);
	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();
	clipboard->setText(originalText);
}

void WebEditView::onEditCopy()
{
	triggerPageAction(QWebPage::Copy);
	QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *mimeData = clipboard->mimeData();
	if (mimeData->hasHtml()) {
		theSln.m_RecentClipboard = mimeData->html();
	}
}

void WebEditView::onEditCopyText()
{
    triggerPageAction(QWebPage::Copy);
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();
    clipboard->setText(originalText);
}

void WebEditView::onEditPaste()
{
	const QClipboard *clipboard = QApplication::clipboard();
	if (!clipboard)
		return;
	const QMimeData *mimeData = clipboard->mimeData();
	if (!mimeData)
		return;
	QList<QString> formats = mimeData->formats();
	if (mimeData->hasImage()) {
		QImage img = clipboard->image();
		if (!img.isNull()) {
			QString html = HtmlImage::MakeHtml(img);
			InsertHtml(html);
		}
	}
	else if (formats.contains("application/x-myformat-doc-uid")) {
		QByteArray uid = mimeData->data("application/x-myformat-doc-uid");
		DocItem* doc = theSln.Locate(uid);
		if (doc) {
			QString html = "<a href=" + doc->GetRelUrl(m_Item, m_di) + ">" + doc->GetTitle(0) + "</a>";
			InsertHtml(html);
		}
	}
	else if (!formats.contains("text/html")) {
		onEditPasteText();
	}
	else {
		triggerPageAction(QWebPage::Paste);
	}
}

void WebEditView::onEditPasteText()
{
	// paste as text
	QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    text = text.toHtmlEscaped();

    text = text.replace("\r\n", "<p>");
    text = text.replace("\r", "<p>");
    text = text.replace("\n", "<p>");
    InsertHtml(text);
}

void WebEditView::onEditPasteAsTable()
{
    // paste text as table
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();

	QString html = HtmlTable::MakeHtml(originalText);
    InsertHtml(html);
}

void WebEditView::onEditPasteAsCode()
{
	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();
	originalText = originalText.toHtmlEscaped();
	QString html = theSln.m_Snippets.GetSnippet("code.html", originalText);
    InsertHtml(html);
}

void WebEditView::onEditPasteAsBilingua()
{
	QClipboard *clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if (mimeData && mimeData->hasHtml()) {
		QString htmlContent = mimeData->html();
		QString html = theSln.m_Snippets.GetSnippet("bilingua.html", htmlContent);
		InsertHtml(html);
    }
    else {
        // for cross-vm copy/paste
        QString originalText = clipboard->text();
        originalText = originalText.toHtmlEscaped();
        QString html = theSln.m_Snippets.GetSnippet("bilingua.html", originalText);
        InsertHtml(html);
    }
}

void WebEditView::onEditPasteAsLWText()
{
	// paste text and correct line-wrapping: replace all ".L" to ".\r\nL"
	QClipboard *clipboard = QApplication::clipboard();
	QString text = clipboard->text();
	text = text.toHtmlEscaped();

	text = text.replace("\r\n", "<p>");
	text = text.replace("\r", "<p>");
	text = text.replace("\n", "<p>");

	QRegularExpression regex("([.!?])(\\S)"); //("\\.(\\S)");
	text = text.replace(regex, "\\1<p>\\2");

	InsertHtml(text);
}

void WebEditView::onEditPasteCell()
{
	// paste as text without CR LF s
	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();
	originalText.replace('\r', ' ');
	originalText.replace('\n', ' ');
	clipboard->setText(originalText);
	triggerPageAction(QWebPage::Paste);
}

void WebEditView::onTableInsertData()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);

	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();

	table.InsertData(originalText, m_elTD);
	setWindowModified(true);
}

QWebElement WebEditView::getTag()
{
	QWebFrame *frame = this->page()->mainFrame();
	QWebElement el = frame->documentElement(); // html?
	QString s = el.tagName();
	el = el.findFirst("BODY");
	s = el.tagName();
	el = el.findFirst("TABLE");
	s = el.tagName();
	
	return el;
}

void WebEditView::onEditPasteSpecial()
{
	QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *mimeData = clipboard->mimeData();
	if (mimeData->hasHtml()) {
		QString data = mimeData->html();
		if (data != theSln.m_RecentClipboard) {
			clipboard->setText(clipboard->text());
		}
	}
	triggerPageAction(QWebPage::Paste);
}


void WebEditView::onEditUndo()
{
	triggerPageAction(QWebPage::Undo);
}

void WebEditView::onEditRedo()
{
	triggerPageAction(QWebPage::Redo);
}

void WebEditView::onEditUntag()
{
	execScript("unwrapCaretTag()");
}

void WebEditView::onEditOutside()
{
	execScript("insertOutside()");
}

void WebEditView::onEditTagInfo()
{
	execScript("getTags()");
}

void WebEditView::onEditUntable()
{
	execScript("tableToText()");
}

bool WebEditView::onEditDeflist()
{
	return execScript("makeDefItem()");
}

void WebEditView::onEditCopyLink()
{
	QString uid = m_Item->GetGuid();
	CopyLink(uid.toLocal8Bit().constData());
}

//////////////////////////////////////////////////////////////////////////
// text
void WebEditView::onTextBold()
{
	//execCommand("bold");
	execScript("makeSpan('B')");
}

void WebEditView::onTextItalic()
{
	//execCommand("italic");
	execScript("makeSpan('I')");
}

void WebEditView::onTextUnderline()
{
	//execCommand("underline");
	execScript("makeSpan('U')");
}

void WebEditView::onTextStrike()
{
	//execCommand("strikeThrough");
	execScript("makeSpan('S')");
}

void WebEditView::onTextSubscript()
{
	//execCommand("subscript");
	execScript("makeSpan('SUB')");
}

void WebEditView::onTextSuperscript()
{
	//execCommand("superscript");
	execScript("makeSpan('SUP')");
}

void WebEditView::onTextCode()
{
	execScript("makeSpan('CODE')");
}

void WebEditView::onTextMark()
{
	execScript("makeSpan('MARK')");
}

void WebEditView::onTextMark1()
{
	execScript("makeSpan('MARK1')");
}

void WebEditView::onTextMark2()
{
	execScript("makeSpan('MARK2')");
}

void WebEditView::onTextMark3()
{
	execScript("makeSpan('MARK3')");
}

void WebEditView::onTextMark4()
{
	execScript("makeSpan('MARK4')");
}

void WebEditView::onTextMark5()
{
	execScript("makeSpan('MARK5')");
}

void WebEditView::onTextMark6()
{
	execScript("makeSpan('MARK6')");
}

void WebEditView::onTextMark7()
{
	execScript("makeSpan('MARK7')");
}


void WebEditView::onTextStrong()
{
	execScript("makeSpan('STRONG')");
}
void WebEditView::onTextEm()
{
	execScript("makeSpan('EM')");
}
void WebEditView::onTextIns()
{
	execScript("makeSpan('INS')");
}
void WebEditView::onTextDel()
{
	execScript("makeSpan('DEL')");
}
void WebEditView::onTextSamp()
{
	execScript("makeSpan('SAMP')");
}
void WebEditView::onTextVar()
{
	execScript("makeSpan('VAR')");
}
void WebEditView::onTextKbd()
{
	execScript("makeSpan('KBD')");
}

void WebEditView::onParaHeading1()
{
	//execCommand("formatBlock", "h1");
	execScript("makeDiv('H1')");
}

void WebEditView::onParaHeading2()
{
	execScript("makeDiv('H2')");
}

void WebEditView::onParaHeading3()
{
	execScript("makeDiv('H3')");
}

void WebEditView::onParaHeading4()
{
	execScript("makeDiv('H4')");
}

void WebEditView::onParaHeading5()
{
	execScript("makeDiv('H5')");
}

void WebEditView::onParaHeading6()
{
	execScript("makeDiv('H6')");
}

void WebEditView::onParaDiv()
{
	execScript("makeDiv('DIV')");
}

void WebEditView::onParaPara()
{
	execScript("makeDiv('P')");
}

void WebEditView::onParaComment()
{
	execScript("makeDiv('COMMENT')");
}

void WebEditView::onParaQuestion()
{
	execScript("makeDiv('QUESTION')");
}

void WebEditView::onParaImportant()
{
	execScript("makeDiv('IMPORTANT')");
}

void WebEditView::onParaFeature()
{
	execScript("makeDiv('FEATURE')");
}

void WebEditView::onParaQuote()
{
	execScript("makeDiv('BLOCKQUOTE')");
}

void WebEditView::onParaSource()
{
	execScript("makeDiv('PRE')");
}

void WebEditView::onParaAnn()
{
	execScript("makeDiv('ANN')");
}

void WebEditView::onParaTerm()
{
	execScript("makeDiv('TERMINAL')");
}

void WebEditView::onParaNote()
{
	execScript("makeDiv('NOTE')");
}

//////////////////////////////////////////////////////////////////////////
// insert
void WebEditView::onInsertHorzLine()
{
	execCommand("insertHorizontalRule");
}

void WebEditView::onInsertTable(int cols, int rows)
{
	HtmlTable table;
	InsertHtml(table.MakeHtml(rows, cols));
}

void WebEditView::onInsertTable()
{
	TableProperties dlg;
	HtmlTable table;
	if(dlg.DoModal(5,5,"","","") == QDialog::Accepted)
	{
		InsertHtml(table.MakeHtml(dlg.m_rowsCount, dlg.m_colsCount));
	}
}

void WebEditView::onInsertImage()
{
	ImageProperties dlg;
	dlg.m_action = static_cast<ImageAction> (INI::LastImageAction);
	dlg.m_adir = m_Item->GetAbsDir(m_di);

	if(dlg.DoModal() == QDialog::Accepted)
	{
		INI::LastImageAction = static_cast<int> (dlg.m_action);
		QString res = PrepareImage(dlg.m_action, dlg.m_fpath);
		InsertImage(res, dlg.m_width, dlg.m_height);
	}
}

void WebEditView::InsertLink(const QString &url, const QString &text)
{
	execCommand("createLink", url);
	execCommand("insertText", text);
//	InsertHtml(QString::asprintf("<a href='%s'>%s</a>", url, text));
}

void WebEditView::onInsertHyperlink()
{
    QString sel = page()->selectedText();
	LinkProperties dlg;
    if(dlg.DoModal(false, sel, sel.startsWith("http") ? sel : "", m_Item, m_di)) {
		InsertLink(dlg.m_url, dlg.m_text);
	}
}

void WebEditView::onInsertDateTime()
{
	QDateTime t = QDateTime::currentDateTime();
	QString s = t.toString(Qt::SystemLocaleLongDate);
	
	InsertHtml(s);
}

void WebEditView::onInsertSnippet()
{
	SnippetsDlg dlg;
	if(dlg.DoModal() == QDialog::Accepted && dlg.m_pSel)
	{
		// snippet name
		QString s = *dlg.m_pSel;
		s = theSln.m_Snippets.GetSnippet(s);
		InsertHtml(s);
	}
}

void WebEditView::onInsertSymbol()
{
	// insert a special dialog for selecting a unicode character here
	//triggerPageAction(QWebPage::InspectElement);
    SymbolsDlg dlg;
	if (dlg.DoModal() == QDialog::Accepted)
	{
		// snippet name
		QString s = dlg.m_Symbol;
		InsertSymbol(s);
	}
	m_wMain->getSln()->UpdateSymbols();
}

void WebEditView::onInsertNumList()
{
	execCommand("insertOrderedList");
}

void WebEditView::onInsertBulList()
{
	execCommand("insertUnorderedList");
}

void WebEditView::InsertSymbol(QString html)
{
	QString js =
		"var r = document.createTextNode('" + html + " '); "
		"var p = r.nodeValue; "
		"var s = document.createElement('span'); "
		"s.innerHTML = p; "
		"var sl = window.getSelection ? window.getSelection() : window.document.selection; "
		"var t = sl.rangeCount > 0 ? sl.getRangeAt(0) : document.createRange(); "
		"t.insertNode(s); "
		"var z = s.innerHTML; "
		"s.outerHTML = z; "
		"t.setStart(t.startContainer, t.startOffset + 1); "
		"t.collapse(true); "
		"sl.removeAllRanges(); "
		"sl.addRange(t); "
		;

	QWebFrame *frame = this->page()->mainFrame();
	QVariant r = frame->evaluateJavaScript(js);
	setWindowModified(true);
}

void WebEditView::InsertHtml(QString html)
{	
    html.replace("\r\n", "&NewLine;");
    html.replace("\r", "&NewLine;");
    html.replace("\n", "&NewLine;");
	html.replace("\'", "&rsquo;");
	html.replace("\\", "&bsol;");
	
	QString js = 
		"var r = document.createTextNode('" + html + " '); "
		"var p = r.nodeValue; "
		"var s = document.createElement('span'); "
		"s.innerHTML = p; "
		"var sl = window.getSelection ? window.getSelection() : window.document.selection; "
		"var t = sl.rangeCount > 0 ? sl.getRangeAt(0) : document.createRange(); "
		"t.insertNode(s); "
		"var z = s.innerHTML; "
		"s.outerHTML = z; "
		;

	QWebFrame *frame = this->page()->mainFrame();
	QVariant r = frame->evaluateJavaScript(js);
	setWindowModified(true);
}

QString WebEditView::PrepareImage(ImageAction action, const QString &fpath)
{
	QString res;
	
	if (action == ImageAction::Copy) {
		// copy the image, with generation of new name
		QFileInfo fi(fpath);
		res = GenerateUniqueFPath(m_Item->GetAbsDir(m_di), fi.baseName(), fi.completeSuffix());
		QFile::copy(fpath, res);
		res = GetRelPath(res, m_Item->GetAbsDir(m_di), false);
	}
	else if (action == ImageAction::Move) {
		// move the image, with generation of new name
		QFileInfo fi(fpath);
		res = GenerateUniqueFPath(m_Item->GetAbsDir(m_di), fi.baseName(), fi.completeSuffix());
		QFile::rename(fpath, res);
		res = GetRelPath(res, m_Item->GetAbsDir(m_di), false);
	}
	else if (action == ImageAction::Embed) {
		// convert to embedded data
		res = HtmlImage::ConvertToEmbedded(fpath);
	}
	else if (action == ImageAction::Extract) {
		res = GetRelPath(fpath, m_Item->GetAbsDir(m_di), false);
	}
	else if (action == ImageAction::Href) {
		// direct hyperlink to image
		res = GetRelPath(fpath, m_Item->GetAbsDir(m_di), false);
	}
	return res;
}

void WebEditView::InsertImage(const QString &image, int w, int h)
{
	QString html;
	// without w/h:
	// execCommand("insertImage", npath);

	// 2 generate html
	html = "<img src=\"" + image + "\"";
	if (w < 0)
		html += QString::asprintf(" width=\"%d%%\"", -w);
	else if (w > 0)
		html += QString::asprintf(" width=\"%d\"", w);
	if (h < 0)
		html += QString::asprintf(" height=\"%d%%\"", -h);
	else if (h > 0)
		html += QString::asprintf(" height=\"%d\"", h);
	html += ">";

	// 3 insert image into document
	InsertHtml(html);
}

void WebEditView::onTableProperties()
{
	if(m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	TableProperties dlg;
	if(dlg.DoModal(table.GetRowCount(), table.GetColCount(), 
        Html::GetClass(m_elTable), Html::GetClass(m_elTR), Html::GetClass(m_elTD)) == QDialog::Accepted) {
		table.SetDimensions(dlg.m_rowsCount, dlg.m_colsCount);
        // set classes
        Html::SetClass(m_elTable, dlg.m_classTable);
        Html::SetClass(m_elTR, dlg.m_classTR);
        Html::SetClass(m_elTD, dlg.m_classTD);

		setWindowModified(true);
	}
}

void WebEditView::onTableInsAbove()
{
    GetCaretContext();
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.InsertRowAbove(m_elTR);
	setWindowModified(true);
}

void WebEditView::onTableInsBelow()
{
    GetCaretContext();
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.InsertRowBelow(m_elTR);
	setWindowModified(true);
}

void WebEditView::onTableNormalizeRow()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	if(table.NormalizeRow(m_elTR))
		setWindowModified(true);
}

void WebEditView::onTableRemoveEmptyRows()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	if (table.RemoveEmptyRows())
		setWindowModified(true);
}

void WebEditView::onTableInsLeft()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.InsertColLeft(m_elTD);
	setWindowModified(true);
}

void WebEditView::onTableInsRight()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.InsertColRight(m_elTD);
	setWindowModified(true);
}

void WebEditView::onTableClearColumn()
{
	HtmlTable table(m_elTable);
	table.ClearColumn(m_elTD);
	setWindowModified(true);
}

void WebEditView::onTableClearRow()
{
	HtmlTable table(m_elTable);
	table.ClearRow(m_elTD);
	setWindowModified(true);
}

void WebEditView::onTableDeleteColumn()
{
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	int r = QMessageBox::warning(this, AppTitle,
		tr("Are you really want to remove column from table?"),
		QMessageBox::Yes | QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		HtmlTable table(m_elTable);
		table.DeleteColumn(m_elTD);
		setWindowModified(true);
	}
}

void WebEditView::onTableDeleteRow()
{
	GetCaretContext();
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	int r = QMessageBox::warning(this, AppTitle,
		tr("Are you really want to remove row from table?"),
		QMessageBox::Yes | QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		HtmlTable table(m_elTable);
		table.DeleteRow(m_elTR);
		setWindowModified(true);
	}
}

void WebEditView::onTableMoveRowAbove()
{
    GetCaretContext();
    if (m_elTable.isNull() || m_elTR.isNull()) {
        QMessageBox::warning(this, AppTitle, tr("Table or row not found"), QMessageBox::Ok);
        return;
    }
    HtmlTable table(m_elTable);
    table.MoveRow(m_elTR, false);
    setWindowModified(true);
}

void WebEditView::onTableMoveRowBelow()
{
    GetCaretContext();
    if (m_elTable.isNull() || m_elTR.isNull()) {
        QMessageBox::warning(this, AppTitle, tr("Table or row not found"), QMessageBox::Ok);
        return;
    }
    HtmlTable table(m_elTable);
    table.MoveRow(m_elTR, true);
    setWindowModified(true);
}

void WebEditView::onTableMoveColLeft()
{
    GetCaretContext();
    if (m_elTable.isNull() || m_elTR.isNull() || m_elTD.isNull()) {
        QMessageBox::warning(this, AppTitle, tr("Table, row or cell not found"), QMessageBox::Ok);
        return;
    }
    HtmlTable table(m_elTable);
    table.MoveColumn(m_elTD, false);
    setWindowModified(true);
}

void WebEditView::onTableMoveColRight()
{
    GetCaretContext();
    if (m_elTable.isNull() || m_elTR.isNull() || m_elTD.isNull()) {
        QMessageBox::warning(this, AppTitle, tr("Table, row or cell not found"), QMessageBox::Ok);
        return;
    }
    HtmlTable table(m_elTable);
    table.MoveColumn(m_elTD, true);
    setWindowModified(true);
}

void WebEditView::onImageProperties()
{
	if(m_elImage.isNull())
		return;
	HtmlImage image(m_elImage);
	ImageProperties dlg;
	dlg.m_adir = m_Item->GetAbsDir(m_di);
	dlg.m_width = image.GetWidth();
	dlg.m_height = image.GetHeight();
	dlg.m_embedded = image.IsEmbedded();
	dlg.m_fpath = image.GetSrc();
	ImageAction a = dlg.m_action = image.IsEmbedded() ? ImageAction::Embed : ImageAction::Href;
	if(dlg.m_embedded)
		dlg.m_ext = HtmlImage::GetImageExt(dlg.m_fpath);
	
	if(dlg.DoModal() == QDialog::Accepted)
	{
		if(dlg.m_action == ImageAction::Extract)
			HtmlImage::ConvertToFile(image.GetSrc(), dlg.m_fpath);
		
		if ((!dlg.m_fpath.isEmpty() && dlg.m_fpath != image.GetSrc()) || dlg.m_action != a) {
			QString localpath = QUrl(dlg.m_fpath).toLocalFile();
			if (localpath.isEmpty())
				localpath = dlg.m_fpath;
			if(QFileInfo(localpath).isAbsolute())
				dlg.m_fpath = PrepareImage(dlg.m_action, localpath);
			else
				dlg.m_fpath = PrepareImage(dlg.m_action, dlg.m_adir + "/" + localpath);
			if(dlg.m_fpath.isEmpty())
				QMessageBox::warning(this, AppTitle, tr("Bad new img src!"), QMessageBox::Ok);
			else
				image.SetSrc(dlg.m_fpath);
		}

		if (dlg.m_delete && !dlg.m_embedded)
			QFile::remove(dlg.m_adir + "/" + image.GetSrc());

		image.SetWidth(dlg.m_width);
		image.SetHeight(dlg.m_height);
		setWindowModified(true);
	}
}

void WebEditView::onLinkProperties()
{
	if (m_elLink.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Link not found"), QMessageBox::Ok);
		return;
	}
	HtmlLink link(m_elLink);
	LinkProperties dlg;
    if (dlg.DoModal(true, link.GetText(), link.GetUrl(), m_Item, m_di)) {
        link.Make(dlg.m_text, dlg.m_url);
        setWindowModified(true);
	}
}

void WebEditView::onEditClearDoc()
{
	// clear the current document from unnecessary tags and attributes according to a certain algorithm
	execScript("clearDoc()");	
}

void WebEditView::Find(const QString &text, bool backward)
{
	if(backward)
		findText(text, QWebPage::FindWrapsAroundDocument | QWebPage::FindBackward);
	else
		findText(text, QWebPage::FindWrapsAroundDocument);
	setFocus();
}

void WebEditView::onLinkClicked(const QUrl & url)
{
	QString s = url.toString();
	if (!s.startsWith("http"))
		m_wMain->OpenLocalLink(s, m_di);
	else if (INI::BrowserPath.empty())
		QMessageBox::warning(this, "Error", "BrowserPath is not set!");
	else
		OpenInExternalApplication(this, U16(INI::BrowserPath), s);
}

void WebEditView::onToolsLink()
{
	// open the highlighted link in the browser (if it starts with http (s) or in a search engine)
	QString sel;
	sel = page()->selectedText();
	if(sel.isEmpty())
		execScript("getFocusText()", &sel);

	if (INI::BrowserPath.empty())
		QMessageBox::warning(this, "Error", "BrowserPath is not set!");
	else if (sel.startsWith("http://") || sel.startsWith("https://"))
		OpenInExternalApplication(this, U16(INI::BrowserPath), sel);
	else
		OpenInExternalApplication(this, U16(INI::BrowserPath), "https://www.google.com/search?q=" + sel);
}

void WebEditView::onToolsSearch()
{
	// 1 get selected text
	QString sel;
	sel = page()->selectedText();
	if (sel.isEmpty())
		execScript("getFocusText()", &sel);

	// 2 activate search
	m_wMain->Search(sel);
}

void WebEditView::onToolsTranslate()
{
	QString sel;
	sel = page()->selectedText();
	if (sel.isEmpty())
		execScript("getFocusText()", &sel);
	if (INI::BrowserPath.empty())
		QMessageBox::warning(this, "Error", "BrowserPath is not set!");
	else
		OpenInExternalApplication(this, U16(INI::BrowserPath), "http://translate.google.com/#auto/en/" + sel);
}

void WebEditView::onTableAppendData()
{
	// paste into end of table
	GetCaretContext();
	if (m_elTable.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}

	HtmlTable table(m_elTable);

	QClipboard *clipboard = QApplication::clipboard();
	QString originalText = clipboard->text();
	table.AppendData(originalText);
	setWindowModified(true);
}

void WebEditView::onTableExpand()
{
	GetCaretContext();
	if (m_elTable.isNull() || m_elTD.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.Expand(m_elTD);
	setWindowModified(true);
}

void WebEditView::onTableCollapse()
{
	GetCaretContext();
	if (m_elTable.isNull() || m_elTD.isNull()) {
		QMessageBox::warning(this, AppTitle, tr("Table not found"), QMessageBox::Ok);
		return;
	}
	HtmlTable table(m_elTable);
	table.Collapse(m_elTD);
	setWindowModified(true);
}
