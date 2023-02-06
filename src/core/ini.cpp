#include "ini.h"
#include <string>
#include <list>

#include "../service/xini.h"

namespace INI
{
	std::list<std::string> RecentProjects;
	std::string  CurrProjectPath;	// Last default path
	int QSModeNew;					// the last selected mode in the quick start window

	std::string  WinGeometry;
	std::string  WinState;
	std::string  LastImageDir;
	std::string  AppPassword;
	int AutoSavePages = 1;		// autosave pages and tree
	int LastImageAction = 0;	// Last action with a picture

	int CreateNewVmb;	// Create new vmb file
	int CreateNewDir;	// Create in a separate directory

    int IconSize = 16;	// Size of pictograms
	int BackColor1 = 0xFFC782;
	int BackColor2 = 0xF0CF72;

	int OutlinerMode = 0;		// open with a single click
	int DefItemStatus = 0;
    int OpenNewDoc = 0;

	std::string  HtmEditPath;	// Text editor
	std::string  VisEditPath;	// Visual html editor
	std::string  ImgEditPath;	// Image editor
	std::string  BrowserPath;	// Browser
	std::string  ExplorePath;	// File manager
	std::string  CommitPath;	// 
	std::string  SyncPath;		// 
	std::string  PdfgenPath;	// Pdf generation program
	std::string  ScriptsDir;	// Path to neopad.js
	std::string  TitleRedef;	// Window title

//////////////////////////////////////////////////////////////////////////

XIniData SecSettings[] = {
	XINI_INT(AutoSavePages),
	XINI_STR(CurrProjectPath),

	XINI_INT(QSModeNew),

	XINI_STR(HtmEditPath),
	XINI_STR(VisEditPath),
	XINI_STR(ImgEditPath),
	XINI_STR(BrowserPath),
	XINI_STR(ExplorePath),
	XINI_STR(CommitPath),
	XINI_STR(SyncPath),
	XINI_STR(PdfgenPath),
	XINI_INT(OutlinerMode),
	XINI_INT(DefItemStatus),
    XINI_INT(OpenNewDoc),
	XINI_INT(CreateNewVmb),
	XINI_INT(CreateNewDir),
	XINI_INT(LastImageAction),
    XINI_STR(ScriptsDir),
    XINI_INT(IconSize),
	XINI_HEX(BackColor1),
	XINI_HEX(BackColor2),
    XINI_STR(TitleRedef),
	0
};


XIniData SecRecent[] = {
	XINI_SLIST	(RecentProjects),
	0
};

XIniData SecWindows[] = {
	XINI_STR	(WinGeometry),
	XINI_STR	(WinState),
	XINI_STR	(LastImageDir),
	0
};

// main menu
XIniData Main[] = {
	XINI_GROUP(SecSettings),
	XINI_GROUP(SecRecent),
	XINI_GROUP(SecWindows),
	XINI_STR  (AppPassword),
	0
};

};// end namespace INI


