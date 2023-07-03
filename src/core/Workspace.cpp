#include "Workspace.h"
#include <QTextCodec>
#include "../service/pugitools.h"

extern QTextCodec *codecUtf8;

void Workspace::Init()
{
	DocItems.clear();
//	FavItems.clear();
	TabItems.clear();
	TabActive = "";
}

bool Workspace::Load(const QString &basePath)
{
	// load workspace
	pugi::xml_document xdoc;
	pugi::xml_node xroot, xbase, xitem;;
	QString fpath = basePath + ".workspace";
	if (!xdoc.load_file(codecUtf8->fromUnicode(fpath)))
		return false;
	xroot = xdoc.first_child();
	if (!xroot)
		return false;

	xbase = xroot.child("doc_items");
	xitem = xbase.first_child();
	while (xitem) {
		DocItems.insert(codecUtf8->toUnicode(xitem.text().as_string()));
		xitem = xitem.next_sibling();
	}

//	xbase = xroot.child("fav_items");
//	xitem = xbase.first_child();
//	while (xitem) {
//		FavItems.insert(codecUtf8->toUnicode(xitem.text().as_string()));
//		xitem = xitem.next_sibling();
//	}

	xbase = xroot.child("tab_items");
	xitem = xbase.first_child();
	while (xitem) {
		TabItems.push_back(codecUtf8->toUnicode(xitem.text().as_string()));
		xitem = xitem.next_sibling();
	}

	xbase = xroot.child("tab_active");
	xitem = xbase.first_child();
	if (xitem) {
		TabActive = codecUtf8->toUnicode(xitem.text().as_string());
	}

	return true;
}

bool Workspace::Save(const QString &basePath)
{
	// save workspace
	pugi::xml_document xdoc;
	pugi::xml_node xroot, xbase, xitem;

	pugi::xml_node decl = xdoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version").set_value("1.0");
	decl.append_attribute("encoding").set_value("utf-8");

	xroot = xdoc.append_child("workspace");
	set_attr(xroot, "version").set_value("1.0");

	xbase = xroot.append_child("doc_items");
	for (auto &i : DocItems) {
		xitem = xbase.append_child("item");
		xitem.text().set(codecUtf8->fromUnicode(i).constData());
	}

//	xbase = xroot.append_child("fav_items");
//	for (auto &i : FavItems) {
//		xitem = xbase.append_child("item");
//		xitem.text().set(codecUtf8->fromUnicode(i).constData());
//	}

	xbase = xroot.append_child("tab_items");
	for (auto &i : TabItems) {
		xitem = xbase.append_child("item");
		xitem.text().set(codecUtf8->fromUnicode(i).constData());
	}

	xbase = xroot.append_child("tab_active");
	xitem = xbase.append_child("item");
	xitem.text().set(codecUtf8->fromUnicode(TabActive).constData());

	QString fpath = basePath + ".workspace";
	if (xdoc.save_file(codecUtf8->fromUnicode(fpath))) {
		return true;
	}
	return false;
}
