#include "selectdocdlg.h"
#include <QFileDialog>

SelectDocDlg::SelectDocDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.linePath->setEnabled(false);
	ui.toolOverview->setEnabled(false);

	connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui.toolOverview, SIGNAL(clicked()), this, SLOT(onOverview()));
	connect(ui.radioMoveExisting, SIGNAL(clicked()), this, SLOT(onRadioMove()));
	connect(ui.radioCopyExisting, SIGNAL(clicked()), this, SLOT(onRadioCopy()));
	connect(ui.radioCreateNew, SIGNAL(clicked()), this, SLOT(onRadioNew()));

	m_Mode = MODE_NEW;
}

SelectDocDlg::~SelectDocDlg()
{

}

int SelectDocDlg::DoModal()
{
	ui.linePath->setText(m_DocPath);
	ui.radioCreateNew->setChecked(1);

	return this->exec();
}

void SelectDocDlg::onOk()
{
	if(ui.radioCreateNew->isChecked())
		m_DocPath = "";
	else // move & copy
		m_DocPath = ui.linePath->text();
	accept();
}

void SelectDocDlg::onOverview()
{
	m_DocPath = QFileDialog::getOpenFileName(this, tr("Select File..."),
		ui.linePath->text(),
		tr("Documents (*.htm *.html);;All Files (*)"));
	if(!m_DocPath.isEmpty())
	{
		ui.linePath->setText(m_DocPath);
	}
}

void SelectDocDlg::onRadioMove()
{
	m_Mode = MODE_MOVE;
	ui.linePath->setEnabled(true);
	ui.toolOverview->setEnabled(true);
}

void SelectDocDlg::onRadioNew()
{
	m_Mode = MODE_NEW;
	ui.linePath->setEnabled(false);
	ui.toolOverview->setEnabled(false);
}

void SelectDocDlg::onRadioCopy()
{
	m_Mode = MODE_COPY;
	ui.linePath->setEnabled(true);
	ui.toolOverview->setEnabled(true);
}

