#include <time.h>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QTextCodec>
#include <QCoreApplication>
#include <QUuid>
#include "../service/pugitools.h"
#include "../service/sys.h"
#include "../service/tools.h"
#include "../service/xini.h"
#include "../service/search.h"
#include "vmbsrv.h"
#include "Solution.h"
#include "ini.h"
#include "Cryptor.h"

extern QTextCodec *codecUtf8;

CSolution theSln;

CSolution::CSolution(void)
{
	m_BI = &Books;
	// constructor - zeroing only! * otherwise conflict with other global constructors)
	RemoveAll();
	char buf[1024];
    if (get_app_path(buf, sizeof(buf)) >= 0) {
		m_sProgDir = QFileInfo(buf).absolutePath();
        m_sPargPath = buf;
    }
    else {
		m_sProgDir = QDir::currentPath();
        m_sPargPath = "<<<UNDEFINED>>>";
    }
	m_bModify = 0;
}

CSolution::~CSolution(void)
{
}

void CSolution::loadSettings()
{
	XIni ini;
	ini.Load(codecUtf8->fromUnicode(m_sProgDir + "/settings.xml"), INI::Main);

	INI::RecentProjects.unique();
	INI::RecentProjects.remove_if([](const std::string &str) {return str.empty(); });
	if (INI::RecentProjects.size() > 10)
		INI::RecentProjects.resize(10);

	if (IsBlank(INI::HtmEditPath.c_str()))
		INI::HtmEditPath = "notepad.exe";
	if (IsBlank(INI::ExplorePath.c_str()))
		INI::ExplorePath = "explorer";
}

void CSolution::saveSettings()
{
	XIni ini;
	ini.Save(codecUtf8->fromUnicode(m_sProgDir + "/settings.xml"), INI::Main);
}

void CSolution::addProjectToRecent(const QString &path)
{
	// to make a separate class for storing settings? PrjSettings?
	std::string spath = U8(path);
	INI::RecentProjects.remove(spath);
	INI::RecentProjects.push_front(spath);
	if (INI::RecentProjects.size() > 10)
		INI::RecentProjects.resize(10);

	INI::CurrProjectPath = U8(path);
}

bool CSolution::CreateProject(const QString& name, const QString& dir, const QString &btitle0, const QString &bsuffix0)
{
	// create document
	pugi::xml_document xdoc;
	pugi::xml_node xRoot, xBase;
	MakeXmlDoc(xdoc, xRoot, xBase);

	// 
	if (!Documents::CreateProject(name, dir, xBase))
		return false;

	Books.AddBook(btitle0, bsuffix0, "", "", "");
	Books.SaveBooksInfo(xRoot);
	
	return true;
}

bool CSolution::LoadProject(const QString &fpath)
{
	// get extension
	QString ext = QFileInfo(fpath).suffix().toLower();
	if (ext != "neopad" && ext != "vmbase")
		return Fail("Bad extension"), false;

	// load xml
	pugi::xml_document xdoc;
	pugi::xml_node xRoot;
	if (!LoadXmlDoc(fpath, xdoc, xRoot))
		return false;

	// load books info - before main tree
	if (!Books.LoadBooksInfo(xRoot))
		return Fail("No bases found"), false;

	// load documents tree
	if (!Documents::LoadProject(fpath, xRoot))
		return false;
	
	// load favorites (after main tree)
	Favs.LoadFavorites(xRoot);

	// load paths from attributes (deprecated)
	m_ImageDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xRoot.attribute("images").as_string()));
	m_Snippets.m_SnippDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xRoot.attribute("snippets").as_string()));
	m_Snippets.LoadSnippets();

	// add to recent
	theSln.addProjectToRecent(fpath);

	return true;
}

bool CSolution::SaveProject(bool recursive)
{
	// create document
	pugi::xml_document xdoc;
	pugi::xml_node xRoot, xBase;
	MakeXmlDoc(xdoc, xRoot, xBase);

	// save vmb content
	if (!xBase)
		return false;

	// save documents tree
	Documents::SaveProject(recursive, xBase);

	// save books info
	Books.SaveBooksInfo(xRoot);

	// save paths as attributes (deprecated)
	set_attr(xRoot, "images").set_value(codecUtf8->fromUnicode(GetRelPath(m_ImageDir, m_RootDir, false)).constData());
	set_attr(xRoot, "snippets").set_value(codecUtf8->fromUnicode(GetRelPath(m_Snippets.m_SnippDir, m_RootDir, false)).constData());

	// save favorites
	//Favs.SaveFavorites(xRoot);

	// write file
	QString path = m_root->GetVmbAbsPath();
	if (SaveXmlDoc(path, xdoc)) {
		m_root->ChangeModify(false, true);
		return true;
	}
	return false;
}

