#pragma once
#include <QSet>
#include <QList>
#include <QString>

class Workspace {
public:
	struct {
		QSet<QString> DocItems;
		//QSet<QString> FavItems;
		QList<QString> TabItems;
		QString TabActive;
	} Curr, Loaded;
public:
	void Init();
	bool Load(const QString &basePath);
	bool Save(const QString &basePath);
};
