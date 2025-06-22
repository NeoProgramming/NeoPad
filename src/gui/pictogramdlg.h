#pragma once

#include <QDialog>
#include "ui_pictogramdlg.h"

class PictogramDlg : public QDialog
{
	Q_OBJECT

public:
	PictogramDlg(QWidget *parent = Q_NULLPTR);
	~PictogramDlg();
	static int getPictogram();
private:
	void buildTreeLevel(QTreeWidgetItem *parentItem, int currentLevel, int &i);
private:
	Ui::PictogramDlg ui;
	int SelectedIndex;
};
