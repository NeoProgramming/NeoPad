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

QString	GenerateUniqueFPath(const QString& path, const QString& name, const QString& ext)
{
	// generate a unique filename at a specific path
	QString spath = path;
	QString sname = name;
	spath += "/";
	spath += name;
	spath += ".";
	spath += ext;

	// first based on short name
	if( QFileInfo(spath).exists() )
	{
		// such a file exists - we are trying to add the current timestamp to it
		time_t timeObj;
		time(&timeObj);
		tm *pt = gmtime(&timeObj);

		sname.sprintf("%s_%02d%02d%02d_%02d%02d%02d", name,
			pt->tm_mday, pt->tm_mon, pt->tm_year, pt->tm_hour, pt->tm_min, pt->tm_sec);
		spath = path + "/" + sname + "." + ext;
		srand(QDateTime::currentMSecsSinceEpoch() / 1000);
		while (QFileInfo(spath).exists())
		{
			// if such a file exists, we just generate random numbers until we guess
			sname.sprintf("%s_rnd%d", name, rand());
			spath = path + "/" + sname + "." + ext;
		}
	}
	return spath;
}

QString GenerateUniqueNumFName(const QString& path, const QString& name, const QString& ext)
{
	// generate a unique file name based on the path and the first part of the name
	// the file name is set separately from the extension
		
	// but is there such a file at all? if not, there is no need to search
	unsigned int n = 0;
	while (n < ~0) {
		QString uname = path + "/" + name + QString::number(n) + "." + ext;
		if (!QFileInfo(uname).exists())
			return uname;
		n++;
	}
	return "";
}

int GetDocNum(const QString & doc)
{
	// get document number if filename starts with doc
	if (IsBlank(doc))
		return 0;
	QString fname = QFileInfo(doc).fileName();
	if (fname.leftRef(3) == "doc")
	{
		int num = fname.midRef(3).toInt();
		return num;
	}
	return 0;
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
	QString d = QFileInfo(fpath).absoluteDir().absolutePath();
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
