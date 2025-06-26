#include "newitemdlg.h"
#include "../core/DocItem.h"
#include "../core/Solution.h"

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
    ui.checkOpen->setChecked(m_open);
	
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_READY), tr("Ready"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_ALMOST), tr("Almost ready"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_75), tr("75 %"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_50), tr("50 %"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_25), tr("25 %"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_UNREADY), tr("Under construction"));
	ui.comboStatus->addItem(theSln.Picts.GetIcon(ETreeStatus::TS_LOCKED), tr("Locked"));
		
	ui.comboStatus->setCurrentIndex(m_status - (int)ETreeStatus::TS_READY);
	return this->exec();
}

void NewItemDlg::onOk()
{
	m_title = ui.lineTitle->text();
	m_id = ui.lineId->text();
	m_open = ui.checkOpen->isChecked();
	m_status = ui.comboStatus->currentIndex() + (int)ETreeStatus::TS_READY;
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
