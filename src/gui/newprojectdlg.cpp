#include "newprojectdlg.h"
#include <QFileDialog>

NewProjectDlg::NewProjectDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushOk,		 SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.toolOverview, SIGNAL(clicked()), this, SLOT(onOverview()));
	connect(ui.pushCancel,	 SIGNAL(clicked()), this, SLOT(reject()));
}

NewProjectDlg::~NewProjectDlg()
{

}

int NewProjectDlg::DoModal()
{
	return this->exec();
}

void NewProjectDlg::onOk()
{
	m_name = ui.lineTitle->text();
	m_base = ui.lineBase->text();
	m_suffixes[0] = ui.lineSuffix1->text();
	m_suffixes[1] = ui.lineSuffix2->text();
	accept();
}

void NewProjectDlg::onOverview()
{
	QString dirName = QFileDialog::getExistingDirectory ( this, tr("Select base folder"));
	if (!dirName.isEmpty())
	{
		ui.lineBase->setText(dirName);
	}
}
