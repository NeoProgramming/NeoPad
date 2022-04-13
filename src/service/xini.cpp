#include "xini.h"
#include <string.h>
#include "cstr.h"

bool XIni::Load(const char *fpath, XIniData *root)
{
	pugi::xml_document doc;
	if(IsBlank(fpath) || !root)
		return 0;
	if(!doc.load_file(fpath))
		return 0;
	pugi::xml_node r = doc.root();
	if (!root)
		return false;
	pugi::xml_node node = r.child("ini");
	if (!node)
		return false;

	LoadLevel(node, root);

	return 1;
}

bool XIni::Save(const char *fpath, XIniData *root)
{
	pugi::xml_document doc;
	if(IsBlank(fpath) || !root)
		return 0;
	if(!doc.load_file(fpath))
	{
		pugi::xml_node r = doc.root();

		pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
		decl.append_attribute("version") = "1.0";
		decl.append_attribute("encoding") = "utf-8";
		decl.append_attribute("standalone") = "no";

		pugi::xml_node node = r.append_child("ini");
	}
	pugi::xml_node r = doc.root();
	pugi::xml_node ini = r.child("ini");

	SaveLevel(ini, root);
	doc.save_file(fpath);
	return 1;
}

void XIni::LoadLevel(pugi::xml_node node, XIniData *ini)
{
	for(int i=0; ini[i].addr; i++)
	{
		// looking for an element
		pugi::xml_node elem = node.find_child_by_attribute("i", ini[i].id);
		if(!elem)
			continue;
		switch(ini[i].type)
		{
		case XIniData::T_GROUP:
			LoadLevel(elem, (XIniData*)(ini[i].addr));
			break;

		case XIniData::T_INT:
			*(int*)ini[i].addr = elem.attribute("v").as_int();
			break;
		case XIniData::T_STR:
			*(std::string*)ini[i].addr = elem.attribute("v").as_string();
			break;
		case XIniData::T_FLOAT:
			*(float*)ini[i].addr = elem.attribute("v").as_float();
			break;
		case XIniData::T_SLIST:
			LoadList(node, ini[i].id, (std::list<std::string> *)ini[i].addr);
			break;
		case XIniData::T_HEX:
			*(unsigned long*)ini[i].addr = strToHex(elem.attribute("v").as_string());
			break;
		}
	}
}

void XIni::LoadList(pugi::xml_node node, const char *id, std::list<std::string> *psl)
{
	pugi::xml_node elem = node.child("rec");
	while(elem)
	{
		psl->push_back(elem.attribute("v").as_string());
		elem = elem.next_sibling("rec");
	}
}

void XIni::SaveLevel(pugi::xml_node node, XIniData *ini)
{
	char buf[16];
	for(int i=0; ini[i].addr; i++)
	{
		// we are looking for an element with the corresponding name (so as not to erase what we do not read)
		bool cmt = 1;
		pugi::xml_node elem = node.find_child_by_attribute("i", ini[i].id);
		if(!elem)
		{
			elem = node.append_child("rec");
			elem.append_attribute("i").set_value(ini[i].id);
			elem.append_attribute("v");
		}
		pugi::xml_attribute a = elem.attribute("v");
		// value
		switch(ini[i].type)
		{
		case XIniData::T_GROUP:
			SaveLevel(elem, (XIniData*)(ini[i].addr));
			break;
		case XIniData::T_INT:
			a.set_value(*(int*)ini[i].addr);
			break;
		case XIniData::T_HEX:
			sprintf(buf, "%08X", *(int*)ini[i].addr);
			a.set_value(buf);
			break;
		case XIniData::T_STR:
			a.set_value(((std::string*)ini[i].addr)->c_str());
			break;
		case XIniData::T_FLOAT:
			a.set_value(*(float*)ini[i].addr);
			break;
		case XIniData::T_SLIST:
			SaveList(node, ini[i].id, (std::list<std::string>*)ini[i].addr);
			break;
		}
	}
}

void XIni::SaveList(pugi::xml_node node, const char *id, std::list<std::string> *psl)
{
	pugi::xml_node elem = node.child("rec");
	
	// overwrite old values
	auto i = psl->begin(), e = psl->end();
	while(elem && i!=e)
	{
		pugi::xml_attribute a = elem.attribute("i");
		if (!a) {
			a = elem.append_attribute("i");
			a.set_value(id);
		}
		const char *ii = a.value();
		if (ii && !strcmp(id, ii)) {
			a = elem.attribute("v");
			if (!a)
				a = elem.append_attribute("v");
			a.set_value(i->c_str());
		}
		elem = elem.next_sibling("rec");
		++i;
	}
	// if there are still values in the list, add
	if(i!=e)
	{
		while(i != e)
		{
			elem = node.append_child("rec");
			elem.append_attribute("i").set_value(id);
			elem.append_attribute("v").set_value(i->c_str());
			++i;
		}
	}
	// otherwise, remove the old values from the xml
	else
	{
		while(elem)
		{
			pugi::xml_node next = elem.next_sibling("rec");
			const char *i = elem.attribute("i").value();
			if(i && !strcmp(id, i))
				node.remove_child(elem);
			elem = next;
		}
	}
}

