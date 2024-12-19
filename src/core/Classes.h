#pragma once
#include "../service/pugitools.h"
#include <QList>
#include <QString>
#include <QStringList>
#include <QMap>

class Classes
{
	QMap<QString, QStringList> data;
public:
	bool	Load(pugi::xml_node txRoot);
	bool	Save(pugi::xml_node txRoot);
	QStringList* Get(const QString &name);
};
