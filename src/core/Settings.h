#pragma once
#include <QString>
#include <QByteArray>

#define SETTINGS_LIST	\
	X(int,		OutlinerMode,	0)\
	X(int,		DefItemStatus,  0)\
	X(int,		OpenNewDoc,     0)\
	X(QString,	CurrProjectPath,"")\
	X(int,		QSModeNew,		0)\
	X(int,		AutoSavePages,	1)\
	X(int,		LastImageAction,0)\
	X(int,		IconSize,		16)\
	X(int,		BackColor1,		0xFFC782)\
	X(int,		BackColor2,		0xF0CF72)\
	X(QString,	HtmEditPath,	"notepad.exe")\
	X(QString,	BrowserPath,	"")\
	X(QString,	VisEditPath,	"")\
	X(QString,	ImgEditPath,	"")\
	X(QString,	ExplorePath,	"explorer")\
	X(QString,	CommitPath,		"")\
	X(QString,	SyncPath,		"")\
	X(QString,	PdfgenPath,		"")\
	X(QString,	ScriptsDir,		"")\
	X(QString,	TitleRedef,		"")\
	X(int,		CreateNewVmb,	0)\
	X(int,		CreateNewDir,	0)\
	X(QString,	RecentProjects,	"")\
	X(QString,	LastImageDir,	"")\
	X(QString,	AppPassword,	"")\
	X(QByteArray, WinGeometry,	QVariant())\
	X(QByteArray, WinState,		QVariant())

struct Settings
{
public:
	void loadSettings();
	void saveSettings();
public:
#define X(type, var, def)	type var;
	SETTINGS_LIST
#undef X
};

