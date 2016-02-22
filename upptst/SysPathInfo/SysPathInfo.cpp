#include <Core/Core.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	DUMP(GetWindowsDirectory());
	DUMP(GetModuleFileName(LoadLibrary("kernel32")));
	DUMP(GetTempPath());
	DUMP(GetCurrentDirectory());
	
	DUMP(GetComputerName());
	DUMP(GetUserName());
	DUMP(GetDesktopManager());
	
	DUMP(GetDesktopFolder());
	DUMP(GetProgramsFolder());
#ifdef PLATFORM_WIN32
	DUMP(GetProgramsFolderX86());
#endif
	DUMP(GetAppDataFolder());
	DUMP(GetMusicFolder());
	DUMP(GetPicturesFolder());
	DUMP(GetVideoFolder());
	DUMP(GetDocumentsFolder());
	DUMP(GetTemplatesFolder());
	DUMP(GetDownloadFolder());
	
	String h = GetExeFilePath();
	
	SetCurrentDirectory(GetFileFolder(h));
	DUMP(GetFullPath(GetFileName(h)));
	
	for(FindFile ff(GetFileFolder(h) + "/*.*"); ff; ff.Next()) {
		DUMP(ff.GetPath());
		DUMP(ff.GetLength());
		DUMP(ff.IsFolder());
	}
}
