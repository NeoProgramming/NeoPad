#ifndef SYMBOLSDLG_H
#define SYMBOLSDLG_H

#include <QWidget>
#include <QListWidgetItem>
#include "ui_symbolsdlg.h"


class SymbolsDlg : public QDialog
{
	Q_OBJECT

public:
	SymbolsDlg(QWidget *parent = 0);
	~SymbolsDlg();
	int DoModal();

	QString m_Symbol;
public slots:
	void onOk();
	void onEdit();
	void onSelect(QListWidgetItem *item);
private:
	Ui::SymbolsDlg ui;
};

#endif // SYMBOLSDLG_H
