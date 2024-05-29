#include "symbolsdlg.h"
#include <QTextCodec>
#include <QMessageBox>

#include "../core/Solution.h"
#include "../core/ini.h"
#include "../service/tools.h"

extern QTextCodec *codecUtf8;

Q_DECLARE_METATYPE(QString *)

SymbolsDlg::SymbolsDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui.pushEdit, SIGNAL(clicked()), this, SLOT(onEdit()));
	connect(ui.listSnippets, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onSelect(QListWidgetItem*)));
}

SymbolsDlg::~SymbolsDlg()
{

}

int SymbolsDlg::DoModal()
{
	for (auto &s : theSln.m_Snippets.m_SnippList)
	{
		QListWidgetItem *item = new QListWidgetItem();
		item->setText(s);
		item->setData(Qt::UserRole, QVariant::fromValue(&s));
		ui.listSnippets->addItem(item);
	}

	return this->exec();
}

void SymbolsDlg::onOk()
{
	QListWidgetItem *item = ui.listSnippets->currentItem();
	if(item)
	{
		//m_pSel = item->data(Qt::UserRole).value<QString*>();
		accept();
	}
}

void SymbolsDlg::onEdit()
{
	// open the selected snippet in an external editor
	QListWidgetItem *item = ui.listSnippets->currentItem();
	if(item)
	{
		QString *path = item->data(Qt::UserRole).value<QString *>();
		QString  cmd = codecUtf8->toUnicode(INI::HtmEditPath.c_str());
		OpenInExternalApplication(this, cmd, *path);
	}
}

void SymbolsDlg::onSelect(QListWidgetItem *item)
{
	onOk();
}
