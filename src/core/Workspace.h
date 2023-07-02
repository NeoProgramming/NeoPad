#pragma once
#include <QSet>
#include <QList>
#include <QString>

class Workspace {
public:
	QSet<QString> DocItems;
	QSet<QString> FavItems;
	QList<QString> TabItems;
public:
	void Init();
	bool Load(const QString &basePath);
	bool Save(const QString &basePath);
};
