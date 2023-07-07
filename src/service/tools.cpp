#include "tools.h"
#include <time.h>
#include <QStringList>
#include <QProcess>
#include <QToolButton>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QTextCodec>
#include <QMessageBox>
#include "../core/DocItem.h"

extern QTextCodec *codecUtf8;

bool IsBlank(const QString &s)
{
	return s.trimmed().isEmpty();
}

void AddSlash(QString &path)
{
	if (path.isEmpty())
		return;
	QChar ch = path[path.count()-1];
	if (ch != '\\' && ch != '/')
		path += '/';
}

void NormalizeFName(QString &fname)
{
	// normalization of the name, replacement of any curved characters like *? / \ on underscores
    QString illegal="<>:\"|?*/\\ .";
    for(int i=0; i<fname.length(); i++)  {
        if (fname[i].toLatin1() >= 0 && fname[i].toLatin1() < 32)
            fname[i] = '_';
        if (illegal.contains(fname[i]))
            fname[i] = '_';
    }
}

bool IsLegalFileName(QString fname)
{
    if (!fname.length())
        return false;
    QString illegal="<>:\"|?*/\\ .";
    foreach (const QChar& c, fname) {
        if (c.toLatin1() >= 0 && c.toLatin1() < 32)
            return false;
        if (illegal.contains(c))
            return false;
    }
    fname = fname.toUpper();
    static QStringList devices;
    if (!devices.count())
        devices << "CON" << "PRN" << "AUX" << "NUL"
                << "COM0" << "COM1" << "COM2" << "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8" << "COM9"
                << "LPT0" << "LPT1" << "LPT2" << "LPT3" << "LPT4" << "LPT5" << "LPT6" << "LPT7" << "LPT8" << "LPT9";
    foreach (const QString& d, devices)
        if (fname == d)
            return false;
    return true;
}

unsigned int GenerateUniqueFNum(const QString& path, const QString& name, const QString& ext)
{
	// generate a unique filename at a specific path
	QString spath = path + "/" + name + "0." + ext;
	unsigned int n = 0;
	while (QFileInfo(spath).exists()) {
		n++;
		spath = path + "/" + name + QString::number(n) + "." + ext;
	}
	return n;
}

QString	GenerateUniqueFTitle(const QString& path, const QString& name, const QString& ext)
{
	unsigned int n = GenerateUniqueFNum(path, name, ext);
	return name + QString::number(n);
}

QString	GenerateUniqueFName(const QString& path, const QString& name, const QString& ext)
{
	unsigned int n = GenerateUniqueFNum(path, name, ext);
	return name + QString::number(n) + "." + ext;
}

QString	GenerateUniqueFPath(const QString& path, const QString& name, const QString& ext)
{
	unsigned int n = GenerateUniqueFNum(path, name, ext);
	return path + "/" + name + QString::number(n) + "." + ext;
}

QString U16(const char* s)
{
	return codecUtf8->toUnicode(s);
}

QString U16(const std::string &s)
{
	return codecUtf8->toUnicode(s.c_str());
}

std::string U8(const QString &s)
{
	return codecUtf8->fromUnicode(s).toStdString();
}

QByteArray U8a(const QString &s)
{
	return codecUtf8->fromUnicode(s);
}

bool OpenInExternalApplication(QWidget *par, const QString &app, const QString &fpath)
{
	QStringList arguments;
	QString d;
	if(fpath[0]=='/' || fpath[1]==':')
		d = QFileInfo(fpath).absoluteDir().absolutePath();
	arguments << fpath;
	bool r = QProcess::startDetached(app, arguments, d);
	//bool r = QProcess::startDetached(app + " \"" + QFileInfo(fpath).canonicalFilePath() + "\"");
	if (!r)
		QMessageBox::warning(par, "NeoPad", "Process start error! Process: \r\n" + app);
	return r;
}

int StartExternalApplication(const QString &app, const QString& FileName, const QString& dir)
{
	QString fn = FileName;
	QStringList arguments;
	arguments << fn;

	int c = StartExternalApplication(app, arguments, dir);
	return c;
}

int StartExternalApplication(const QString &app, QStringList &arguments, const QString& dir)
{
	QString old = QDir::currentPath();
	QDir::setCurrent(dir);
	int c = QProcess::execute(app, arguments);
	QDir::setCurrent(old);
	return c;
}

void SetAutoRaiseToolBar( QToolBar *toolbar, bool value )
{
	QObjectList childs = toolbar->children();
	QListIterator<QObject*> i( childs );
	 while( i.hasNext() )
	 {
		QToolButton *btn = qobject_cast<QToolButton*>( i.next() );
		if( btn )
			btn->setAutoRaise( value );
	 }
}
 
QString GetRelPath(const QString &path, const QString &base, bool base_is_file)
{
	QDir dir;
	if (base_is_file)
		dir = QFileInfo(base).dir();
	else 
		dir = QDir(base);
	return dir.relativeFilePath(path);
}

QString GetRelDir(const QString &path, const QString &base, bool base_is_file)
{
	return QFileInfo(GetRelPath(path, base, base_is_file)).dir().path();
}

QTreeWidgetItem* AddTreeItem(QTreeWidgetItem *par, QTreeWidgetItem *after, const QString &title, const QIcon &icon)
{
    QTreeWidgetItem *newitem = after ? new QTreeWidgetItem(par, after) : new QTreeWidgetItem(par);
    newitem->setText(0, title);
    newitem->setIcon(0, icon);
    par->setExpanded(true);
    QTreeWidget *tree = par->treeWidget();
    tree->setCurrentItem(newitem);
    return newitem;
}

void RemoveTreeNode(QTreeWidgetItem *newitem, QTreeWidgetItem *olditem)
{
    QTreeWidget *tree = newitem->treeWidget();
    if(olditem)
        tree->setCurrentItem(olditem);
    QTreeWidgetItem *par = newitem->parent();
    if(par) {
        int index = par->indexOfChild(newitem);
        delete par->takeChild(index);
    }
    else {
        int index = tree->indexOfTopLevelItem(newitem);
        delete tree->takeTopLevelItem(index);
    }
}

QTreeWidgetItem *GetPrevSibling(QTreeWidgetItem *item)
{
    QTreeWidget *tree = item->treeWidget();
    QTreeWidgetItem *parent = item->parent();
    QTreeWidgetItem *sibling;
    if(parent)
        sibling = parent->child(parent->indexOfChild(item)-1);
    else
        sibling = tree->topLevelItem(tree->indexOfTopLevelItem(item)-1);
    return sibling;
}

QTreeWidgetItem *GetNextSibling(QTreeWidgetItem *item)
{
    QTreeWidget *tree = item->treeWidget();
    QTreeWidgetItem *parent = item->parent();
    QTreeWidgetItem *sibling;
    if(parent)
        sibling = parent->child(parent->indexOfChild(item)+1);
    else
        sibling = tree->topLevelItem(tree->indexOfTopLevelItem(item)+1);
    return sibling;
}

void MoveItem(QTreeWidgetItem *item, QTreeWidgetItem *insparent, QTreeWidgetItem *insafter)
{
    // move item element - Child of insparent, after insafter (or to the end if null)
    QTreeWidgetItem *oldparent = item->parent();
    if(oldparent)
    {
        // take
        int oldindex = oldparent->indexOfChild(item);
        QTreeWidgetItem *i = oldparent->takeChild(oldindex); // should be i == item

        // put in a new place
        if(insafter)
        {
            int insindex = insparent->indexOfChild(insafter) + 1;
            insparent->insertChild(insindex, item);
        }
        else
        {
            int insindex = insparent->childCount();
            insparent->insertChild(insindex, item);
        }
    }
}

void SetCurrentItem(QTreeWidgetItem *item)
{
    item->treeWidget()->setCurrentItem(item);
}

QTreeWidgetItem* FindItem(QTreeWidgetItem *par, DocItem* mtpos)
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
