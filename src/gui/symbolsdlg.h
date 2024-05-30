#ifndef SYMBOLSDLG_H
#define SYMBOLSDLG_H

#include <QWidget>
#include <QListWidgetItem>
#include "ui_symbolsdlg.h"
#include "../service/unicode.h"

class SymbolsDlg : public QDialog
{
	Q_OBJECT

public:
    SymbolsDlg(int cols, QWidget *parent = 0);
	~SymbolsDlg();
	int DoModal();

	QString m_Symbol;
public slots:
	void onOk();
	void onEdit();
    void onSelect(QTreeWidgetItem *item);
private:
    void LoadLevel(QTreeWidgetItem *node, Unicode::Group *group);
	void LoadGroup(Unicode::Group* gr);

	Ui::SymbolsDlg ui;
    int m_cols, m_rows;		// total cells
    int m_col, m_row;		// last cell
};

#endif // SYMBOLSDLG_H
