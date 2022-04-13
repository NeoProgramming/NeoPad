#include "existitemdlg.h"

ExistItemDlg::ExistItemDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));

}

ExistItemDlg::~ExistItemDlg()
{

}

int ExistItemDlg::DoModal()
{
	ui.lineTitle->setText(m_title);
	ui.lineFile->setText(m_file);
	ui.lineDir->setText(m_dir);

	return this->exec();
}

void ExistItemDlg::onOk()
{
	m_title = ui.lineTitle->text();
	m_file = ui.lineFile->text();
	m_dir = ui.lineDir->text();

	accept();
}

