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

const char* csStatusNames[] = {
	"unknown",
	"normal",
	"almost",
	"75",
	"50",
	"25",
	"unready",
	"locked",
	NULL
};

QString CSolution::GetDocExt(int bi)
{
	// get extension by document type
	if (bi < 0)
		return MBA::extVmbase;
	if (bi >= BCnt())
		return QString(".error") + MBA::extHtml;
	if(m_Bases[bi].suffix.isEmpty())
		return MBA::extHtml;
	return "." + m_Bases[bi].suffix + MBA::extHtml;
}

ETreeStatus CSolution::GetTreeStatus(const char* attr)
{
	if (IsBlank(attr))
		return ETreeStatus::TS_UNKNOWN;
	for (int i = 0; csStatusNames[i] && i < (int)ETreeStatus::TS_ITEMS_COUNT; i++)
	{
		if (!strcmp(attr, csStatusNames[i]))
			return (ETreeStatus)i;
	}
	return ETreeStatus::TS_UNKNOWN;
}

const char* CSolution::GetTreeStatus(ETreeStatus status)
{
	if (status >= ETreeStatus::TS_ITEMS_COUNT)
		return "";
	if (!csStatusNames[(int)status])
		return "";
	return csStatusNames[(int)status];
}

CSolution::CSolution(void)
{
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

void CSolution::LoadSettings()
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

void CSolution::SaveSettings()
{
	XIni ini;
	ini.Save(codecUtf8->fromUnicode(m_sProgDir + "/settings.xml"), INI::Main);
}

bool CSolution::SaveProject(bool recursive)
{
	// save project system to disk
	if (!m_root)
		return false;

	// create document
	pugi::xml_document xdoc;
	pugi::xml_node xRoot, xBase;
	MakeXmlDoc(xdoc, xRoot, xBase);

	// save paths as attributes (deprecated)
	set_attr(xRoot, "images").set_value(codecUtf8->fromUnicode(GetRelPath(m_ImageDir, m_RootDir, false)).constData());
	set_attr(xRoot, "snippets").set_value(codecUtf8->fromUnicode(GetRelPath(m_Snippets.m_SnippDir, m_RootDir, false)).constData());

	// save vmb content
	if (!xBase)
		return false;

	// save item data
	SaveItemData(xBase, m_root);
	SaveSubTag(xBase, m_root, recursive);

	// save bases
	SaveBasesInfo(xRoot);

	// write file
	QString path = m_root->GetVmbAbsPath();
	if (SaveXmlDoc(path, xdoc)) {
		m_root->ChangeModify(false, true);
		return true;
	}
	return false;
}

void CSolution::CreateProject(const QString& name, const QString& dir, const QString &btitle0, const QString &bsuffix0)
{
	// create a new empty project with the given name at the given path
	RemoveAll();
	CreateRoot(name, dir);
	AddBase(btitle0, bsuffix0, "", "", "");
	MakeDoc(m_root, 0);
	
	SaveProject(true);
	QString s = m_root->GetVmbAbsPath();
	addProjectToRecent(s);
}

void CSolution::SaveItemData(pugi::xml_node txItem, MT_ITEM *item)
{
	// save titles & timestamps; save docs
	set_attr(txItem, "guid").set_value(codecUtf8->fromUnicode(item->guid).constData());
	for (int i = 0; i < theSln.BCnt(); i++) {
		char sbuf[32];
		sprintf(sbuf, "title%d", i);
		set_attr(txItem, sbuf).set_value(codecUtf8->fromUnicode(item->GetTitle(i)).constData());

		char tbuf[50];
		if (item->time[i] >= 0) {
			QString tt = item->GetDocTimeStr(i);
			sprintf(sbuf, "time%d", i);
			set_attr(txItem, sbuf).set_value(codecUtf8->fromUnicode(tt).constData());
		}
	}

	// save status
	if (item->status != ETreeStatus::TS_UNKNOWN)
		set_attr(txItem, "s").set_value(GetTreeStatus(item->status));
}

void CSolution::MakeXmlDoc(pugi::xml_document &xdoc, pugi::xml_node &xroot, pugi::xml_node &xbase)
{
	pugi::xml_node decl = xdoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version").set_value("1.0");
	decl.append_attribute("encoding").set_value("utf-8");

	// adding an enclosing <vmbase> element (root of xml)
	xroot = xdoc.append_child("vmbase");
	set_attr(xroot, "version").set_value("1.0");

	// add the <node> element
	xbase = xroot.append_child(MBA::node);
}

bool CSolution::SaveXmlDoc(const QString &path, const pugi::xml_document &xdoc)
{
	if (m_Password.isEmpty()) {
		if (xdoc.save_file(codecUtf8->fromUnicode(path))) {
			m_bModify = true;	//???
			return true;
		}
	}
	else {
		QByteArray plain;
		save_blob(xdoc, plain);
		if (encryptFile(path, m_Password, plain)) {
			m_bModify = true;	//???
			return true;
		}
	}

	return false;
}

bool CSolution::SaveSubBase(MTPOS tpPar, bool recursive)
{
	// save (sub) project
	// if it is a root, additional tags need to be saved
	if (tpPar == m_root)
		return SaveProject(recursive);

	pugi::xml_document xdoc;
	pugi::xml_node xroot, xbase;

	// prepare document 
	MakeXmlDoc(xdoc, xroot, xbase);
	
	// save item data
	SaveItemData(xbase, tpPar);

	// recursively save the entire tree
	SaveSubTag(xbase, tpPar, recursive);

	// save
	QString path = tpPar->GetVmbAbsPath();
	if (SaveXmlDoc(path, xdoc)) {
		tpPar->ChangeModify(false, true);
		return true;
	}
	return false;
}

void CSolution::SaveSubTag(pugi::xml_node pxParent, MTPOS tposParent, bool recursive)
{
	// save VMBase project level
	// recircive - recursive saving of all attached files (not needed for local modifications, 
	// for example, when renaming an item in a tree)
	for (auto pItem : tposParent->children)
	{
		// create a tag
		pugi::xml_node pxElem = pxParent.append_child("node");
		if (pxElem)
		{
			// if the item does not have its own vmbase file
			if (!pItem->p_subbase)
			{
				SaveItemData(pxElem, pItem);
				SaveSubTag(pxElem, pItem, recursive);
			}
			else
			{
				// id
				set_attr(pxElem, MBA::id).set_value(codecUtf8->fromUnicode(pItem->GetId()).constData());
				// recursive 
				if (recursive)
					SaveSubBase(pItem, true);
			}
		}
	}
}

MTPOS CSolution::CreateRoot(const QString& name, const  QString& dir)
{
	// create root
	if (!QDir::isAbsolutePath(dir))
		m_RootDir = m_sProgDir + "/" + dir;
	else
		m_RootDir = dir;
	MT_ITEM *item = AddRoot();
	item->id = name;
	NormalizeFName(item->id);
	item->title[0] = !IsBlank(name) ? name : "noname";
	item->rdir = "";
	item->p_subbase = 1;
	return item;
}

void CSolution::LoadItemData(pugi::xml_node txElem, MT_ITEM *item, bool vmb)
{
	// element analysis
	// load an element based on data from an xml node
	const char *ct;
	struct tm t;

	item->guid = codecUtf8->toUnicode(txElem.attribute("guid").as_string());
	if (item->guid.isEmpty())
		item->guid = QUuid::createUuid().toRfc4122().toHex();
	

	for (int i = 0; i < theSln.BCnt(); i++) {
		char s[32];
		sprintf(s, "title%d", i);
		item->title[i] = codecUtf8->toUnicode(txElem.attribute(s).as_string());

		sprintf(s, "time%d", i);
		ct = txElem.attribute(s).as_string();
		memset(&t, 0, sizeof(t));

		item->SetDocTime(ct, i);
	}

	item->status = GetTreeStatus(txElem.attribute(MBA::status).as_string());
}

void CSolution::AddBase(const QString &title, const QString &suffix, const QString &rpath, 
	const QString &csspath, const QString &prefix)
{
	NeopadBase base;
	base.title = title;
	base.suffix = suffix;
	base.rpath = rpath;
	base.csspath = csspath;
	base.load_prefix = prefix;
	base.save_prefix = prefix;

	if (base.rpath.isEmpty())
		base.rpath = ".";
	if (base.csspath.isEmpty())
		base.csspath = "default.css";

	// unique path for base
	base.path_is_unique = QDir::cleanPath(base.rpath) != "." &&
		!std::count_if(&m_Bases[0], &m_Bases[BCNT], [&base](const NeopadBase&b)->bool {
			return QDir::cleanPath(base.rpath) == QDir::cleanPath(b.rpath);
		}
	);

	// only one element without prefix is possible 
	if (base.suffix.isEmpty()) {
		for (int i = 0; i < m_BasesCnt; i++)
			if (m_Bases[i].suffix.isEmpty())
				return;
	}

	// add to bases
	m_Bases[m_BasesCnt] = base;
	m_BasesCnt++;
}

void CSolution::LoadBasesInfo(pugi::xml_node txRoot)
{
	pugi::xml_node txBases = txRoot.child("bases");
	if (!txBases)
		return;
	
	m_BasesCnt = 0;
	pugi::xml_node txBase = txBases.first_child();
	while (txBase && m_BasesCnt<BCNT) {
		
		AddBase(
			U16(txBase.attribute("title").as_string()),
			U16(txBase.attribute("suffix").as_string()),
			U16(txBase.attribute("rpath").as_string()),
			U16(txBase.attribute("csspath").as_string()),
			U16(txBase.attribute("prefix").as_string())
			);

		txBase = txBase.next_sibling();
	}
}

void CSolution::SaveBasesInfo(pugi::xml_node txRoot)
{
	pugi::xml_node txBases = txRoot.append_child("bases");
	if (!txBases)
		return;
	for (const NeopadBase& base : m_Bases) {
		pugi::xml_node txBase = txBases.append_child("base");
		set_attr(txBase, "title").set_value(U8a(base.title).constData());
		set_attr(txBase, "suffix").set_value(U8a(base.suffix).constData());
		set_attr(txBase, "rpath").set_value(U8a(base.rpath).constData());
		set_attr(txBase, "csspath").set_value(U8a(base.csspath).constData());
		set_attr(txBase, "prefix").set_value(U8a(base.save_prefix).constData());
	}
}

bool CSolution::LoadXmlDoc(const QString &fpath, pugi::xml_document &xdoc, pugi::xml_node &xroot)
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

bool CSolution::LoadProject(const QString &fpath)
{
	// get extension
	QString ext = QFileInfo(fpath).suffix().toLower();
	if (ext != "neopad" && ext != "vmbase")
		return Fail("Bad extension"), false;
		
	// load
	pugi::xml_document xdoc;
	pugi::xml_node xroot;
	if (!LoadXmlDoc(fpath, xdoc, xroot))
		return false;
	
	RemoveAll();
	m_bModify = false;

	// root directory
	m_RootDir = QFileInfo(fpath).dir().canonicalPath();

	// bases
	LoadBasesInfo(xroot);
	if (m_BasesCnt < 1)
		return Fail("No bases found"), false;

	// load content (only after downloading the bases!)
	CreateRoot("", m_RootDir);
	pugi::xml_node xelem = xroot.child(MBA::node);
	if (!xelem)
		return Fail("solution node not found"), false;
	bool res = LoadSubTag(xelem, m_root);

	// load paths
	m_root->LoadItemPaths(fpath);

	// load paths from attributes (deprecated)
	m_ImageDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xroot.attribute("images").as_string()));
	m_Snippets.m_SnippDir = QDir::cleanPath(m_RootDir + "/" + codecUtf8->toUnicode(xroot.attribute("snippets").as_string()));
	m_Snippets.LoadSnippets();

	// add to recent projects
	if (res)
		addProjectToRecent(fpath);

	return res;
}

bool CSolution::LoadSubBase(const QString &id, MTPOS tpNode)
{
	// load the SUBBASE recursively into the tpNode node
	// ARGS: file - the file that we upload (DO NOT WAY !!!)
	//     tpNode - where do we load in the tree
	// fill in: while tpNode is not filled, you cannot use GetAbsDir!

	// building absolute path to vmbase file
	QString apath = tpNode->parent->GetAbsDir(-1) + "/" + id + "/" + id + MBA::extVmbase;

	// load
	pugi::xml_document xdoc;
	pugi::xml_node txElem;
	if (!LoadXmlDoc(apath, xdoc, txElem))
		return false;
	
	tpNode->LoadItemPaths(apath);

	tpNode->p_subbase = 1;
	// <vmbase><node>
	txElem = txElem.child(MBA::node);
	if (!txElem)
		return false;
	return LoadSubTag(txElem, tpNode);
}

bool CSolution::LoadSubTag(pugi::xml_node txNode, MTPOS tpNode)
{
	// load INNER NODE recursively
	// ARGS: txPar - root tag in open xml file
	//       tpPar - the position of the parent in the single tree

	// fetching the title
	LoadItemData(txNode, tpNode, false);

	// child reading loop
	pugi::xml_node txElem = txNode.first_child();
	while (txElem)
	{
		// adding an empty element to the tree
		MTPOS tpItem = AddCTail(tpNode);

		// see what kind of element is in xml
		if (txElem.child(MBA::node))
			LoadSubTag(txElem, tpItem);
		else if (auto id = txElem.attribute(MBA::id).as_string())
			LoadSubBase(id, tpItem);

		// next item
		txElem = txElem.next_sibling();
	}

	return true;
}

MTPOS CSolution::AddItem(MTPOS tpPar, MTPOS tpAfter, const QString& title, const QString& id)
{
	// add child to tree
	if (!tpPar)
		return nullptr;
	if (IsBlank(id))
		return nullptr;

	// check to see if we overwrite something
	QString adirVmb = tpPar->GetAbsDir(-1) + "/" + id;
	QString adirDoc = tpPar->GetAbsDir(0) + "/" + id;

	if (QFileInfo(adirVmb + "/" + id + ".vmbase").exists()) 
		return Fail("vmbase file already exist!"), nullptr;
	
	if (QFileInfo(adirDoc + "/" + id + GetDocExt(0)).exists()) 
		return Fail("html file already exist!"), nullptr;
		
	MT_ITEM item;
	item.p_subbase = 1;
	
	// identifier; just in case, normalize it
	item.id = id;
	NormalizeFName(item.id);

	// guid
	item.guid = QUuid::createUuid().toRfc4122().toHex();

	// title
	item.title[0] = IsBlank(title) ? id : title;

	// subfolder starting from the root of the base
	item.rdir = tpPar->GetBaseDir() + "/" + id;

	// creating a folder for .vmbase
	QDir().mkpath(adirVmb);

	// insert into the tree; if tpAfter == null then end
	MTPOS tpos = nullptr;
	if (tpAfter == nullptr)
		tpos = AddCTail(tpPar, &item);
	else
		tpos = AddAfter(tpAfter, &item);

	if (!tpos)
		return nullptr;
	
	// create a NEW document for bi = 0
	MakeDoc(tpos, 0);

	// update the creation time of the document
	tpos->SetDocTime(0);

	// save changes
	HandleChanges(tpos, tpPar);
	
	return tpos;
}

void CSolution::RemoveNodeDoc(MTPOS tpItem, int bi)
{
	// deleting a node document
	if (bi < 0 || bi >= theSln.BCnt())
		return;
	QFile::remove(tpItem->GetDocAbsPath(bi));
	tpItem->title[bi].clear();
	tpItem->time[bi] = -1;
}

void CSolution::RemoveNodeFiles(MTPOS tpItem)
{
	// recursively deleting node files
	std::vector<QString> d(theSln.BCnt() + 1);
	for (int bi = 0; bi <= theSln.BCnt(); bi++)
		d[bi] = tpItem->GetAbsDir(bi - 1);

	for (int bi = 0; bi < theSln.BCnt(); bi++)
		QFile::remove(tpItem->GetDocAbsPath(bi));

	QFile::remove(tpItem->GetVmbAbsPath());
	for (auto tpChild : tpItem->children)
	{
		RemoveNodeFiles(tpChild);
	}

	for (int bi = 0; bi <= theSln.BCnt(); bi++) {
		QDir dir(d[bi]);
		if (dir.count() == 2)
			dir.removeRecursively();
	}
}

bool CSolution::RemoveNode(MTPOS tpItem, bool del_files)
{
	MTPOS tpPar = tpItem->parent;
	if (!tpPar)
		return false;
	if (del_files)
		RemoveNodeFiles(tpItem);

	tpItem->RemoveChildren();
	if (tpItem->parent) {
		tpItem->parent->children.remove(tpItem);
	}
	else {
		m_root = nullptr;
	}
	delete tpItem;

	HandleChanges(tpPar, false);
	return true;
}

bool CSolution::MakeDoc(MTPOS tpItem, int bi)
{
	// ensure the folder exists for the document
	QString path = tpItem->GetAbsDir(bi);
	QDir().mkpath(path);

	// create document
	path = tpItem->GetDocAbsPath(bi);
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
		return false;

	path = tpItem->GetCssRelPath(bi);
	QByteArray a = U8a(m_Bases[bi].save_prefix);
	file.write(a);
	file.write("<html>\n");
	file.write("<head>\n");
	file.write("<link rel=\"stylesheet\" type=\"text/css\" href=\"");
	

	file.write(U8a(path));

	file.write("\">\n");
	file.write("<meta content=\"text/html; charset=utf-8\" http-equiv=Content-Type>\n");
	file.write("</head>\n");
    file.write("<body><p>\n");
    file.write("</p></body>\n");
	file.write("</html>\n");
	file.close();
	return true;
}

bool CSolution::MoveUp(MTPOS tpItem)
{
	// move element up (swap with an overlying sibling)
	MTPOS tpPrev = GetPrevSibling(tpItem);
	if (tpPrev)
	{
		Exchange(tpItem, tpPrev);
		MTPOS tpPar = GetAncestorWithFile(tpItem, false);
		HandleChanges(tpPar, false);
		return 1;
	}
	return 0;
}

bool CSolution::MoveDown(MTPOS tpItem)
{
	// move element down (swap with the underlying sibling)
	MTPOS tpNext = GetNextSibling(tpItem);
	if (tpNext)
	{
		Exchange(tpItem, tpNext);
		MTPOS tpPar = GetAncestorWithFile(tpItem, false);
		HandleChanges(tpPar, false);
		return 1;
	}
	return 0;
}


bool CSolution::Move(MTPOS tpItem, MTPOS tpNewPar, MTPOS tpAfter)
{
	// move an element; make it OR the last child of tpNewPar, OR after tpAfter
	// insertion as the first element or insertion before the selected one is deliberately excluded as unclaimed in real software
	// moving an item includes moving the vmb file (if any) and moving documents

	// there is no point in moving the node to itself
	if (tpItem == tpNewPar)
		return true;
	// you cannot move an ancestor to its descendant
	if (tpItem->IsAncestor(tpNewPar))
		return Fail("Moved node is ancestor of new parent node"), false;	

	// transfer files first
	if (! MoveFiles(tpItem, tpNewPar))
		return false;

	// it should be understood that elements are pointers to structures;
	// a temporary element tpNew is created and a pointer to it is returned;

	// add a temporary element at the insertion position
	MTPOS tpNew = tpAfter ?
		AddAfter(tpAfter) :
		AddCTail(tpNewPar);

	// swap the added temporary element and the moved one in the tree
	// after swapping, the tpItem element will be in the position of the tpNew element;
	Exchange(tpNew, tpItem);

	// adjust the rdir paths of the moved node
	UpdateBaseDirs(tpItem);

	// delete the old node; in the same place we set the Modify flag for the parent node
	RemoveNode(tpNew, false);	
		
	// setting Modify for the node where you inserted
	HandleChanges(tpItem, tpNewPar);
	return true;
}

void CSolution::UpdateBaseDirs(MTPOS tpNode)
{
	// update base directories recursively for the whole node
	tpNode->UpdateBaseDir();
	for (auto tpChild : tpNode->children)
		UpdateBaseDirs(tpChild);
}

bool CSolution::MoveChild(MTPOS tpItem)
{
	// attach the child to the upper sibling; make the element the last child of its previous one
	MTPOS tpPrev = GetPrevSibling(tpItem);
	if (tpPrev)
		return Move(tpItem, tpPrev, NULL);
	return false;
}

bool CSolution::MoveParent(MTPOS tpItem)
{
	// take out to the upper level; make it next after its parent
	MTPOS tpPar = tpItem->parent;
	if (tpPar && tpPar != m_root)
		return Move(tpItem, tpPar->parent, tpPar);
	return false;
}

void CSolution::MakeUnsavedList(CMtposList &mpl)
{
	// generate a list of FILE elements that have changes
	mpl.clear();
	MakeUnsavedListR(mpl, GetRoot());
}

MTPOS CSolution::GetAncestorWithFile(MTPOS item, bool include_this)
{
	// find parent with file
	if (!include_this)
		item = item->parent;
	while (item) {
		if (item->p_subbase)
			return item;
		item = item->parent;
	}
	return GetRoot();
}



void CSolution::MakeUnsavedListR(CMtposList &mpl, MTPOS mtNode)
{
	// recursively building a list of unsaved nodes
	// if the node has the p_modify sign, and if this node corresponds to a unique file, then add its position to the list
	// do not add the same node twice
	if (!mtNode)
		return;
	if (mtNode->p_modify)
	{
		MTPOS mtFileNode = GetAncestorWithFile(mtNode, true);
		if (mtFileNode && std::find(mpl.begin(), mpl.end(), mtFileNode) == mpl.end())
			mpl.push_back(mtFileNode);
	}

	for (auto mtChild : mtNode->children)
	{
		MakeUnsavedListR(mpl, mtChild);
	}
}

bool CSolution::IsFNamesAvailable(MTPOS tpos, const QString & id)
{
	// check if create/move/rename is possible
	// doc dirs can match with vmbase dir and each other
	QString vmb = tpos->GetAbsDir(-1) + "/" + id + ".vmbase";
	QString dir = tpos->parent->GetAbsDir(-1) + "/" + id;

	if (QFileInfo(vmb).exists())
		return Fail("vmbase file with the same name already exists"), false;
	if (QFileInfo(dir).exists())
		return Fail("directory with the same name already exists"), false;
	
	for (int bi = 0; bi < BCnt(); bi++) {
		QString doc = tpos->GetAbsDir(bi) + "/" + id + GetDocExt(bi);
		if (QFileInfo(doc).exists())
			return Fail("document with the same name already exists"), false;
		if (m_Bases[bi].path_is_unique) {
			QString dir = tpos->parent->GetAbsDir(bi) + "/" + id;
			if (QFileInfo(dir).exists())
				return Fail("directory with the same name already exists"), false;
		}
	}
	return true;
}

bool CSolution::RenameItem(MTPOS tpos, const QString & id)
{
	if (id == tpos->GetId())
		return true;
	if (!IsFNamesAvailable(tpos, id))
		return false;

	QString vmb = tpos->GetAbsDir(-1) + "/" + id + ".vmbase";
	QString dir = tpos->parent->GetAbsDir(-1) + "/" + id;

	// rename vmbase file
	QFile::rename(tpos->GetVmbAbsPath(), vmb);
	// rename docs & doc dirs
	for (int bi = 0; bi < BCnt(); bi++) {
		QFile::rename(tpos->GetDocAbsPath(bi), tpos->GetAbsDir(bi) + "/" + id + GetDocExt(bi));
		if (m_Bases[bi].path_is_unique)
			QDir().rename(tpos->GetAbsDir(bi), tpos->parent->GetAbsDir(bi) + "/" + id);
	}
	// rename main dir
	QDir().rename(tpos->GetAbsDir(-1), dir);
	// change id
	tpos->id = id;
	UpdateBaseDirs(tpos);
	HandleChanges(tpos, false);
	return true;
}

void  CSolution::RenameTitle(MTPOS item, const QString & title, int bi)
{
	// change title
	if (bi < 0 || bi >= theSln.BCnt())
		return;
	item->title[bi] = title;
	HandleChanges(item, false);
}


void CSolution::HandleChanges(MTPOS tpItem, bool recursive)
{
	tpItem->p_modify = 1;
	if (INI::AutoSavePages)
	{
		tpItem = GetAncestorWithFile(tpItem, true);
		SaveSubBase(tpItem, recursive);
	}
}

void CSolution::HandleChanges(MTPOS tpItem1, MTPOS tpItem2)
{
	tpItem1->p_modify = 1;
	tpItem2->p_modify = 1;
	if (INI::AutoSavePages)
	{
		tpItem1 = GetAncestorWithFile(tpItem1, true);
		tpItem2 = GetAncestorWithFile(tpItem2, true);
		SaveSubBase(tpItem1, false);
		if (tpItem1 != tpItem2)
			SaveSubBase(tpItem2, false);
	}
}


void CSolution::SetStatus(MTPOS item, ETreeStatus status, bool rec)
{
	item->status = status;
	item->p_modify = 1;
	if (rec)
	{
		for (MTPOS child : item->children)
		{
			SetStatus(child, status, true);
		}
	}
	HandleChanges(item, rec);
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

bool CSolution::MoveFiles(MTPOS tpItem, MTPOS tpNewPar)
{
	// move tpItem files to tpNewPar item folder
	int d = 1, n = theSln.BCnt();

	// first, let's just check that the target paths do not exist
	for (int bi = -1; bi < n; bi++) {
		QString pathOld = QDir::cleanPath(tpItem->GetAbsDir(bi));
		QFileInfo fiOld = QFileInfo(pathOld);
		QString pathNew = QDir::cleanPath(tpNewPar->GetAbsDir(bi) + "/" + tpItem->GetId());
		QFileInfo fiNew = QFileInfo(pathNew);
		if (fiNew.exists())
			return Fail(QString("Path '%1' already exists").arg(pathNew)), false;
		if (pathOld == pathNew)
			return Fail(QString("Paths are equal ('%1')").arg(pathNew)), false;
	}

	// now we transfer
	for (int bi = -1; bi != n; bi+=d) {
		if (bi < 0 || m_Bases[bi].path_is_unique) {
			QDir dir;
			QString from = QDir::cleanPath(tpItem->GetAbsDir(bi));
			QString to = QDir::cleanPath(tpNewPar->GetAbsDir(bi));
			if (QFileInfo(from).exists()) {
				dir.mkpath(to);
				to = to +"/" + tpItem->GetId();
				if (!dir.rename(from, to)) {
					// rollback
				//	if (d > 0) {
				//		d = -1;
				//		n = -2;
				//	}
				//	else {
						return Fail("Rename error"), false;
				//	}
				}
			}
		}
	}

	return true;
}

void CSolution::SaveItem(MTPOS tpItem, int di)
{
	// update file save date
	tpItem->time[di] = time(NULL);
	HandleChanges(tpItem, false);
}

void CSolution::GenContents(int bi, const QString &fpath, const QString &base)
{
	QFile file(fpath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	if (!m_root)
		return;

	GenContentsLevel(m_root, bi, file, base);

	file.close();
}

void CSolution::GenContentsLevel(MTPOS node, int bi, QFile &file, const QString &base)
{
	int index = 0;
	int count = node->GetPublicChildrenCount();
	for (MTPOS child : node->children) {
        if (!child->title[bi].isEmpty() && child->IsPublic() ) {
			
			if (index==0)
				file.write("\n<ul class='Container'>");

			file.write("\n<li id='");
			file.write(U8a(child->GetGuid()));
			file.write("' ");
			file.write("class='Node");
			if (child->GetPublicChildrenCount())
				file.write(" ExpandClosed");
			else
				file.write(" ExpandLeaf");
			if (index == count - 1)
				file.write(" IsLast");
			file.write("'>\n");

			file.write("<div class='Expand'></div>\n");
			
			file.write("<div class='Content'>");
				file.write("<span class='NodeImage'></span>");
				file.write("<a target='content' href='");
				file.write(U8a(base));
				file.write(U8a(child->GetDocRelPath(bi)));
				file.write("'>");
				file.write(U8a(child->GetTitle(bi)));
				file.write("</a>");
			file.write("</div>");
			
			GenContentsLevel(child, bi, file, base);

			file.write("\n</li>");
			index++;
		}
	}

	if (index > 0)
		file.write("\n</ul>");
}

QString CSolution::GetBaseDir(int bi)
{
	return theSln.m_RootDir + "/" + theSln.m_Bases[bi].rpath;
}

QString CSolution::GetPrjTitle()
{
	MTPOS root = GetRoot();
	if (!root)
		return "";
	if (m_Password.isEmpty())
		return root->title[0];
	return root->title[0] + " (ENCRYPTED)";
}

QString CSolution::GetCssAbsPath(int bi)
{
	if (QFileInfo(theSln.m_Bases[bi].csspath).isAbsolute())
		return theSln.m_Bases[bi].csspath;
	return GetBaseDir(bi) + "/" + theSln.m_Bases[bi].csspath;
}

void CSolution::EncryptDocs(MTPOS tposParent, const QString &oldPsw, const QString &newPsw)
{
	for (int bi = 0; bi < theSln.BCnt(); bi++) {
		recryptFile(tposParent->GetDocAbsPath(bi), oldPsw, newPsw);
	}
	for (auto pItem : tposParent->children) {
		EncryptDocs(pItem, oldPsw, newPsw);
	}
}

void CSolution::TransformDocs(int bi)
{
	ForEach([&](MTPOS pos) {
		TransformFile(pos, bi);
	});
	m_Bases[bi].load_prefix = m_Bases[bi].save_prefix;
}

MTPOS CSolution::Locate(const QString &guid)
{
	// recursive search on tree
	MTPOS result = nullptr;
	ForEach([&](MTPOS pos) {
		// search in title
		if (pos->guid == guid)
			result = pos;
	});
	return result;
}

void CSolution::Search(const QString &text, unsigned int scope, CMtposList &results)
{
    // recursive search on tree
    ForEach([&](MTPOS pos) {
        // search in title
		if (scope & ESM_TREE) {
			if (pos->GetTitle(0).contains(text, Qt::CaseInsensitive))
				results.push_back(pos);
		}
        // search in file
		if (scope & (ESM_TEXT|ESM_TAG|ESM_ATTR)) {
			if (SearchInFile(pos, text, scope))
				results.push_back(pos);
		}
    });
}

bool CSolution::TransformFile(MTPOS pos, int bi)
{
	QString html;
	if (!LoadDoc(pos, bi, html))
		return false;
	if (!SaveDoc(pos, bi, html))
		return false;
	return true;
}

bool CSolution::SearchInFile(MTPOS pos, const QString &text, unsigned int scope)
{
	QByteArray data;
	QString html;
	QString fpath = pos->GetDocAbsPath(0);
	if (void *wnd = m_pCB->FindOpenedDoc(pos, 0)) {
		m_pCB->GetDocData(wnd, html);
	}
	else if (!LoadDoc(pos, 0, html)) {
		return false;
	}
    
    return search(html, text, scope);
}

bool CSolution::LoadDoc(MTPOS item, int bi, QString &content)
{
	QString fpath = item->GetDocAbsPath(bi);
	fpath.replace("\\", "/");

	// load file
	QByteArray data;
	if (isEncrypted(fpath)) {
		if (m_Password.isEmpty())
			return Fail("Password not set"), false;
		if (!decryptFile(fpath, m_Password, data))
			return Fail("decryptFile() error"), false;
		content = QString::fromUtf8(data);
	}
	else {
		QFile file(fpath);
		if (!file.open(QIODevice::ReadOnly))
			return Fail("file.open() error"), false;
		data = file.readAll();
		file.close();
		content = QString::fromUtf8(data);
	}

	// remove prefix
	if (!m_Bases[bi].load_prefix.isEmpty()) {
		if (content.startsWith(m_Bases[bi].load_prefix))
			content = content.mid(m_Bases[bi].load_prefix.length());
	}

	return true;
}

bool CSolution::SaveDoc(MTPOS item, int bi, const QString &content)
{
	QByteArray s;

	// get path
	QString fpath = item->GetDocAbsPath(bi);
	fpath.replace("\\", "/");

	// add prefix
	if(!m_Bases[bi].save_prefix.isEmpty())
		s = (m_Bases[bi].save_prefix + content).toUtf8();
	else
		s = content.toUtf8();

	// save
	bool success = false;
	if (theSln.m_Password.isEmpty()) {
		QFile file(fpath);
		if (success = file.open(QIODevice::WriteOnly))
			file.write(s);
	}
	else {
		success = encryptFile(fpath, m_Password, s);
	}

	if (success)
		SaveItem(item, bi);

	m_bModify = true;	//???
	return true;
}

