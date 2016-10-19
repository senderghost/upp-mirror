#include <Core/Core.h>

using namespace Upp;

#define TEST(x, ref) DDUMP(x), LOG("reference: " << ref), ASSERT(x == ref), LOG("----");

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	TEST(MD5String(""), "d41d8cd98f00b204e9800998ecf8427e");
	TEST(MD5StringS(""), "d41d8cd9 8f00b204 e9800998 ecf8427e");
	TEST(MD5String("Hello world"), "3e25960a79dbc69b674cd4ec67a72c62");

	TEST(SHA1String(""), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	TEST(SHA1StringS(""), "da39a3ee 5e6b4b0d 3255bfef 95601890 afd80709");
	TEST(SHA1String("Hello world"), "7b502c3a1f48c8609ae212cdfb639dee39673f5e");

	TEST(SHA256String(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	TEST(SHA256StringS(""), "e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855");
	TEST(SHA256String("Hello world"), "64ec88ca00b268e5ba1a35678a1b5316d212f4f366b2477232534a8aeca37f3c");
	
	TEST(xxHash(""), 46947589);
	TEST(xxHash("Hello world"), -1761225673);
	TEST(xxHash64(""), -1205034819632174695);
	TEST(xxHash64("Hello world"), -4251203670589081896);

	LOG("===== Everything OK");
}
