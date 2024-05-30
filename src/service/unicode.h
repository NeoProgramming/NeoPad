#pragma once
#include <string>
#include <utility>
#include <list>
#include <QSharedPointer>

class Unicode {
public:
    struct Symbol {
        unsigned int code = 0;
        std::string name;
    };
    struct Group {
        std::string name;
        std::list<std::pair<unsigned int, unsigned int> > ranges;
		std::list<Group> children;

        Group*   AddGroup(const char *name);
        bool     LoadGroups(const char *fpath);
		unsigned int GetCount();
		
		struct Iterator {
			friend Group;
			unsigned int operator*() const;
			Iterator& operator++();
			bool operator==(const Iterator& other) const;
			bool operator!=(const Iterator& other) const;
		private:
			std::list<std::pair<unsigned int, unsigned int> >::iterator ri, re;
			unsigned int rc;
		};

		Group::Iterator begin();
		Group::Iterator end();
    };	
public:
	Unicode();
    bool Load(const char* fpath);
public:
    Group All;
    Group Recent;
    Group Faves;
private:
    enum { Count = 0xF0000 };
    Symbol Data[Count];

};
Q_DECLARE_METATYPE(Unicode::Group*)

