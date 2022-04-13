#ifndef SAVEALLDLG_H
#define SAVEALLDLG_H

#include <QDialog>
#include <QMdiSubWindow>
#include <QList>
#include "ui_savealldlg.h"

#include "../core/PrjItem.h"

class MainWindow;

class SaveAllDlg : public QDialog
{
	Q_OBJECT

public:
	SaveAllDlg(QWidget *parent = 0);
	~SaveAllDlg();

	QList<QMdiSubWindow *> wl;
	CMtposList mpl;

	bool DoModal(MainWindow *mw);
private:
	void DoSave(bool all);
public slots:
	void onSaveAll();
	void onSaveSel();
private:
	Ui::SaveAllDlg ui;
};

#endif // SAVEALLDLG_H
