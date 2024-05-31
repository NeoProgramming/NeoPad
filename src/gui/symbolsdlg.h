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
    void onSelectGroup(QTreeWidgetItem *item);
    void onDoubleClickSymbol(QTableWidgetItem *item);
    void onSplitterMoved(int pos, int index);
    void onFontChanged(const QString &text);
	void onSearch();
private:
    void resizeEvent(QResizeEvent* event);
    void Done(QTableWidgetItem *item);
    void LoadLevel(QTreeWidgetItem *node, Unicode::Group *group);
	void LoadGroup(Unicode::Group* gr);
    void ResizeTable();

	Ui::SymbolsDlg ui;
    QFont m_font;
    int m_cols, m_rows;		// total cells
    int m_col, m_row;		// last cell
    enum { Q_Columns = 8 };
};

#endif // SYMBOLSDLG_H
