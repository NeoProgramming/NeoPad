#pragma once
#include <QString>
#include <QIcon>
#include <QVector>
#include <QMenu>
#include "TreeStatus.h"

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
private:
	QIcon    TreeIcons[(int)ETreeStatus::TS_ITEMS_COUNT];
	QIcon    LangIcons[(int)ELangStatus::LS_ITEMS_COUNT];
public:
	void    LoadStdIcons();
	void	LoadPicts(const QString &path);
	void    BuildMenu(QMenu *parentMenu);
	QIcon&  GetIcon(int i);
	QIcon&  GetIcon(ETreeStatus i);
	QIcon&  GetIcon(ELangStatus i);
	
private:
	void	loadDirectory(const QString &path, int level);
	void    buildMenuLevel(QMenu *parentMenu, int currentLevel, int &i);
};
