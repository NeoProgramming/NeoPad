#ifndef SELECTDOCDLG_H
#define SELECTDOCDLG_H

#include <QDialog>
#include "ui_selectdocdlg.h"

class SelectDocDlg : public QDialog
{
	Q_OBJECT
public:
	enum EMode
	{
		MODE_NEW,
		MODE_COPY,
		MODE_MOVE
	} m_Mode;
public:
	SelectDocDlg(QWidget *parent = 0);
	~SelectDocDlg();
	int DoModal();

	QString m_DocPath;	// path to existing doc or doc dir
public slots:
	void onOk();
	void onOverview();
	void onRadioMove();
	void onRadioNew();
	void onRadioCopy();
private:
	Ui::SelectDocDlg ui;
};

#endif // SELECTDOCDLG_H
