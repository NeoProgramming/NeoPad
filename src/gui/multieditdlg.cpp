#include "multieditdlg.h"
#include "ui_multieditdlg.h"

MultiEditDlg::MultiEditDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiEditDlg)
{
    ui->setupUi(this);
}

MultiEditDlg::~MultiEditDlg()
{
    delete ui;
}

int MultiEditDlg::DoModal(QString text)
{
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    ui->textEdit->setPlainText(text);
	ui->textEdit->setFont(fixedFont);
    setWindowTitle(m_title);
    return this->exec();
}

void MultiEditDlg::accept()
{
    m_text = ui->textEdit->toPlainText();
    QDialog::accept();
}

