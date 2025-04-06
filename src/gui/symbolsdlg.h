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
    SymbolsDlg(QWidget *parent = 0);
	~SymbolsDlg();
	int DoModal();

	QString m_Symbol;
public slots:
	void onOk();
    void onSelectGroup(QTreeWidgetItem *item);
	void onClickSymbol(QTableWidgetItem *item);
    void onDoubleClickSymbol(QTableWidgetItem *item);
	void onRightClickSymbol(const QPoint & pos);
    void onSplitterMoved(int pos, int index);
    void onFontChanged(const QString &text);
	void onSearch();
	void onCopy();
	void onAdd();
	void onPostInit();
	void onItemAddToQuick();
public:
	static void LoadGroup(Unicode::Group* gr, QTableWidget *tableSymbols, QFont &font);
	static void ResizeTable(QTableWidget *tableSymbols, QFont &font);
private:
    void resizeEvent(QResizeEvent* event);
    void Done(QTableWidgetItem *item);
    void LoadLevel(QTreeWidgetItem *node, Unicode::Group *group);	

	Ui::SymbolsDlg ui;
    QFont m_font;
	QMenu* m_contextMenu;
    enum { Q_Columns = 8 };
};

#endif // SYMBOLSDLG_H
