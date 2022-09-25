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

extern QTextCodec *codecUtf8;

bool IsBlank(const QString &s)
{
	
	return s.isEmpty();
}

void AddSlash(QString &path)
{
	if (path.isEmpty())
		return;
	QChar ch = path[path.count()-1];
	if (ch != '\\' && ch != '/')
		path += '/';
}

void NormalizeFName(QString &c)
{
	// normalization of the name, replacement of any curved characters like *? / \ on underscores
	c.replace('/', '_');
	c.replace('\\', '_');
	c.replace('*', '_');
	c.replace('?', '_');
	c.replace(':', '_');
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

//-------------------------------------------------
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
