#pragma once

#include <QDialog>
#include "ui_passworddlg.h"

class PasswordDlg : public QDialog
{
	Q_OBJECT

public:
	PasswordDlg(QWidget *parent = Q_NULLPTR);
	~PasswordDlg();
	int DoEnterPassword();
	int DoSetPassword();
	int DoChangePassword();
public:
	QString m_psw0, m_psw1, m_psw2, m_psw3;
public slots:
	void onOk();
private:
	Ui::PasswordDlg ui;
	QPalette pal;
};
