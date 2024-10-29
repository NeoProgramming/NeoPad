#ifndef SAVEALLDLG_H
#define SAVEALLDLG_H

#include <QDialog>
#include <QMdiSubWindow>
#include <QList>
#include <list>
#include "ui_savealldlg.h"
#include "../core/DocItem.h"

class MainWindow;

class SaveAllDlg : public QDialog
{
	Q_OBJECT

public:
	SaveAllDlg(QWidget *parent = 0);
	~SaveAllDlg();

	QList<QMdiSubWindow *> wl;
	std::list<DocItem*> mpl;

	bool DoModal(MainWindow *mw);
private:
	void DoSave(bool all, bool save);
public slots:
	void onSaveAll();
	void onSaveSel();
	void onSaveNothing();
private:
	Ui::SaveAllDlg ui;
};

#endif // SAVEALLDLG_H
