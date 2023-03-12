#include "DocItem.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include "Solution.h"
#include "../service/tools.h"

DocItem::DocItem() : BaseItem<DocItem>()
{
	//for (int i = 0; i<theSln.BCnt(); i++)
   //	time[i] = -1;
	for (auto &v : time)
		v = -1;

	attrs = 0;
	status = ETreeStatus::TS_UNREADY;
	check = 1;
}

DocItem::DocItem(const char *text) : BaseItem<DocItem>()
{
	attrs = 0;
	title[0] = text;
	status = ETreeStatus::TS_UNREADY;
	//for (int i = 0; i<theSln.BCnt(); i++)
	//	time[i] = -1;
	for (auto &v : time)
		v = -1;
}

DocItem::DocItem(const DocItem &obj) 
{
	attrs = obj.attrs;
	for (int i = 0; i < BCNT; i++) {
		title[i] = obj.title[i];
		time[i] = obj.time[i];
	}

	rdir = obj.rdir;
	attrs = obj.attrs;
	status = obj.status;

	parent = obj.parent;
	children = obj.children;
}

bool DocItem::IsPublic()
{
	// check if the document is published
	return status == ETreeStatus::TS_READY || status == ETreeStatus::TS_ALMOST;
}

int DocItem::GetPublicChildrenCount()
{
	int count = 0;
	for (auto child : children) {
		if (child->IsPublic())
			count++;
	}
	return count;
}

void DocItem::SetCheck(bool _check)
{
	this->check = _check;
}

bool DocItem::GetCheck()
{
	return check;
}

QString DocItem::GetId()
{
	return id;
}

void DocItem::ChangeModify(bool modify, bool recursive)
{
	p_modify = modify;
	if (recursive)
		for (auto child : children)
			child->ChangeModify(modify, recursive);
}


QString DocItem::GetInfo()
{
	// generate an information string with the element status
	QString info = QString("SubBase: %1, Modify: %2, OwnRDir: %3")
		.arg(p_subbase).arg(p_modify).arg(rdir);
	return info;
}

QString DocItem::GetInfo2()
{
	QString s;
	int nc = children.size();
	int nd = QDir(GetAbsDir(-1)).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).size();

	s.sprintf("Nodes: %d; Subdirs: %d; GUID: ", nc, nd);
	s += guid;
	return s;
}

void DocItem::LoadItemPaths(const QString &apath)
{
	QFileInfo fi(apath);
	rdir = GetRelDir(apath, theSln.m_RootDir, false);
	id = fi.baseName();	// vmb = id + ".vmbase"
}

QString DocItem::GetGuid()
{
	return guid;
}

QString DocItem::GetTitle(int bi)
{
	if (bi < 0 || bi >= BCNT)
		return QString();
	return title[bi];
}

QString DocItem::GetTitles(int bi)
{
	if (bi < 0 || bi >= BCNT)
		return QString();
	if (!parent)
		return GetTitle(bi);
	return GetTitle(bi) + QChar(0x26AB) + parent->GetTitles(bi);
}

QString DocItem::GetDocLocPath(int bi)
{
	QString c = id + theSln.m_Books.GetDocExt(bi);
	return c;
}

QString DocItem::GetDocRelPath(int bi)
{
	// get document path
	if (bi < 0 || bi >= BCNT)
		return QString();

	QString c = GetBaseDir();
	c += "/";
	c += GetDocLocPath(bi);
	return c;
}

QString DocItem::GetDocAbsPath(int bi)
{
	// get document path
	if (bi < 0 || bi >= BCNT)
		return QString();

	QString c = GetAbsDir(bi);
	c += "/";
	c += GetDocLocPath(bi);
	c = QDir::cleanPath(c);
	return c;
}

QString DocItem::GetVmbAbsPath()
{
	// get the path to the .vmbase file that contains this item
	QString c;
	c = GetAbsDir(-1);
	c += "/";
	c += id;
	c += ".vmbase";
	return c;
}

QString DocItem::GetVmbLocPath()
{
	QString c = id + ".vmbase";
	return c;
}

time_t DocItem::GetDocTime(int bi)
{
	// get document time from vmbase tags
	if (bi < 0 || bi >= BCNT)
		return 0;
	return time[bi];
}

ETreeStatus DocItem::GetTreeStatus()
{
	QString path = GetDocAbsPath(0);
	if (!QFileInfo(path).isFile())
		return ETreeStatus::TS_UNKNOWN;
	return status;
}

ELangStatus DocItem::GetLangStatus(int di2)
{
	QString path = GetDocAbsPath(di2);
	if (!QFileInfo(path).isFile())
		return ELangStatus::LS_NONE;

	time_t t0, t1;
	t0 = GetDocTime(0);
	t1 = GetDocTime(di2);
	if (t0 != -1 && t1 != -1)
	{
		if (t0 > t1)
			return ELangStatus::LS_OLD;
		return ELangStatus::LS_OK;
	}

	if (t0 == -1)
		t0 = QFileInfo(GetDocAbsPath(0)).lastModified().toTime_t();
	if (t1 == -1)
		t1 = QFileInfo(GetDocAbsPath(di2)).lastModified().toTime_t();
	if (t0 > t1)
		return ELangStatus::LS_QOLD;
	return ELangStatus::LS_QOK;
}

QString DocItem::GetAbsDir(int bi)
{
	// getting the absolute path for a given tree element
	// "general" version, so far without division into bases
	// c:/users/user1/project/base1/dir1/dir2/dir3"
	QString bdir = GetBaseDir();

	if (bi < 0 || bi >= BCNT)
		return theSln.m_RootDir + "/" + bdir;
	if (theSln.m_Books.books[bi].rpath.isEmpty())
		return theSln.m_RootDir + "/" + bdir;
	return     theSln.m_RootDir + "/" + theSln.m_Books.books[bi].rpath + "/" + bdir;
}

QString DocItem::GetBaseDir()
{
	// getting the path within base ; empty return is impossible
	// "./dir1/dir2/dir3"
	DocItem *item = this;
	while (item) {
		if (!item->rdir.isEmpty())
			return item->rdir;
		item = item->parent;
	}
	return ".";
}

void DocItem::UpdateBaseDir()
{
	// regenerate rdir based on id
	// there is a certain node; to regenerate rdir, you need to go through all the nodes
	rdir = id;
	DocItem* item = parent;
	while (item && item->parent) {
		rdir = item->id + "/" + rdir;
		item = item->parent;
	}
}

const char csTimeFormat[] = "yyyy-MM-dd HH:mm:ss";
QString DocItem::GetDocTimeStr(int bi)
{
	QDateTime dt;
	if (time[bi] == -1)
		return "";
	dt.setTime_t(time[bi]);
	QString tt = dt.toString(csTimeFormat);
	return tt;
}

void DocItem::SetDocTime(const char *tstr, int bi)
{
	if (IsBlank(tstr))
		time[bi] = -1;
	else {
		QDateTime dt = QDateTime::fromString(tstr, csTimeFormat);
		time[bi] = dt.toTime_t();
	}
}

void DocItem::SetDocTime(int bi)
{
	QFileInfo fi = QFileInfo(GetDocAbsPath(bi));
	if (!fi.exists())
		time[bi] = -1;
	else
		time[bi] = fi.lastModified().toTime_t();
}

QString DocItem::GetCssRelPath(int bi)
{
	// calculate the path to css from the given (this) document to the css file specified in the database
	// the path in the database can be either absolute or relative (relative to the root)
	QString doc = GetDocAbsPath(bi);
	QString css = theSln.GetCssAbsPath(bi);
	QString rel = ::GetRelPath(css, doc, true);
	return rel;
}

QString DocItem::GetRelPath(DocItem *item, int di)
{
	QString rel = ::GetRelPath(GetDocAbsPath(di), item->GetDocAbsPath(di), true);
	return rel;
}

QString DocItem::GetRelUrl(DocItem *item, int di)
{
	return GetRelPath(item, di) + "?" + GetGuid();
}
