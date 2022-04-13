#include "PrjItem.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include "Solution.h"
#include "../service/tools.h"

const char csTimeFormat[] = "yyyy-MM-dd HH:mm:ss";

MT_ITEM::MT_ITEM()
{
	attrs = 0;
	status = ETreeStatus::TS_UNREADY;
	check = 1;
	for (int i = 0; i<theSln.BCnt(); i++)
		time[i] = -1;
}

MT_ITEM::MT_ITEM(const char *text)
{
	attrs = 0;
	title[0] = text;
	status = ETreeStatus::TS_UNREADY;
	for (int i = 0; i<theSln.BCnt(); i++)
		time[i] = -1;
}

MT_ITEM::MT_ITEM(const MT_ITEM &obj)
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

bool MT_ITEM::IsAncestor(MT_ITEM *item)
{
	// check if the given element is an ancestor of the item element
	while (item) {
		if (item == this)
			return true;
		item = item->parent;
	}
	return false;
}

bool MT_ITEM::IsPublic()
{
    // check if the document is published
    return status == ETreeStatus::TS_READY || status == ETreeStatus::TS_ALMOST;
}

void MT_ITEM::RemoveChildren()
{
	for (auto child : children) {
		child->RemoveChildren();
		delete child;
	}
	children.clear();
}

void MT_ITEM::LoadItemPaths(const QString &apath)
{
	QFileInfo fi(apath);
	rdir = GetRelDir(apath, theSln.m_RootDir, false);
	id = fi.baseName();	// vmb = id + ".vmbase"
}

void MT_ITEM::SetCheck(bool _check)
{
	this->check = _check;
}

bool MT_ITEM::GetCheck()
{
	return check;
}

QString MT_ITEM::GetInfo()
{
	// generate an information string with the element status
    QString info = QString("SubBase: %1, Modify: %2, OwnRDir: %3")
            .arg(p_subbase).arg(p_modify).arg(rdir);
	return info;
}

QString MT_ITEM::GetGuid()
{
	return guid;
}

QString MT_ITEM::GetTitle(int bi)
{
	if (bi<0 || bi >= theSln.BCnt())
		return QString();
	return title[bi];
}

QString MT_ITEM::GetDocLocPath(int bi)
{
	QString c = id + theSln.GetDocExt(bi);
	return c;
}

QString MT_ITEM::GetDocRelPath(int bi)
{
	// get document path
	if (bi<0 || bi >= theSln.BCnt())
		return QString();

	QString c = GetBaseDir();
	c += "/";
	c += GetDocLocPath(bi);
	return c;
}

QString MT_ITEM::GetDocAbsPath(int bi)
{
	// get document path
	if (bi<0 || bi >= theSln.BCnt())
		return QString();

	QString c = GetAbsDir(bi);
	c += "/";
	c += GetDocLocPath(bi);
	c = QDir::cleanPath(c);
	return c;
}

QString MT_ITEM::GetVmbAbsPath()
{
	// get the path to the .vmbase file that contains this item
	QString c;
	c = GetAbsDir(-1);
	c += "/";
	c += id;
	c += ".vmbase";
	return c;
}

QString MT_ITEM::GetVmbLocPath()
{
	QString c = id + ".vmbase";
	return c;
}

time_t MT_ITEM::GetDocTime(int bi)
{
	// get document time from vmbase tags
	if (bi<0 || bi >= theSln.BCnt())
		return 0;
	return time[bi];
}

QString MT_ITEM::GetId()
{
	return id;
}


ETreeStatus MT_ITEM::GetTreeStatus()
{
	QString path = GetDocAbsPath(0);
	if (!QFileInfo(path).isFile())
		return ETreeStatus::TS_UNKNOWN;
	return status;
}

ELangStatus MT_ITEM::GetLangStatus(int di2)
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

QString MT_ITEM::GetInfo2()
{
	QString s;
	int nc = children.size();
	int nd = QDir(GetAbsDir(-1)).entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).size();

	s.sprintf("Nodes: %d; Subdirs: %d; GUID: ", nc, nd);
	s += guid;
	return s;
}

void MT_ITEM::ChangeModify(bool modify, bool recursive)
{
	p_modify = modify;
	if (recursive) 
		for (MTPOS child : children) 
			child->ChangeModify(modify, recursive);
}

QString MT_ITEM::GetAbsDir(int bi)
{
	// getting the absolute path for a given tree element
	// "general" version, so far without division into bases
	// c:/users/user1/project/base1/dir1/dir2/dir3"
	QString bdir = GetBaseDir();
	
	if (bi < 0 || bi >= theSln.BCnt())
		return theSln.m_RootDir + "/" + bdir;
	if(theSln.m_Bases[bi].rpath.isEmpty())
		return theSln.m_RootDir + "/" + bdir;
	return theSln.m_RootDir + "/" + theSln.m_Bases[bi].rpath + "/" + bdir;
}

QString MT_ITEM::GetBaseDir()
{
	// getting the path within base ; empty return is impossible
	// "./dir1/dir2/dir3"
	MT_ITEM *item = this;
	while (item) {
		if (!item->rdir.isEmpty())
			return item->rdir;
		item = item->parent;
	}
	return ".";
}

void MT_ITEM::UpdateBaseDir()
{
	// regenerate rdir based on id
	// there is a certain node; to regenerate rdir, you need to go through all the nodes
	rdir = id;
	MT_ITEM* item = parent;
	while (item && item->parent) {
		rdir = item->id + "/" + rdir;
		item = item->parent;
	}
}

QString MT_ITEM::GetDocTimeStr(int bi)
{
	QDateTime dt;
	if (time[bi] == -1)
		return "";
	dt.setTime_t(time[bi]);
	QString tt = dt.toString(csTimeFormat);
	return tt;
}

void MT_ITEM::SetDocTime(const char *tstr, int bi)
{
	if (IsBlank(tstr))
		time[bi] = -1;
	else {
		QDateTime dt = QDateTime::fromString(tstr, csTimeFormat);
		time[bi] = dt.toTime_t();
	}
}

void MT_ITEM::SetDocTime(int bi)
{
	QFileInfo fi = QFileInfo(GetDocAbsPath(bi));
	if (!fi.exists())
		time[bi] = -1;
	else
		time[bi] = fi.lastModified().toTime_t();
}

QString MT_ITEM::GetCssRelPath(int bi)
{
	// calculate the path to css from the given (this) document to the css file specified in the database
	// the path in the database can be either absolute or relative (relative to the root)
	QString doc = GetDocAbsPath(bi);
	QString css = theSln.GetCssAbsPath(bi);
	QString rel = ::GetRelPath(css, doc, true);
	return rel;
}

QString MT_ITEM::GetRelPath(MT_ITEM *item, int di)
{
	QString rel = ::GetRelPath(GetDocAbsPath(di), item->GetDocAbsPath(di), true);
	return rel;
}

QString MT_ITEM::GetRelUrl(MT_ITEM *item, int di)
{
	return GetRelPath(item, di) + "?" + GetGuid();
}


