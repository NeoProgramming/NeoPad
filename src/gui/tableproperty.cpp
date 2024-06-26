#include <QColorDialog>
#include <QtGui>
#include "tableproperty.h"
#include "../service/tools.h"

TableProperties::TableProperties(QWidget *parent)
     : QDialog(parent)
{	
	ui.setupUi(this);
}

int  TableProperties::DoModal(int rowsCount, int colsCount, const QString &cls)
{
	ui.spinRows->setValue( rowsCount );
	ui.spinColumns->setValue( colsCount );
	ui.comboClass->setCurrentText(cls);
	return this->exec();
}

void TableProperties::accept()
{	
	// transfer the selected data to the table structure
	m_rowsCount = ui.spinRows->value();
	m_colsCount = ui.spinColumns->value();
	m_class = ui.comboClass->currentText();
	QDialog::accept();
}

