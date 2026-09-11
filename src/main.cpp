#include "gui/mainwindow.h"

#include <QApplication>
#include <QStringList>
#include <QDir>
#include <QPointer>
#include <QTranslator>
#include <QInputDialog>
#include <stdlib.h>
#include <stdio.h>

#include "core/Solution.h"
#include "core/Settings.h"

QTextCodec *codecUtf8 = 0;

int main( int argc, char ** argv )
{
    Q_INIT_RESOURCE(neopad); //Initialize the resources specified by the .qrc file
    
    INI.AppPassword = "1122"; // default

    QApplication a(argc, argv, true);
    a.setOrganizationName("NeoProgramming");
    a.setApplicationName("NeoPad");
    a.setWindowIcon(QIcon(":/app/images/app-about.png"));

	// Задаем стиль прямо в коде
	/*
	a.setStyleSheet(
		"QToolButton {"
		"    background-color: #d4d0c8;"
		"    border: 2px solid #d4d0c8;"
		"    border-top-color: #ffffff;"
		"    border-left-color: #ffffff;"
		"    border-bottom-color: #404040;"
		"    border-right-color: #404040;"
		"    border-radius: 0px;"
		"    color: #000000;"
		"    font-weight: bold;"
		"}"
		"QToolButton:checked {"
		"    border-top-color: #404040;"
		"    border-left-color: #404040;"
		"    border-bottom-color: #ffffff;"
		"    border-right-color: #ffffff;"
		"    background-color: #b0aca4;"
		"    transform: translate(1px, 1px);"
		"}"
		"QToolButton:hover {"
		"    border-top-color: #f0f0f0;"
		"    border-left-color: #f0f0f0;"
		"    border-bottom-color: #404040;"
		"    border-right-color: #404040;"
		"}"
		"QToolButton:hover:checked {"
		"    border-top-color: #404040;"
		"    border-left-color: #404040;"
		"    border-bottom-color: #f0f0f0;"
		"    border-right-color: #f0f0f0;"
		"    background-color: #a09c94;"
		"}"
	);
	*/

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

 	codecUtf8 = QTextCodec::codecForName("Utf-8");

    theSln.loadSettings();

    bool ok = true;
	int appExec = 0;
    QString psw, desired_psw = INI.AppPassword;
    if(desired_psw != "")
        psw = QInputDialog::getText(NULL, "Input PIN",  "PIN:", QLineEdit::Password, "", &ok);

    if(ok && desired_psw == psw)
    {
        QPointer<MainWindow> mw = new MainWindow();
		mw->show();
		a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );

		appExec = a.exec();

        theSln.saveSettings();
	}
        	
    return appExec;
}
