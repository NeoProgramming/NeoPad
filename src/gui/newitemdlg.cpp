#include "newitemdlg.h"

NewItemDlg::NewItemDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushOk, &QPushButton::clicked, this, &NewItemDlg::onOk);
	connect(ui.pushCancel, &QPushButton::clicked, this, &NewItemDlg::reject);
	connect(ui.lineId, &QLineEdit::textEdited, this, &NewItemDlg::onChangeId);
	connect(ui.lineTitle, &QLineEdit::textEdited, this, &NewItemDlg::onChangeTitle);
}

NewItemDlg::~NewItemDlg()
{

}

int NewItemDlg::DoModal()
{
	ui.lineTitle->setText(m_title);
	ui.lineId->setText(m_id);
	ui.lineId->selectAll();
	ui.lineId->setFocus();
	return this->exec();
}

void NewItemDlg::onOk()
{
	m_title = ui.lineTitle->text();
	m_id = ui.lineId->text();

	accept();
}

void NewItemDlg::onChangeId(const QString &name)
{
	QString c;
	c = ui.lineTitle->text();
	if(c.isEmpty() || start_editing)
	{
		m_title = name;
		ui.lineTitle->setText(m_title);
	}
}

void NewItemDlg::onChangeTitle(const QString &name)
{
	start_editing = false;
}
