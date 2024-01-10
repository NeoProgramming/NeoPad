TEMPLATE = app
TARGET = NeoPad
DESTDIR	= .

CONFIG += c++11

MOC_DIR	= build/.moc
RCC_DIR	= build/.rcc
OBJECTS_DIR = build/.obj
UI_DIR	= ui

QT += core
QT += gui
QT += widgets
QT += network
QT += webkit
QT += webkitwidgets

ICON = ./logo.png

#for windows: place OpenSSL-111m-Win32 to c:/Libs
win32 {
	system(./updateBuildNumber.bat ./datetime.gen)
    INCLUDEPATH += c:/Libs/OpenSSL-111m-Win32/include
    LIBS += -Lc:/Libs/OpenSSL-111m-Win32/lib -llibssl -llibcrypto
}
#for linux: sudo apt-get install libssl-dev
linux|macx {
	system(./updateBuildNumber.sh ./datetime.gen)
	LIBS += -lcrypto
}

win32:RC_FILE = ./src/neopad.rc
#macx:RC_FILE = VolumeIcon.icns
#macx:QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
#macx:CONFIG+=x86 ppc

INCLUDEPATH += .

FORMS += \
    ./src/gui/existitemdlg.ui \
    ./src/gui/imageproperty.ui \
    ./src/gui/itemproperty.ui \
    ./src/gui/linkproperty.ui \
    ./src/gui/mainwindow.ui \
    ./src/gui/newitemdlg.ui \
    ./src/gui/newprojectdlg.ui \
    ./src/gui/quickstartdlg.ui \
    ./src/gui/passworddlg.ui \
    ./src/gui/savealldlg.ui \
    ./src/gui/selectdocdlg.ui \
    ./src/gui/slnpanel.ui \
    ./src/gui/snippetsdlg.ui \
    ./src/gui/tableproperty.ui \
    ./src/gui/topicchooser.ui \
    ./src/gui/prjpropsdlg.ui \
    ./src/gui/multieditdlg.ui

HEADERS += \
    ./src/core/ini.h \
    ./src/core/BaseItem.h \
    ./src/core/Books.h \
    ./src/core/Cryptor.h \
    ./src/core/DocItem.h \
    ./src/core/Documents.h \
    ./src/core/FavItem.h \
    ./src/core/Favorites.h \
    ./src/core/Importants.h \
    ./src/core/PrjStat.h \
    ./src/core/PrjTree.h \
    ./src/core/Snippets.h \
    ./src/core/Solution.h \
    ./src/core/vmbsrv.h \
    ./src/core/Workspace.h \
    ./src/gui/mainwindow.h \
    ./src/gui/slnpanel.h \
    ./src/gui/slntreewidget.h \
    ./src/gui/webeditview.h \
    ./src/gui/existitemdlg.h \
    ./src/gui/itemproperty.h \
    ./src/gui/newitemdlg.h \
    ./src/gui/newprojectdlg.h \
    ./src/gui/quickstartdlg.h \
    ./src/gui/savealldlg.h \
    ./src/gui/selectdocdlg.h \
    ./src/gui/snippetsdlg.h \
    ./src/gui/topicchooser.h \
    ./src/gui/imageproperty.h \
    ./src/gui/linkproperty.h \
    ./src/gui/passworddlg.h \
    ./src/gui/tableproperty.h \
    ./src/gui/tablemenu.h \
    ./src/gui/htmltable.h \
    ./src/gui/prjpropsdlg.h \
    ./src/gui/htmlimage.h \
    ./src/gui/htmllink.h \
    ./src/gui/multieditdlg.h \
    ./src/service/fail.h \
    ./src/service/search.h \
    ./src/service/numerator.h \
    ./src/service/sys.h \
    ./src/service/tools.h \
    ./src/3rdparty/pugixml/pugixml.hpp \
    ./src/3rdparty/pugixml/pugiconfig.hpp
    
SOURCES += \
    ./src/main.cpp \
    ./src/core/Books.cpp \
    ./src/core/Cryptor.cpp \
    ./src/core/DocItem.cpp \
    ./src/core/Documents.cpp \
    ./src/core/FavItem.cpp \
    ./src/core/Favorites.cpp \
    ./src/core/Importants.cpp \
    ./src/core/ini.cpp \
    ./src/core/PrjStat.cpp \
    ./src/core/Snippets.cpp \
    ./src/core/Solution.cpp \
    ./src/core/Workspace.cpp \
    ./src/gui/mainwindow.cpp \
    ./src/gui/slnpanel.cpp \
    ./src/gui/slntreewidget.cpp \
    ./src/gui/webeditview.cpp \
    ./src/gui/existitemdlg.cpp \
    ./src/gui/itemproperty.cpp \
    ./src/gui/newitemdlg.cpp \
    ./src/gui/newprojectdlg.cpp \
    ./src/gui/quickstartdlg.cpp \
    ./src/gui/savealldlg.cpp \
    ./src/gui/selectdocdlg.cpp \
    ./src/gui/snippetsdlg.cpp \
    ./src/gui/topicchooser.cpp \
    ./src/gui/imageproperty.cpp \
    ./src/gui/linkproperty.cpp \
    ./src/gui/passworddlg.cpp \
    ./src/gui/tableproperty.cpp \
    ./src/gui/tablemenu.cpp \
    ./src/gui/htmltable.cpp \
    ./src/gui/prjpropsdlg.cpp \
    ./src/gui/htmlimage.cpp \
    ./src/gui/htmllink.cpp \
    ./src/gui/multieditdlg.cpp \
    ./src/service/fail.cpp \
    ./src/service/search.cpp \
    ./src/service/numerator.cpp \
    ./src/service/sys.cpp \
    ./src/service/tools.cpp \
    ./src/service/xini.cpp \
    ./src/service/cstr.cpp \
    ./src/service/pugitools.cpp \
    ./src/3rdparty/pugixml/pugixml.cpp

RESOURCES += ./src/neopad.qrc

        
