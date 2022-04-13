#ifndef SNIPPETSDLG_H
#define SNIPPETSDLG_H

#include <QWidget>
#include <QListWidgetItem>
#include "ui_snippetsdlg.h"


class SnippetsDlg : public QDialog
{
	Q_OBJECT

public:
	SnippetsDlg(QWidget *parent = 0);
	~SnippetsDlg();
	int DoModal();

	QString* m_pSel;
public slots:
	void onOk();
	void onEdit();
	void onSelect(QListWidgetItem *item);
private:
	Ui::SnippetsDlg ui;
};

#endif // SNIPPETSDLG_H
