#pragma once
#include <string>
#include <list>
#include "../3rdparty/pugixml/pugixml.hpp"

// convert to simpler xml format
// tag name = variable name
// value = variable value
/*
<var1>100</var1>
<group1>
	<var2>hello</var2>
	<var2>2.71</var2>
</group1>
<list1>
	<path>c:/file1.txt</path>
	<path>c:/file2.txt</path>
</list1>
*/

// structure describing one ini-record
struct XIniData
{
	// data types
	enum XIniDataType
	{
		T_NONE,		// null
		T_GROUP,	// 

		T_INT,		// 
		T_HEX,		// 
		T_STR,		// 
		T_FLOAT,	// 
		
		T_SLIST,	// 
	};
	
	void *addr;		// variable address
	const char *id; // text identifier of the variable or path to the ini file
	int  type;		// data type
};

// macros representing XIniData records
#define XINI_GROUP(x)	    {  x,   #x,   XIniData::T_GROUP }		// group is used to connect child XIniData arrays
#define XINI_INT(x) 		{ &x,   #x,   XIniData::T_INT }			// a decimal integer
#define XINI_HEX(x) 		{ &x,   #x,   XIniData::T_HEX }			// a hexadecimal integer
#define XINI_STR(x) 		{ &x,   #x,   XIniData::T_STR }			// string
#define XINI_FLOAT(x)		{ &x,   #x,   XIniData::T_FLOAT }		// float number
#define XINI_SLIST(x)		{ &x,   #x,   XIniData::T_SLIST }		// list of strings



// a class that implements reading and writing ini files associated with XIniData structures
class XIni
{
public:
	bool Load(const char *fpath, XIniData *root);
	bool Save(const char *fpath, XIniData *root);
protected:
	void LoadLevel(pugi::xml_node node, XIniData *ini);
	void SaveLevel(pugi::xml_node node, XIniData *ini);
	void LoadList(pugi::xml_node node, const char *id, std::list<std::string> *psl);
	void SaveList(pugi::xml_node node, const char *id, std::list<std::string> *psl);
};
