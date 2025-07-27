#include "Pictograms.h"
#include <QDir>
#include "vmbsrv.h"

void Pictograms::LoadStdIcons()
{
	TreeIcons[(int)ETreeStatus::TS_UNKNOWN] = QIcon(":/treeicons/images/ti-unknown.png");
	TreeIcons[(int)ETreeStatus::TS_READY] = QIcon(":/treeicons/images/ti-html.png");
	TreeIcons[(int)ETreeStatus::TS_ALMOST] = QIcon(":/treeicons/images/ti-htmlx.png");
	TreeIcons[(int)ETreeStatus::TS_75] = QIcon(":/treeicons/images/ti-html75.png");
	TreeIcons[(int)ETreeStatus::TS_50] = QIcon(":/treeicons/images/ti-html50.png");
	TreeIcons[(int)ETreeStatus::TS_25] = QIcon(":/treeicons/images/ti-html25.png");
	TreeIcons[(int)ETreeStatus::TS_UNREADY] = QIcon(":/treeicons/images/ti-html0.png");
	TreeIcons[(int)ETreeStatus::TS_LOCKED] = QIcon(":/treeicons/images/ti-locked.png");
	TreeIcons[(int)ETreeStatus::TS_PINNED] = QIcon(":/treeicons/images/ti-pinned.png");
	TreeIcons[(int)ETreeStatus::TS_QUICK] = QIcon(":/treeicons/images/ti-quick.png");
	TreeIcons[(int)ETreeStatus::TS_FOLDER] = QIcon(":/treeicons/images/ti-folder.png");
	TreeIcons[(int)ETreeStatus::TS_EMPTY] = QIcon(":/treeicons/images/ti-htmlempty.png");

	LangIcons[(int)ELangStatus::LS_NONE] = QIcon(":/langicons/images/li-none.png");
	LangIcons[(int)ELangStatus::LS_OK] = QIcon(":/langicons/images/li-ok.png");
	LangIcons[(int)ELangStatus::LS_OLD] = QIcon(":/langicons/images/li-old.png");
	LangIcons[(int)ELangStatus::LS_QOK] = QIcon(":/langicons/images/li-qok.png");
	LangIcons[(int)ELangStatus::LS_QOLD] = QIcon(":/langicons/images/li-qold.png");
}

QIcon& Pictograms::GetIcon(int i)
{
	return TreeIcons[i];
}

QIcon& Pictograms::GetIcon(ETreeStatus i)
{
	return TreeIcons[(int)i];
}

QIcon& Pictograms::GetIcon(ELangStatus i)
{
	return LangIcons[(int)i];
}


void Pictograms::LoadPicts(const QString &path)
{
	Dir = path;
	Tree.clear();
	loadDirectory(path, 0);
}

void Pictograms::loadDirectory(const QString &path, int level) 
{
	const int iconSize = 20;
	QDir dir(path);
	if (!dir.exists()) 
		return;
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::Name | QDir::DirsFirst);
	QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	for (const QFileInfo &entry : entries) {
		PictNode node;
		node.name = entry.fileName();
		node.level = level;

		if (!entry.isDir()) {
			QString ext = entry.suffix().toLower();
			if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
				QImage img(entry.filePath());
				if (!img.isNull()) {
					img = img.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					node.icon = QIcon(QPixmap::fromImage(img));
				}
			}
		}

		Tree.append(node);

		if (entry.isDir()) {
			loadDirectory(entry.absoluteFilePath(), level + 1);
		}
	}
}

void Pictograms::BuildMenu(QMenu *parentMenu)
{
	if (Tree.isEmpty()) 
		return;	
	int i = 0;
	while (i < Tree.size()) {
		if (Tree[i].level == 0) {
			QMenu *topMenu = new QMenu(Tree[i].name);
			parentMenu->addMenu(topMenu);
			++i;
			buildMenuLevel(topMenu, 1, i);
		}
		else {
			++i; // skip incorrect level
		}
	}
}
/*
	{"File", 1},
		{"New", 2},
		{"Open", 2},
		{"Recent Files", 2},
			{"file1.txt", 3},
			{"file2.txt", 3},
		{"Save", 2},
	{"Edit", 1},
		{"Copy", 2},
		{"Paste", 2},
	{"Help", 1},
*/

void Pictograms::buildMenuLevel(QMenu *parentMenu, int currentLevel, int &i)
{
	while (i < Tree.size() && Tree[i].level == currentLevel) {
		const PictNode &node = Tree[i];

		// Let's see if there is a sublevel ahead
		if (i + 1 < Tree.size() && Tree[i + 1].level > currentLevel) {
			QMenu *subMenu = new QMenu(node.name, parentMenu);
			parentMenu->addMenu(subMenu);
			++i;
			buildMenuLevel(subMenu, currentLevel + 1, i);
		}
		else {
			QAction *action = new QAction(node.name, parentMenu);
			parentMenu->addAction(action);
			++i;
		}
	}
}
