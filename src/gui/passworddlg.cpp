#include "passworddlg.h"
#include <QMovie>
#include <QMessageBox>

PasswordDlg::PasswordDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	pal = palette();
	pal.setColor(QPalette::Background, Qt::black); 
	pal.setColor(QPalette::WindowText, Qt::green);
	pal.setColor(QPalette::Button, Qt::darkRed);
	pal.setColor(QPalette::ButtonText, Qt::yellow);
	pal.setColor(QPalette::Base, Qt::darkBlue);
	pal.setColor(QPalette::Text, Qt::yellow);
	
	setAutoFillBackground(true); 
	setPalette(pal); 

	ui.linePsw1->setPalette(pal);
	ui.linePsw2->setPalette(pal);
	ui.linePsw3->setPalette(pal);

	ui.pushOk->setPalette(pal);
	ui.pushCancel->setPalette(pal);

	QMovie *movie = new QMovie(":/ani/images/skull.gif");
	if (!movie->isValid())
		QMessageBox::warning(this, "err", "err");
	movie->setParent(this);
	ui.labelMovie->setMovie(movie);
	movie->start();

	connect(ui.pushOk, &QPushButton::clicked, this, &PasswordDlg::onOk);
	connect(ui.pushCancel, &QPushButton::clicked, this, &PasswordDlg::reject);
}

PasswordDlg::~PasswordDlg()
{
}

int PasswordDlg::DoEnterPassword()
{
	ui.labelPsw2->hide();
	ui.linePsw2->hide();
	ui.labelPsw3->hide();
	ui.linePsw3->hide();
	setWindowTitle("Enter password");
	ui.linePsw1->setFocus();
	int r = this->exec();

	return r;
}

int PasswordDlg::DoSetPassword()
{
	ui.labelPsw3->hide();
	ui.linePsw3->hide();
	setWindowTitle("Set password");
	ui.linePsw1->setFocus();
	int r = this->exec();

	return r;
}

int PasswordDlg::DoChangePassword()
{
	ui.labelPsw1->setText("Enter current password");
	ui.labelPsw2->setText("Enter new password");
	ui.labelPsw3->setText("Re-enter new password");
	setWindowTitle("Change password");
	ui.linePsw1->setFocus();
	int r = this->exec();
	
	return r;
}

void PasswordDlg::onOk()
{
	m_psw1 = ui.linePsw1->text();
	m_psw2 = ui.linePsw2->text();
	m_psw3 = ui.linePsw3->text();

	QMessageBox msgBox(QMessageBox::Critical, "Error", "");
	msgBox.setPalette(pal);

	// check psw1==psw2 , or psw2==psw3
	if (ui.linePsw3->isVisible()) {
		if (m_psw0 != m_psw1)
			msgBox.setText("Old password does not match"), msgBox.exec();
		else  if (m_psw2 != m_psw3)
			msgBox.setText("New password does not match"), msgBox.exec();
		else
			accept();
	}
	else if (ui.linePsw2->isVisible()) {
		if (m_psw1 != m_psw2)
			msgBox.setText("New password does not match"), msgBox.exec();
		else
			accept();
	}
	else {
		accept();
	}
}
