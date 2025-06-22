#include "newitemdlg.h"
#include "../core/DocItem.h"

NewItemDlg::NewItemDlg(QIcon *statusIcons, QWidget *parent)
	: m_pStatusIcons(statusIcons)
	, QDialog(parent)
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
	
	ui.comboStatus->addItem(tr("Ready"));
	ui.comboStatus->addItem(tr("Almost ready"));
	ui.comboStatus->addItem(tr("75 %"));
	ui.comboStatus->addItem(tr("50 %"));
	ui.comboStatus->addItem(tr("25 %"));
	ui.comboStatus->addItem(tr("Under construction"));
	ui.comboStatus->addItem(tr("Locked"));
	ui.comboStatus->addItem(tr("Important"));

	if (m_pStatusIcons) {
		ui.comboStatus->setItemIcon(0, m_pStatusIcons[(int)ETreeStatus::TS_READY]);
		ui.comboStatus->setItemIcon(1, m_pStatusIcons[(int)ETreeStatus::TS_ALMOST]);
		ui.comboStatus->setItemIcon(2, m_pStatusIcons[(int)ETreeStatus::TS_75]);
		ui.comboStatus->setItemIcon(3, m_pStatusIcons[(int)ETreeStatus::TS_50]);
		ui.comboStatus->setItemIcon(4, m_pStatusIcons[(int)ETreeStatus::TS_25]);
		ui.comboStatus->setItemIcon(5, m_pStatusIcons[(int)ETreeStatus::TS_UNREADY]);
		ui.comboStatus->setItemIcon(6, m_pStatusIcons[(int)ETreeStatus::TS_LOCKED]);
	}
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
