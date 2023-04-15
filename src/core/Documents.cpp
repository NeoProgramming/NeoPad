#include "Documents.h"
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

ETreeStatus Documents::GetTreeStatus(const char* attr)
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

const char* Documents::GetTreeStatus(ETreeStatus status)
{
	if (status >= ETreeStatus::TS_ITEMS_COUNT)
		return "";
	if (!csStatusNames[(int)status])
		return "";
	return csStatusNames[(int)status];
}

bool Documents::SaveRootBase(bool recursive, pugi::xml_node xBase)
{
	// save project system to disk
	if (!m_root)
		return false;
		
	// save item data
	SaveItemData(xBase, m_root);
	SaveSubTag(xBase, m_root, recursive);
	
	return false;
}

bool Documents::MakeRootBase(const QString& name, const QString& dir, pugi::xml_node xBase)
{
	// create a new empty project with the given name at the given path
	RemoveAll();
	if (!CreateRoot(name, dir))
		return false;
	
	if (!MakeDoc(m_root, 0))
		return false;

    // save item data
    SaveItemData(xBase, m_root);
    SaveSubTag(xBase, m_root, true);

	return true;
}

void Documents::SaveItemData(pugi::xml_node txItem, DocItem *item)
{
	// save titles & timestamps; save docs
	set_attr(txItem, "guid").set_value(codecUtf8->fromUnicode(item->guid).constData());
	for (int i = 0; i < theSln.m_BI->BCnt(); i++) {
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



bool Documents::SaveSubBase(DocItem* node, bool recursive)
{
	// save (sub) project
	pugi::xml_document xdoc;
	pugi::xml_node xroot, xbase;

	// prepare document 
    theSln.MakeXml(xdoc, xroot, xbase);

    // if it is a root, additional tags need to be saved!
    // otherwise, all BooksInfo and Favorites will be lost!
    if (node == m_root)
        theSln.SaveProjectData(xroot);

	// save item data
    SaveItemData(xbase, node);

	// recursively save the entire tree
    SaveSubTag(xbase, node, recursive);

	// save
    QString path = node->GetVmbAbsPath();
    if (theSln.SaveXml(path, xdoc)) {
        node->ChangeModify(false, recursive);
		return true;
	}
	return false;
}

void Documents::SaveSubTag(pugi::xml_node pxParent, DocItem* tposParent, bool recursive)
{
	// save VMBase project level
	// recircive - recursive saving of all attached files (not needed for local modifications, 
	// for example, when renaming an item in a tree)
	for (auto pItem : tposParent->children) {
		// create a tag
		pugi::xml_node pxElem = pxParent.append_child("node");
		if (pxElem) {
			// if the item does not have its own vmbase file
			if (!pItem->p_subbase) {
				SaveItemData(pxElem, pItem);
				SaveSubTag(pxElem, pItem, recursive);
			}
			else {
				// id
				set_attr(pxElem, MBA::id).set_value(codecUtf8->fromUnicode(pItem->GetId()).constData());
				// recursive 
				if (recursive)
					SaveSubBase(pItem, true);
			}
		}
	}
}

DocItem * Documents::CreateRoot(const QString& name, const  QString& dir)
{
	// create root
	if (!QDir::isAbsolutePath(dir))
		return nullptr;	// error
	else
		m_RootDir = dir;
	DocItem *item = AddRoot();
	item->id = name;
	item->guid = QUuid::createUuid().toRfc4122().toHex();
	NormalizeFName(item->id);
	item->title[0] = !IsBlank(name) ? name : "noname";
	item->rdir = "";
	item->p_subbase = 1;
	return item;
}

void Documents::LoadItemData(pugi::xml_node txElem, DocItem *item)
{
	// element analysis
	// load an element based on data from an xml node
	const char *ct;
	struct tm t;

	item->guid = codecUtf8->toUnicode(txElem.attribute("guid").as_string());
	if (item->guid.isEmpty())
		item->guid = QUuid::createUuid().toRfc4122().toHex();

	for (int i = 0; i < theSln.m_BI->BCnt(); i++) {
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


bool Documents::LoadRootBase(const QString &fpath, pugi::xml_node xRoot)
{
	// remove previous tree
	RemoveAll();
	m_bModify = false;

	// root directory
	m_RootDir = QFileInfo(fpath).dir().canonicalPath();
	
	// load content (only after downloading the books!)
	CreateRoot("", m_RootDir);
	pugi::xml_node xelem = xRoot.child(MBA::node);
	if (!xelem)
		return Fail("solution node not found"), false;
	bool res = LoadSubTag(xelem, m_root);
		
	// load paths
	m_root->LoadItemPaths(fpath);

	return res;
}

bool Documents::LoadSubBase(const QString &id, DocItem* tpNode)
{
	// load the SUBBASE recursively into the tpNode node
    // ARGS: id   - the file that we upload ( NOT PATH !!! )
	//     tpNode - where do we load in the tree
	// fill in: while tpNode is not filled, you cannot use GetAbsDir!

	// building absolute path to vmbase file
	QString apath = tpNode->parent->GetAbsDir(-1) + "/" + id + "/" + id + MBA::extVmbase;

	// load
	pugi::xml_document xdoc;
	pugi::xml_node txElem;
    if (!theSln.LoadXml(apath, xdoc, txElem))
		return Fail("Error open xml file"), false;

    // rdir, id
	tpNode->LoadItemPaths(apath);

	tpNode->p_subbase = 1;
	// <vmbase><node>
	txElem = txElem.child(MBA::node);
	if (!txElem)
		return false;
	return LoadSubTag(txElem, tpNode);
}

bool Documents::LoadSubTag(pugi::xml_node txNode, DocItem* tpNode)
{
	// load INNER NODE recursively
	// ARGS: txPar - root tag in open xml file
	//       tpPar - the position of the parent in the single tree

	// fetching the title
	LoadItemData(txNode, tpNode);

	// child reading loop
	pugi::xml_node txElem = txNode.first_child();
	while (txElem)
	{
		// adding an empty element to the tree
		DocItem* tpItem = AddCTail(tpNode);

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

DocItem* Documents::AddItem(DocItem* tpPar, DocItem* tpAfter, const QString& title, const QString& id)
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

	if (QFileInfo(adirDoc + "/" + id + m_BI->GetDocExt(0)).exists())
		return Fail("html file already exist!"), nullptr;

	// insert into the tree; if tpAfter == null then end
	DocItem *item = (tpAfter == nullptr) ? AddCTail(tpPar) : AddAfter(tpAfter);
	if (!item)
		return Fail("error creating node in memory"), nullptr;

	item->p_subbase = 1;

	if (INI::DefItemStatus != 0)
		item->status = (ETreeStatus)INI::DefItemStatus;

	// identifier; just in case, normalize it
	item->id = id;
	NormalizeFName(item->id);

	// guid
	item->guid = QUuid::createUuid().toRfc4122().toHex();

	// title
	item->title[0] = IsBlank(title) ? id : title;

	// subfolder starting from the root of the base
	item->rdir = tpPar->GetBaseDir() + "/" + id;

	// creating a folder for .vmbase
	QDir().mkpath(adirVmb);	

	// create a NEW document for bi = 0
	MakeDoc(item, 0);

	// update the creation time of the document
	item->SetDocTime(0);

	// save changes
	HandleChanges(item, tpPar);

	return item;
}

void Documents::RemoveNodeDoc(DocItem* tpItem, int bi)
{
	// deleting a node document
	if (bi < 0 || bi >= theSln.m_BI->BCnt())
		return;
	QFile::remove(tpItem->GetDocAbsPath(bi));
	tpItem->title[bi].clear();
	tpItem->time[bi] = -1;
}

void Documents::RemoveNodeFiles(DocItem* tpItem)
{
	// recursively deleting node files
	std::vector<QString> d(theSln.m_BI->BCnt() + 1);
	for (int bi = 0; bi <= theSln.m_BI->BCnt(); bi++)
		d[bi] = tpItem->GetAbsDir(bi - 1);

	for (int bi = 0; bi < theSln.m_BI->BCnt(); bi++)
		QFile::remove(tpItem->GetDocAbsPath(bi));

	QFile::remove(tpItem->GetVmbAbsPath());
	for (auto tpChild : tpItem->children)
	{
		RemoveNodeFiles(tpChild);
	}

	for (int bi = 0; bi <= theSln.m_BI->BCnt(); bi++) {
		QDir dir(d[bi]);
		if (dir.count() == 2)
			dir.removeRecursively();
	}
}

bool Documents::RemoveNode(DocItem* node, bool del_files)
{
    DocItem* par = node->parent;
    if (!par)
		return false;
	if (del_files)
        RemoveNodeFiles(node);

    // nullify all fav-refs referring to node items
    ForEach(node, [](DocItem* item) {
        item->p_remove = 1;
    });
    theSln.Favs.ForEach(theSln.Favs.GetRoot(), [](FavItem * fav) {
       if(fav->ref && fav->ref->p_remove)
           fav->ref = nullptr;
    });
	
    PrjTree::RemoveNode(node);
	
    HandleChanges(par, false);
	return true;
}

bool Documents::MakeDoc(DocItem* tpItem, int bi)
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
	QByteArray a = U8a(m_BI->books[bi].save_prefix);
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

bool Documents::MoveUp(DocItem* tpItem)
{
	// move element up (swap with an overlying sibling)
	DocItem* tpPrev = GetPrevSibling(tpItem);
	if (tpPrev)
	{
		Exchange(tpItem, tpPrev);
		DocItem* tpPar = GetAncestorWithFile(tpItem, false);
		HandleChanges(tpPar, false);
		return 1;
	}
	return 0;
}

bool Documents::MoveDown(DocItem* tpItem)
{
	// move element down (swap with the underlying sibling)
	DocItem* tpNext = GetNextSibling(tpItem);
	if (tpNext)
	{
		Exchange(tpItem, tpNext);
		DocItem* tpPar = GetAncestorWithFile(tpItem, false);
		HandleChanges(tpPar, false);
		return 1;
	}
	return 0;
}

bool Documents::Move(DocItem* tpItem, DocItem* tpNewPar, DocItem* tpAfter)
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
	if (!MoveFiles(tpItem, tpNewPar))
		return false;

	// it should be understood that elements are pointers to structures;
	// a temporary element tpNew is created and a pointer to it is returned;

	// add a temporary element at the insertion position
	DocItem* tpNew = tpAfter ?
		AddAfter(tpAfter) :
		AddCTail(tpNewPar);

	// swap the added temporary element and the moved one in the tree
	// after swapping, the tpItem element will be in the position of the tpNew element;
	Exchange(tpNew, tpItem);

	// adjust the rdir paths of the moved node
    UpdateRelDirs(tpItem);

	// delete the old node; in the same place we set the Modify flag for the parent node
	RemoveNode(tpNew, false);

	// setting Modify for the node where you inserted
	HandleChanges(tpItem, tpNewPar);
	return true;
}

void Documents::UpdateRelDirs(DocItem* tpNode)
{
    // update relative directories ('rdir' fields) recursively for the whole node
	tpNode->UpdateBaseDir();
	for (auto tpChild : tpNode->children)
        UpdateRelDirs(tpChild);
}

bool Documents::MoveChild(DocItem* tpItem)
{
	// attach the child to the upper sibling; make the element the last child of its previous one
	DocItem* tpPrev = GetPrevSibling(tpItem);
	if (tpPrev)
		return Move(tpItem, tpPrev, NULL);
	return false;
}

bool Documents::MoveParent(DocItem* tpItem)
{
	// take out to the upper level; make it next after its parent
	DocItem* tpPar = tpItem->parent;
	if (tpPar && tpPar != m_root)
		return Move(tpItem, tpPar->parent, tpPar);
	return false;
}

void Documents::MakeUnsavedList(std::list<DocItem*> &mpl)
{
	// generate a list of FILE elements that have changes
	mpl.clear();
    if(theSln.Favs.m_bModify)
        mpl.push_back(m_root);
    MakeUnsavedLevel(GetRoot(), mpl);
}

DocItem* Documents::GetAncestorWithFile(DocItem* item, bool include_this)
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

void Documents::MakeUnsavedLevel(DocItem* mtNode, std::list<DocItem*> &mpl)
{
	// recursively building a list of unsaved nodes
	// if the node has the p_modify sign, and if this node corresponds to a unique file, then add its position to the list
	// do not add the same node twice
	if (!mtNode)
		return;
	if (mtNode->p_modify) {
		DocItem* mtFileNode = GetAncestorWithFile(mtNode, true);
		if (mtFileNode && std::find(mpl.begin(), mpl.end(), mtFileNode) == mpl.end())
			mpl.push_back(mtFileNode);
	}

	for (auto mtChild : mtNode->children) {
        MakeUnsavedLevel(mtChild, mpl);
	}
}

bool Documents::IsFNamesAvailable(DocItem* tpos, const QString & id)
{
	// check if create/move/rename is possible
	// doc dirs can match with vmbase dir and each other
	QString vmb = tpos->GetAbsDir(-1) + "/" + id + ".vmbase";
	QString dir = tpos->parent->GetAbsDir(-1) + "/" + id;

	if (QFileInfo(vmb).exists())
		return Fail("vmbase file with the same name already exists"), false;
	if (QFileInfo(dir).exists())
		return Fail("directory with the same name already exists"), false;

	for (int bi = 0; bi < m_BI->BCnt(); bi++) {
		QString doc = tpos->GetAbsDir(bi) + "/" + id + m_BI->GetDocExt(bi);
		if (QFileInfo(doc).exists())
			return Fail("document with the same name already exists"), false;
		if (m_BI->books[bi].path_is_unique) {
			QString dir = tpos->parent->GetAbsDir(bi) + "/" + id;
			if (QFileInfo(dir).exists())
				return Fail("directory with the same name already exists"), false;
		}
	}
	return true;
}

bool Documents::RenameItem(DocItem* tpos, const QString & id)
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
	for (int bi = 0; bi < m_BI->BCnt(); bi++) {
		QFile::rename(tpos->GetDocAbsPath(bi), tpos->GetAbsDir(bi) + "/" + id + m_BI->GetDocExt(bi));
		if (m_BI->books[bi].path_is_unique)
			QDir().rename(tpos->GetAbsDir(bi), tpos->parent->GetAbsDir(bi) + "/" + id);
	}
	// rename main dir
	QDir().rename(tpos->GetAbsDir(-1), dir);
	// change id
	tpos->id = id;
    UpdateRelDirs(tpos);
	HandleChanges(tpos, false);
	HandleChanges(tpos->parent, false);
	return true;
}

void  Documents::RenameTitle(DocItem* item, const QString & title, int bi)
{
	// change title
	if (bi < 0 || bi >= BCNT)
		return;
	item->title[bi] = title;
	HandleChanges(item, false);
}

void Documents::HandleChanges(DocItem* tpItem, bool recursive)
{
	tpItem->p_modify = 1;
	if (INI::AutoSavePages)
	{
		tpItem = GetAncestorWithFile(tpItem, true);
		SaveSubBase(tpItem, recursive);
	}
}

void Documents::HandleChanges(DocItem* tpItem1, DocItem* tpItem2)
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

void Documents::SetStatus(DocItem* item, ETreeStatus status, bool rec)
{
	item->status = status;
	item->p_modify = 1;
	if (rec) {
		for (auto child : item->children) {
			SetStatus(child, status, true);
		}
	}
	HandleChanges(item, rec);
}

bool Documents::MoveFiles(DocItem* tpItem, DocItem* tpNewPar)
{
	// move tpItem files to tpNewPar item folder
	int d = 1, n = theSln.m_BI->BCnt();

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
	for (int bi = -1; bi != n; bi += d) {
		if (bi < 0 || m_BI->books[bi].path_is_unique) {
			QDir dir;
			QString from = QDir::cleanPath(tpItem->GetAbsDir(bi));
			QString to = QDir::cleanPath(tpNewPar->GetAbsDir(bi));
			if (QFileInfo(from).exists()) {
				dir.mkpath(to);
				to = to + "/" + tpItem->GetId();
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

void Documents::SaveItem(DocItem* tpItem, int di)
{
	// update file save date
	tpItem->time[di] = time(NULL);
	HandleChanges(tpItem, false);
}

void Documents::GenContents(int bi, const QString &fpath, const QString &base)
{
	QFile file(fpath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	if (!m_root)
		return;

	GenContentsLevel(m_root, bi, file, base);

	file.close();
}

void Documents::GenContentsLevel(DocItem* node, int bi, QFile &file, const QString &base)
{
	int index = 0;
	int count = node->GetPublicChildrenCount();
	for (auto child : node->children) {
		if (!child->title[bi].isEmpty() && child->IsPublic()) {

			if (index == 0)
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

void Documents::EncryptDocs(DocItem* tposParent, const QString &oldPsw, const QString &newPsw)
{
	for (int bi = 0; bi < theSln.m_BI->BCnt(); bi++) {
		recryptFile(tposParent->GetDocAbsPath(bi), oldPsw, newPsw);
	}
	for (auto pItem : tposParent->children) {
		EncryptDocs(pItem, oldPsw, newPsw);
	}
}

void Documents::TransformDocs(int bi)
{
	ForEach([&](DocItem* pos) {
		TransformFile(pos, bi);
	});
	m_BI->SetLPrefix(bi);
}

DocItem* Documents::Locate(const QString &guid)
{
	// recursive search on tree
	if (guid.isEmpty())
		return nullptr;
	DocItem* result = nullptr;
	ForEach([&](DocItem* pos) {
		// search in title
		if (pos->guid == guid)
			result = pos;
	});
	return result;
}

void Documents::Search(const QString &text, unsigned int scope, DocItem* root, std::list<DocItem*> &results)
{
	// recursive search on tree
	ForEach(root, [&](DocItem* pos) {
		// search in tree
		if (scope & ESM_TREE) {
			if (pos->GetTitle(0).contains(text, Qt::CaseInsensitive))
				results.push_back(pos);
		}
		if(scope & ESM_ID) {
			if (pos->GetId().contains(text, Qt::CaseInsensitive))
				results.push_back(pos);
		}
		// search in file
		if (scope & (ESM_TEXT | ESM_TAG | ESM_ATTR)) {
			if (SearchInFile(pos, text, scope))
				results.push_back(pos);
		}
	});
}

bool Documents::TransformFile(DocItem* pos, int bi)
{
	QString html;
	if (!LoadDoc(pos, bi, html))
		return false;
	if (!SaveDoc(pos, bi, html))
		return false;
	return true;
}

bool Documents::SearchInFile(DocItem* pos, const QString &text, unsigned int scope)
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

bool Documents::LoadDoc(DocItem* item, int bi, QString &content)
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
	m_BI->RemovePrefix(bi, content);

	return true;
}

bool Documents::SaveDoc(DocItem* item, int bi, const QString &content)
{
	QByteArray s;

	// get path
	QString fpath = item->GetDocAbsPath(bi);
	fpath.replace("\\", "/");

	// add prefix
	if (!m_BI->books[bi].save_prefix.isEmpty())
		s = (m_BI->books[bi].save_prefix + content).toUtf8();
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
