#include "../service/xini.h"

#include <string>
#include <list>

namespace INI
{

	extern int OutlinerMode;
	extern int DefItemStatus;
    extern int OpenNewDoc;
	extern std::string  CurrProjectPath;
	extern int QSModeNew;
	extern int AutoSavePages;
	extern int LastImageAction;
    extern int IconSize;
	extern int BackColor1;
	extern int BackColor2;

	extern std::string  HtmEditPath;
	extern std::string  BrowserPath;
	extern std::string  VisEditPath;
	extern std::string  ImgEditPath;
	extern std::string  ExplorePath;
	extern std::string  CommitPath;
	extern std::string  SyncPath;
	extern std::string  PdfgenPath;
    extern std::string  ScriptsDir;
    extern std::string  TitleRedef;

	extern int CreateNewVmb;
	extern int CreateNewDir;

	extern XIniData Main[];

	extern std::list<std::string> RecentProjects;
	extern std::string  WinGeometry;
	extern std::string  WinState;

	extern std::string  LastImageDir;
	extern std::string  AppPassword;
};

