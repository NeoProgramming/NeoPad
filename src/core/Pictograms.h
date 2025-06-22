#pragma once
#include <QString>
#include <QIcon>
#include <QVector>
#include <QMenu>

class Pictograms
{
public:
	struct PictNode {
		QString name;
		QIcon icon;
		int level;
	};
public:
	QVector<PictNode> Tree;
	QString  Dir;		// path to directory with icons
public:
	void	LoadPicts(const QString &path);
	void    BuildMenu(QMenu *parentMenu);
private:
	void	loadDirectory(const QString &path, int level);
	void    buildMenuLevel(QMenu *parentMenu, int currentLevel, int &i);
};
