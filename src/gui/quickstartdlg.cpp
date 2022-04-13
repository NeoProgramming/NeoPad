#include "quickstartdlg.h"
#include <QFileDialog>
#include "../core/ini.h"
#include "../service/tools.h"



extern QTextCodec *codecUtf8;

QuickStartDlg::QuickStartDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.radioOpen, &QRadioButton::clicked, this, &QuickStartDlg::onRadioOpen);
	connect(ui.radioCreate, &QRadioButton::clicked, this, &QuickStartDlg::onRadioNew);
	connect(ui.pushOk, &QPushButton::clicked, this, &QuickStartDlg::onOk);
	connect(ui.toolOverview, &QToolButton::clicked, this, &QuickStartDlg::onOverview);
	connect(ui.toolRemove, &QToolButton::clicked, this, &QuickStartDlg::onRemoveFromList);
}

QuickStartDlg::~QuickStartDlg()
{

}

int QuickStartDlg::DoModal()
{
	for(auto item : INI::RecentProjects)
		ui.comboOpen->addItem( U16(item) );
	ui.comboOpen->setCurrentIndex(0);

	switch(INI::QSModeNew)
	{
	case 0:
		ui.radioOpen->setChecked(1);
		onRadioOpen();
		break;
	case 1:
		ui.radioCreate->setChecked(1);
		onRadioNew();
		break;
	}

	return this->exec();
}

void QuickStartDlg::onRadioOpen()
{
	ui.comboOpen->setEnabled(1);
	ui.toolOverview->setEnabled(1);
}

void QuickStartDlg::onRadioNew()
{
	ui.comboOpen->setEnabled(0);
	ui.toolOverview->setEnabled(0);
}

void QuickStartDlg::onOk()
{
	if(ui.radioOpen->isChecked())
	{	
		INI::CurrProjectPath = U8(ui.comboOpen->currentText());
		INI::QSModeNew = 0;
	}
	else if(ui.radioCreate->isChecked())
	{
		INI::CurrProjectPath = "";
		INI::QSModeNew = 1;
	}

	accept();
}

void QuickStartDlg::onOverview()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open project"),
		ui.comboOpen->currentText(),
		tr("VMBase files (*.vmbase)"));

	if (!fileName.isEmpty())
	{
		ui.comboOpen->setEditText(fileName);
	}
}

void QuickStartDlg::onRemoveFromList()
{
	int i = ui.comboOpen->currentIndex();
	if (i < 0)
		return;
	INI::RecentProjects.remove(U8(ui.comboOpen->currentText()));
	ui.comboOpen->removeItem(i);
	ui.comboOpen->setCurrentIndex(0);
}
