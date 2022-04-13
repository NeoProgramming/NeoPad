#ifndef QUICKSTARTDLG_H
#define QUICKSTARTDLG_H

#include <QDialog>
#include "ui_quickstartdlg.h"

class QuickStartDlg : public QDialog
{
	Q_OBJECT
public:
	QuickStartDlg(QWidget *parent = 0);
	~QuickStartDlg();

	int  DoModal();
public slots:
	void onRadioOpen();
	void onRadioNew();
	void onOk();
	void onOverview();
	void onRemoveFromList();
private:
	Ui::QuickStartDlg ui;
};

#endif // QUICKSTARTDLG_H
