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
Unicode   theUnicode;

CSolution::CSolution(void)
{
    m_BI = &Cols;
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

void CSolution::QuitProject()
{
	if (m_root)
		WS.Save(theSln.m_root->GetVmbAbsPath());
}

bool CSolution::MakeProject(const QString& name, const QString& dir, const QString &btitle0, const QString &bsuffix0)
{
	// add book before creating root node
    Cols.AddBook(CT_BASE, btitle0, bsuffix0, "", "", "");

	// create document
	pugi::xml_document xdoc;
	pugi::xml_node xRoot, xBase;
    MakeXml(xdoc, xRoot, xBase);

    if (!Documents::MakeRootBase(name, dir, xBase))
		return false;

	Favs.MakeRoot();
	WS.Init();

	return SaveProject(true);
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
    if (!LoadXml(fpath, xdoc, xRoot))
		return false;

    // load books info - before main tree!
    if (!Cols.LoadBooksInfo(xRoot))
		return Fail("No bases found"), false;

	// load documents tree
    if (!Documents::LoadRootBase(fpath, xRoot))
		return false;

    // update progress
    int ci = Cols.ProgressCol();
    if(ci>=0)
        GetRoot()->UpdateProgress(ci);
	
    // load favorites - after main tree!
	Favs.LoadFavorites(xRoot);

	// load classes
	Clss.Load(xRoot);

    // load paths from attributes (deprecated, refactor to Settings.SaveSettings(xRoot) )
	m_ImageDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xRoot.attribute("images").as_string()));
	m_Snippets.m_SnippDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xRoot.attribute("snippets").as_string()));
	m_Snippets.LoadSnippets();

	QString picts_dir = codecUtf8->toUnicode(xRoot.attribute("icons").as_string());
	if(!picts_dir.isEmpty())
		Picts.LoadPicts(m_RootDir + "/" + picts_dir);

	// add to recent
	theSln.addProjectToRecent(fpath);
	// load workspace
	WS.Load(fpath);

	return true;
}


void CSolution::SaveProjectData(pugi::xml_node xRoot)
{
    // needs for save project data from Documents.SaveSubBase

    // save books info
    Cols.SaveBooksInfo(xRoot);

    // save paths as attributes ( deprecated, refactor to Settings.SaveSettings(xRoot) )
    set_attr(xRoot, "images").set_value(codecUtf8->fromUnicode(GetRelPath(m_ImageDir, m_RootDir, false)).constData());
    set_attr(xRoot, "snippets").set_value(codecUtf8->fromUnicode(GetRelPath(m_Snippets.m_SnippDir, m_RootDir, false)).constData());
	set_attr(xRoot, "icons").set_value(codecUtf8->fromUnicode(GetRelPath(Picts.Dir, m_RootDir, false)).constData());

    // save favorites
    Favs.SaveFavorites(xRoot);

	// save classes
	//Clss.Save(xRoot);
}

bool CSolution::SaveProject(bool recursive)
{
	// create document
	pugi::xml_document xdoc;
	pugi::xml_node xRoot, xBase;

    MakeXml(xdoc, xRoot, xBase);

	// save vmb content
	if (!xBase)
		return false;

	// save documents tree
    Documents::SaveRootBase(recursive, xBase);

    // save project-specific data
    SaveProjectData(xRoot);
		
	QString fpath = m_root->GetVmbAbsPath();
	// save workspace
	WS.Save(fpath);
	// write file
    if (SaveXml(fpath, xdoc)) {
		m_root->ChangeModify(false, true);
		return true;
	}
	return false;
}

bool CSolution::LoadXml(const QString &fpath, pugi::xml_document &xdoc, pugi::xml_node &xroot)
{
    // psw enc
    //  0   0  load_file
    //  0   1  error
    //  1   0  load_file
    //  1   1  decrypt

    if (isEncrypted(fpath)) {
        if (m_Password.isEmpty()) {
            return Fail("Password not set"), false;
        }
        else {
            QByteArray data;
            if (!decryptFile(fpath, m_Password, data))
                return Fail("decryptFile() error"), false;
            if (!xdoc.load_string(data.constData()))
                return Fail("pugixml load_string() error"), false;
        }
    }
    else {
        if (!xdoc.load_file(codecUtf8->fromUnicode(fpath)))
            return Fail("pugixml load_file() error"), false;
    }


    // vmbase
    xroot = xdoc.first_child();
    if (!xroot)
        return Fail("root not found"), false;
    return true;
}

void CSolution::MakeXml(pugi::xml_document &xdoc, pugi::xml_node &xroot, pugi::xml_node &xbase)
{
    pugi::xml_node decl = xdoc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version").set_value("1.0");
    decl.append_attribute("encoding").set_value("utf-8");

    // adding an enclosing <vmbase> element (root of xml)
    xroot = xdoc.append_child("vmbase");
    set_attr(xroot, "version").set_value("1.0");

    // add the main <node> element
    xbase = xroot.append_child(MBA::node);
}


bool CSolution::SaveXml(const QString &path, const pugi::xml_document &xdoc)
{
    if (m_Password.isEmpty()) {
        if (xdoc.save_file(codecUtf8->fromUnicode(path))) {
        //   m_bModify = true;	//???
            return true;
        }
    }
    else {
        QByteArray plain;
        save_blob(xdoc, plain);
        if (encryptFile(path, m_Password, plain)) {
        //    m_bModify = true;	//???
            return true;
        }
    }

    return false;
}


QString CSolution::GetPrjTitle()
{
    DocItem* root = GetRoot();
    if (!root)
        return "";
    if (m_Password.isEmpty())
        return root->title[0];
    return root->title[0] + " (ENCRYPTED)";
}

QString CSolution::GetBookDir(int bi)
{
    return theSln.m_RootDir + "/" + theSln.Cols.books[bi].rpath;
}

QString CSolution::GetCssAbsPath(int bi)
{
    if (QFileInfo(Cols.books[bi].csspath).isAbsolute())
        return Cols.books[bi].csspath;
    return GetBookDir(bi) + "/" + Cols.books[bi].csspath;
}
