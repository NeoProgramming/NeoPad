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
#include "core/ini.h"

QTextCodec *codecUtf8 = 0;

int main( int argc, char ** argv )
{
    Q_INIT_RESOURCE(neopad); //Initialize the resources specified by the .qrc file
    
    INI::AppPassword = "1122"; // default

    QApplication a(argc, argv, true);
    a.setOrganizationName("NeoProgramming");
    a.setApplicationName("NeoPad");
    a.setWindowIcon(QIcon(":/app/images/app-about.png"));

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

 	codecUtf8 = QTextCodec::codecForName("Utf-8");
    theSln.LoadSettings();

    bool ok = true;
	int appExec = 0;
    QString psw, desired_psw = INI::AppPassword.c_str();
    if(desired_psw != "")
        psw = QInputDialog::getText(NULL, "Input PIN",  "PIN:", QLineEdit::Password, "", &ok);

    if(ok && desired_psw == psw)
	{
		QPointer<MainWindow> mw = new MainWindow();
		mw->show();
		a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );

		appExec = a.exec();
        theSln.SaveSettings();
	}
        	
    return appExec;
}
