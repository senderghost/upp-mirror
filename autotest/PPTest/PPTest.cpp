#include <Core/Core.h>
#include <CppBase/CppBase.h>
#include <CppBase/Internal.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);

	String path = GetDataFile("test.in");
	PPSync(GetFileFolder(path));
	
	Cpp cpp;
	FileIn in(path);
	cpp.Preprocess(path, in, path);
	
	String s = cpp.output;
	String opath = GetDataFile("test.out");
	
	SaveFile("/home/cxl/outs", s);

	LOG("======================");
	LOG(s);

#ifdef flagSAVE
	SaveFile(opath, s);
#else
	ASSERT(LoadFile(opath) == s);
#endif
	LOG("===================== OK");
}
