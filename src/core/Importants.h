#pragma once
#include "../service/pugitools.h"
#include <QList>
#include <QString>

class Importants
{
public:
	QList<QString> AutoLoadedItems;
public:
	void	Load(pugi::xml_node txRoot);
	void	Save(pugi::xml_node txRoot);
	void	Add(const QString &guid);
	void	Del(const QString &guid);
	bool    Contains(const QString &guid);
	void	Toggle(const QString &guid);
};
