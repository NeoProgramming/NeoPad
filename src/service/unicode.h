#pragma once
#include <string>
#include <utility>
#include <list>
#include <QSharedPointer>

class Unicode {
public:
    struct Symbol {
        unsigned int code = 0;
        QString name;
    };
    struct Group {
        std::string name;
        std::list<std::pair<unsigned int, unsigned int> > ranges;
		std::list<Group> children;

        void     AddRange(unsigned int from, unsigned int to);
		void     AddRecent(unsigned int c);
		bool     LoadGroups(const QString& fpath);
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
	~Unicode();
    bool Load(const QString& fpath);
	bool LoadRecent(const QString& fpath);
	bool SaveRecent(const QString& fpath);
    QString GetName(unsigned int c);
	void FindByName(const QString& name, Unicode::Group &gr);
public:
    Group All;
    Group Recent;
    Group Faves;
    Group Search;
private:
    enum { Count = 0xF0000 };
	Symbol *Data = nullptr;

};
Q_DECLARE_METATYPE(Unicode::Group*)

