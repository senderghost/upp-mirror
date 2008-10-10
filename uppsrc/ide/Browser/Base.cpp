#include "Browser.h"

#define LTIMING(x) // RTIMING(x)
#define LLOG(x)

static const char s_dbver[] = "CPP-BASE 2.34";

void GC_Cache()
{
	String cfg = ConfigFile("cfg");
	FindFile ff(AppendFileName(cfg, "*.cache"));
	Time tm0 = ToTime(GetSysDate() - 20);
	while(ff) {
		if(ff.IsFile() && Time(ff.GetLastWriteTime()) > tm0)
			FileDelete(AppendFileName(cfg, ff.GetName()));
		ff.Next();
	}
}

String CacheFile(const String& res)
{
	return AppendFileName(ConfigFile("cfg"), MD5String(res) + ".cache");
}

CppBase& BrowserBase()
{
	static CppBase b;
	return b;
}

ArrayMap<String, BrowserFileInfo>& FileSet()
{
	static ArrayMap<String, BrowserFileInfo> x;
	return x;
}

Vector<Browser *>& RBrowser()
{
	static Vector<Browser *> bp;
	return bp;
}

void     Register(Browser *b)
{
	RBrowser().Add(b);
}

void     UnRegister(Browser *b)
{
	Vector<Browser *>& rb = RBrowser();
	int q = FindIndex(rb, b);
	if(q >= 0)
		rb.Remove(q);
}

static bool   s_console;
static String s_file;

void BrowserScanError(int line, const String& text)
{
	if(s_console)
		PutConsole(String().Cat() << s_file << " (" << line << "): " << text);
}

void SaveBrowserBase()
{
	LTIMING("SaveBrowserBase");
	RealizeDirectory(ConfigFile("cfg"));
	GC_Cache();
	CppBase& base = BrowserBase();
	if(base.GetCount() == 0)
		return;
	ArrayMap<String, BrowserFileInfo>& fileset = FileSet();
	ArrayMap<String, StringStream> out;
	String s;
	for(int j = 0; j < base.GetCount(); j++) {
		Array<CppItem>& n = base[j];
		for(int k = 0; k < n.GetCount(); k++) {
			CppItem& m = n[k];
			StringStream& o = out.GetAdd(GetCppFile(m.file));
			s = base.GetKey(j);
			o % s;
			o % m;
		}
	}

	const Workspace& wspc = GetIdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++) {
		String package = wspc[i];
		FileOut fo(CacheFile(package));
		s = s_dbver;
		fo % s;
		for(int i = 0; i < fileset.GetCount(); i++)
			if(fileset[i].package == package) {
				String fn = fileset.GetKey(i);
				String q = out.GetAdd(fn).GetResult();
				BrowserFileInfo& f = fileset[i];
				fo % fn;
				fo % f.file;
				fo % f.time;
				fo % q;
			}
	}
}

void LoadBrowserBase(Progress& pi)
{
	LTIMING("LoadBrowserBase");
	CppBase& base = BrowserBase();
	ArrayMap<String, BrowserFileInfo>& fileset = FileSet();
	String s;
	const Workspace& wspc = GetIdeWorkspace();
	pi.SetText("Assist++ loading packages");
	pi.SetTotal(wspc.GetCount());
	pi.SetPos(0);
	for(int i = 0; i < wspc.GetCount(); i++) {
		pi.Step();
		String package = wspc[i];
		FileIn in(CacheFile(package));
		in.LoadThrowing();
		if(in) {
			try {
				in % s;
				if(s != s_dbver)
					throw LoadingError();
				while(!in.IsEof()) {
					String fn;
					in % fn;
					BrowserFileInfo& f = fileset.GetAdd(fn);
					in % f.file;
					in % f.time;
					f.package = package;
					String q;
					in % q;
					StringStream ss(q);
					ss.LoadThrowing();
					while(!ss.IsEof()) {
						String s;
						ss % s;
						CppItem& m = base.GetAdd(s).Add();
						ss % m;
						m.file = GetCppFileIndex(fn);
					}
				}
			}
			catch(LoadingError) {
				BrowserBase().Clear();
				fileset.Clear();
				return;
			}
		}
	}
}

void FinishBase()
{
	TimeStop tm;
	Qualify(BrowserBase());
}

void ReQualifyBrowserBase()
{
	Qualify(BrowserBase());
}

Vector<String> SortedNests()
{
	LTIMING("SortedNests()");
	CppBase& base = BrowserBase();
	Vector<String> n;
	for(int i = 0; i < base.GetCount(); i++)
		if(!base.IsUnlinked(i))
			n.Add(base.GetKey(i));
	Sort(n);
	return n;
}

void UpdateBrowserBase(Progress& pi)
{
	Index<String> fp;
	Vector<String> scan;
	ArrayMap<String, BrowserFileInfo>& set = FileSet();
	const Workspace& wspc = GetIdeWorkspace();
	CppBase& base = BrowserBase();
	pi.SetText("Assist++ checking packages");
	pi.SetTotal(wspc.GetCount());
	pi.SetPos(0);
	for(int i = 0; i < wspc.GetCount(); i++) {
		pi.Step();
		const Package& pk = wspc.GetPackage(i);
		String n = wspc[i];
		for(int i = 0; i < pk.file.GetCount(); i++) {
			String path = SourcePath(n, pk.file[i]);
			String ext = ToUpper(GetFileExt(path));
			if(ext == ".C" || ext == ".CPP" || ext == ".CC" || ext == ".CXX" ||
			   ext == ".H" || ext == ".HPP" || ext == ".HH" || ext == ".HXX" ||
			   ext == ".LAY") {
				fp.Add(path);
				int q = set.Find(path);
				Time tm = FileGetTime(path);
				if(q < 0 || set[q].time != tm)
					scan.Add(path);
				BrowserFileInfo& bf = set.GetAdd(path);
				bf.package = n;
				bf.file = pk.file[i];
				bf.time = tm;
			}
		}
	}

	Vector<String> remove(scan, 1);
	Vector<int> rm;
	for(int i = 0; i < set.GetCount(); i++)
		if(fp.Find(set.GetKey(i)) < 0) {
			remove.Add(set.GetKey(i));
			rm.Add(i);
		}
	if(remove.GetCount() == 0)
		return;
	set.Remove(rm);
	Remove(base, remove);
	if(scan.GetCount()) {
		pi.SetTotal(scan.GetCount());
		pi.SetPos(0);
		pi.AlignText(ALIGN_LEFT);
		Vector<String> ignore = IgnoreList();
		for(int i = 0; i < scan.GetCount(); i++) {
			s_file = scan[i];
			pi.SetText(s_file);
			pi.Step();
			String ext = ToUpper(GetFileExt(s_file));
			if(ext == ".LAY")
				ScanLayFile(s_file);
			else {
				FileIn fi(s_file);
				Parse(fi, ignore, base, s_file, callback(BrowserScanError));
			}
		}
	}
}

void BrowserBaseScan(Stream& s, const String& fn)
{
	LTIMING("BrowserBaseScan");
	TimeStop tm;
	CppBase& base = BrowserBase();
	LLOG("Scan2 " << tm);
	Vector<String> remove;
	remove.Add(fn);
	Remove(base, remove);
	LLOG("Scan3 " << tm);
	Parse(s, IgnoreList(), base, fn, CNULL);
	LLOG("Scan4 " << tm);
	FinishBase();
	LLOG("Scan total " << tm);
	LLOG("---------");
}

void BrowserBaseScanLay(const String& fn)
{
	LTIMING("BrowserBaseScanLay");
	Vector<String> before = SortedNests();
	CppBase& base = BrowserBase();
	Vector<String> remove;
	remove.Add(fn);
	Remove(base, remove);
	ScanLayFile(fn);
	FinishBase();
}

void ClearBrowserBase()
{
	BrowserBase().Clear();
	FileSet().Clear();
}

void StartBrowserBase()
{
	static int start;
	if(start) return;
	start++;
	if(BrowserBase().GetCount() == 0) {
		Progress pi;
		pi.Title("Assist++");
		LoadBrowserBase(pi);
		UpdateBrowserBase(pi);
		FinishBase();
	}
	start--;
}

void SyncBrowserBase()
{
	if(BrowserBase().GetCount()) {
		Progress pi;
		pi.Title("Assist++");
		UpdateBrowserBase(pi);
		FinishBase();
	}
}

void RescanBrowserBase()
{
	ClearBrowserBase();
	s_console = true;
	Progress pi;
	pi.Title("Assist++");
	UpdateBrowserBase(pi);
	FinishBase();
	s_console = false;
}

bool ExistsBrowserItem(const String& item)
{
	return GetCodeRefItem(item);
}
