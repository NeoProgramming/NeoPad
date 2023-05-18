#include "prjpropsdlg.h"
#include <QFileDialog>

PrjPropsDlg::PrjPropsDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushOk, &QToolButton::clicked, this, &PrjPropsDlg::onOk);
	connect(ui.pushCancel, &QToolButton::clicked, this, &PrjPropsDlg::reject);
	
	connect(ui.toolSnippets, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewSnippets);
	connect(ui.toolImages, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewImages);

	connect(ui.toolBPath1, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewBPath1);
	connect(ui.toolBPath2, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewBPath2);
	connect(ui.toolBCSS1, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewBCSS1);
	connect(ui.toolBCSS2, &QToolButton::clicked, this, &PrjPropsDlg::onOverviewBCSS2);
}

PrjPropsDlg::~PrjPropsDlg()
{

}

int PrjPropsDlg::DoModal()
{
	ui.lineSnippets->setText(m_snippets);
	ui.lineImages->setText(m_images);

	ui.lineBName1->setText(m_bases[0].title);
	ui.lineBSuffix1->setText(m_bases[0].suffix);
	ui.lineBPath1->setText(m_bases[0].rpath);
	ui.lineBCSS1->setText(m_bases[0].csspath);
	ui.plainPrefix1->setPlainText(m_bases[0].save_prefix);

    if(m_bases.size()>=2) {
        ui.lineBName2->setText(m_bases[1].title);
        ui.lineBSuffix2->setText(m_bases[1].suffix);
        ui.lineBPath2->setText(m_bases[1].rpath);
        ui.lineBCSS2->setText(m_bases[1].csspath);
        ui.plainPrefix2->setPlainText(m_bases[1].save_prefix);
    }
	return this->exec();
}

void PrjPropsDlg::onOk()
{	
	m_snippets = ui.lineSnippets->text();
	m_images = ui.lineImages->text();

	m_bases[0].title = ui.lineBName1->text();
	m_bases[0].suffix = ui.lineBSuffix1->text();
	m_bases[0].rpath = ui.lineBPath1->text();
	m_bases[0].csspath = ui.lineBCSS1->text();
	m_bases[0].save_prefix = ui.plainPrefix1->toPlainText();
    if(m_bases.size()>=2) {
        m_bases[1].title = ui.lineBName2->text();
        m_bases[1].suffix = ui.lineBSuffix2->text();
        m_bases[1].rpath = ui.lineBPath2->text();
        m_bases[1].csspath = ui.lineBCSS2->text();
        m_bases[1].save_prefix = ui.plainPrefix2->toPlainText();
    }
	accept();
}

void PrjPropsDlg::onOverviewSnippets()
{
	QString dirName = QFileDialog::getExistingDirectory ( this, tr("Select snippets folder"));
	if (!dirName.isEmpty())
		ui.lineSnippets->setText(dirName);
}

void PrjPropsDlg::onOverviewImages()
{
	QString dirName = QFileDialog::getExistingDirectory ( this, tr("Select images folder"));
	if (!dirName.isEmpty())
		ui.lineImages->setText(dirName);
}

void PrjPropsDlg::onOverviewBPath1()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"));
	if (!path.isEmpty())
		ui.lineBPath1->setText(path);
}

void PrjPropsDlg::onOverviewBPath2()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"));
	if (!path.isEmpty())
		ui.lineBPath2->setText(path);
}

void PrjPropsDlg::onOverviewBCSS1()
{
	QString cssName = QFileDialog::getOpenFileName(this, tr("Select css file"));
	if (!cssName.isEmpty())
		ui.lineBCSS1->setText(cssName);
}

void PrjPropsDlg::onOverviewBCSS2()
{
	QString cssName = QFileDialog::getOpenFileName(this, tr("Select css file"));
	if (!cssName.isEmpty())
		ui.lineBCSS2->setText(cssName);
}

