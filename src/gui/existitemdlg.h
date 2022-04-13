#ifndef EXISTITEMDLG_H
#define EXISTITEMDLG_H

#include <QDialog>
#include "ui_existitemdlg.h"

class ExistItemDlg : public QDialog
{
	Q_OBJECT

public:
	ExistItemDlg(QWidget *parent = 0);
	~ExistItemDlg();
	int DoModal();

	QString m_title, m_file, m_dir;

public slots:
	void onOk();
	
private:
	Ui::ExistItemDlg ui;
};

#endif // EXISTITEMDLG_H
