#ifndef NEWITEMDLG_H
#define NEWITEMDLG_H

#include <QDialog>
#include "ui_newitemdlg.h"

class NewItemDlg : public QDialog
{
	Q_OBJECT

public:
	NewItemDlg(QWidget *parent = 0);
	~NewItemDlg();
	int DoModal();
	bool start_editing = true;
	QString m_title, m_id;
public slots:
	void onOk();
	void onChangeId(const QString &text);
	void onChangeTitle(const QString &text);
private:
	Ui::NewItemDlg ui;
};

#endif // NEWITEMDLG_H
