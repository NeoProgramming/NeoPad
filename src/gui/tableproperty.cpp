#include <QColorDialog>
#include <QtGui>
#include "tableproperty.h"
#include "../service/tools.h"
#include "../core/Solution.h"

TableProperties::TableProperties(QWidget *parent)
     : QDialog(parent)
{	
	ui.setupUi(this);
}

int  TableProperties::DoModal(int rowsCount, int colsCount, const QString &ctab, const QString &ctr, const QString &ctd)
{
	ui.spinRows->setValue( rowsCount );
	ui.spinColumns->setValue( colsCount );

	// load all classes
	QStringList *L;
    ui.comboTableClass->addItem("");
    L = theSln.Clss.Get("table");
	if (L) {
		for (const QString&c : *L)
			ui.comboTableClass->addItem(c);
	}
    ui.comboTrClass->addItem("");
	L = theSln.Clss.Get("tr");
	if (L) {
		for (const QString&c : *L)
			ui.comboTrClass->addItem(c);
	}
    ui.comboTdClass->addItem("");
	L = theSln.Clss.Get("td");
	if (L) {
		for (const QString&c : *L)
			ui.comboTdClass->addItem(c);
	}

	// load current classes
	ui.comboTableClass->setCurrentText(ctab);
	ui.comboTrClass->setCurrentText(ctr);
	ui.comboTdClass->setCurrentText(ctd);
	return this->exec();
}

void TableProperties::accept()
{	
	// transfer the selected data to the table structure
	m_rowsCount = ui.spinRows->value();
	m_colsCount = ui.spinColumns->value();
	m_classTable = ui.comboTableClass->currentText();
	m_classTR = ui.comboTrClass->currentText();
	m_classTD = ui.comboTdClass->currentText();
	QDialog::accept();
}

