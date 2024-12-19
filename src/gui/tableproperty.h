#ifndef __TABLEPROPERTY_H__
#define __TABLEPROPERTY_H__

#include "ui_tableproperty.h"


class TableProperties : public QDialog
{
    Q_OBJECT
public:
	int m_rowsCount = 0, m_colsCount = 0;
	QString m_classTable;
	QString m_classTR;
	QString m_classTD;
public:
	TableProperties(QWidget *parent = 0);
	int  DoModal(int rows, int cols, const QString &ctab, const QString &ctr, const QString &ctd);
    
public slots:
	void accept();
	
private:
	Ui::TableProperty ui;
	bool modeNewTable;
 
}; // class TableProperties

#endif // __TABLEPROPERTY_H__
