#include "pictogramdlg.h"
#include "../core/Solution.h"

PictogramDlg::PictogramDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	int i = 0;
	while (i < theSln.Picts.Tree.size()) {
		if (theSln.Picts.Tree[i].level == 0) {
			QTreeWidgetItem *topItem = new QTreeWidgetItem(ui.treeIcons);
			topItem->setText(0, theSln.Picts.Tree[i].name);
			topItem->setIcon(0, theSln.Picts.Tree[i].icon);
			topItem->setData(0, Qt::UserRole, i);
			++i;
			buildTreeLevel(topItem, 1, i);
		}
		else {
			++i; 
		}
	}

	connect(ui.pushCancel, &QPushButton::clicked, this, &PictogramDlg::reject);
}

PictogramDlg::~PictogramDlg()
{
}

void PictogramDlg::buildTreeLevel(QTreeWidgetItem *parentItem, int currentLevel, int &i) 
{
	while (i < theSln.Picts.Tree.size() && theSln.Picts.Tree[i].level == currentLevel) {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		parentItem->addChild(item);

		item->setText(0, theSln.Picts.Tree[i].name);
		item->setIcon(0, theSln.Picts.Tree[i].icon);
		item->setData(0, Qt::UserRole, i);
		++i;

		if (i < theSln.Picts.Tree.size() && theSln.Picts.Tree[i].level > currentLevel) {
			buildTreeLevel(item, currentLevel + 1, i);
		}
	}
}

int PictogramDlg::getPictogram()
{
	PictogramDlg dlg;
	if (dlg.exec() == QDialog::Accepted)
		return dlg.SelectedIndex;
	return 0;
}
