#include "ide.h"
#pragma hdrstop

const char tempaux[] = "<temp-aux>";
const char prjaux[] = "<prj-aux>";
const char ideaux[] = "<ide-aux>";

Font WorkspaceWork::ListFont()
{
	return StdFont();
};

String WorkspaceWork::PackagePathA(const String& pn) {
	if(pn == prjaux) {
		String nm;
		String cfg = ConfigFile("cfg");
		for(const char *s = main; *s; s++)
			nm.Cat(*s == '\\' || *s == '/' ? '$' : *s);
		RealizeDirectory(cfg);
		return AppendFileName(cfg, ForceExt(nm + '@' + GetVarsName(), ".aux"));
	}
	if(pn == ideaux)
		return ConfigFile("ide.aux");
	if(pn == tempaux)
		return ConfigFile(Sprintf("aux%x.tmp",
#ifdef PLATFORM_WIN32
		          GetCurrentProcessId()
#endif
#ifdef PLATFORM_POSIX
		          getpid()
#endif
		       ));
	return PackagePath(pn);
}

void WorkspaceWork::ScanWorkspace() {
	Workspace wspc;
	wspc.Scan(main);
	actualpackage.Clear();
	actualfileindex = -1;
	filelist.Clear();
	package.Clear();
	for(int i = 0; i < wspc.package.GetCount(); i++)
		package.Add(wspc.package.GetKey(i),
		            wspc.GetPackage(i).optimize_speed ? IdeCommonImg::FastPackage() : IdeImg::Package(),
		            i == 0 ? ListFont().Bold() : ListFont(), SColorText, false, 0, Null, SColorMark);
	if(!organizer) {
		package.Add(prjaux, IdeImg::PrjAux(), ListFont(), Magenta);
		package.Add(ideaux, IdeImg::IdeAux(), ListFont(), Magenta);
		package.Add(tempaux, IdeImg::TempAux(), ListFont(), Magenta);
	}
	package.SetCursor(0);
}

void WorkspaceWork::SavePackage()
{
	if(IsNull(actualpackage))
		return;
	String pp = PackagePathA(actualpackage);
	RealizePath(pp);
	if(organizer && backup.Find(pp) < 0) {
		Backup& b = backup.Add(pp);
		FindFile ff(pp);
		if(ff) {
			b.time = ff.GetLastWriteTime();
			b.data = LoadFile(pp);
		}
		else
			b.data = String::GetVoid();
	}
	actual.Save(pp);
	String init;
	String mnm = Filter('_' + actualpackage + "_icpp_init_stub", CharFilterMacro);
	init << "#ifndef " << mnm << "\r\n";
	init << "#define " << mnm << "\r\n";
	Index<String> once;
	for(int i = 0; i < actual.uses.GetCount(); i++) {
		String u = actual.uses[i].text;
		if(once.Find(u) < 0) {
			once.Add(u);
			init << "#include \"" << actual.uses[i].text << "/init\"\r\n";
		}
	}
	for(int i = 0; i < actual.GetCount(); i++) {
		String f = actual[i];
		if(ToLower(GetFileExt(f)) == ".icpp")
			init << "#include \"" << f << "\"\r\n";
	}
	init << "#endif\r\n";
	SaveChangedFile(SourcePath(actualpackage, "init"), init);
}

void WorkspaceWork::RestoreBackup()
{
	for(int i = 0; i < backup.GetCount(); i++) {
		Backup& b = backup[i];
		String fn = backup.GetKey(i);
		if(b.data.IsVoid())
			DeleteFile(fn);
		else {
			SaveFile(fn, b.data);
			SetFileTime(fn, b.time);
		}
	}
}

void WorkspaceWork::SaveLoadPackageNS()
{
	SavePackage();
	String p = actualpackage;
	String f;
	if(IsActiveFile())
		f = ActiveFile();
	int psc = package.GetSbPos();
	int fsc = filelist.GetSbPos();
	ScanWorkspace();
	package.SetSbPos(psc);
	package.FindSetCursor(p);
	filelist.SetSbPos(fsc);
	filelist.FindSetCursor(f);
}

void WorkspaceWork::SaveLoadPackage()
{
	SaveLoadPackageNS();
	SyncWorkspace();
}

void WorkspaceWork::LoadActualPackage()
{
	filelist.Clear();
	fileindex.Clear();
	bool open = true;
	for(int i = 0; i < actual.file.GetCount(); i++) {
		Package::File& f = actual.file[i];
		if(f.separator) {
			open = closed.Find(Sepfo(actualpackage, f)) < 0;
			filelist.Add(f, open ? IdeImg::SeparatorClose() : IdeImg::SeparatorOpen(),
			             ListFont().Bold(), open ? SColorMark : SColorText, true, 0, Null);
			fileindex.Add(i);
		}
		else
		if(open) {
			filelist.Add(f, IdeFileImage(f, f.optimize_speed), ListFont(), SColorText, false, 0,
			             Null, SColorMark);
			fileindex.Add(i);
		}
	}
}

void WorkspaceWork::PackageCursor()
{
	actualpackage = GetActivePackage();
	if(actualpackage.IsEmpty()) return;
	String pp = PackagePathA(actualpackage);
	RealizePath(pp);
	actual.Load(pp);
	LoadActualPackage();
	filelist.Enable();
}

void WorkspaceWork::FileCursor()
{
	int i = filelist.GetCursor();
	actualfileindex = -1;
	if(i >= 0 && i < fileindex.GetCount())
		actualfileindex = fileindex[i];
}

String WorkspaceWork::FileName(int i) const
{
	return i >= 0 && i < fileindex.GetCount() ? (String)actual.file[fileindex[i]] : Null;
}

bool WorkspaceWork::IsSeparator(int i) const
{
	return i >= 0 && i < fileindex.GetCount() ? actual.file[fileindex[i]].separator : true;
}

String WorkspaceWork::GetActiveFileName() const
{
	return FileName(filelist.GetCursor());
}

bool   WorkspaceWork::IsActiveFile() const
{
	int i = filelist.GetCursor();
	return i >= 0 && i < fileindex.GetCount() && fileindex[i] < actual.file.GetCount();
}

Package::File& WorkspaceWork::ActiveFile()
{
	return actual.file[fileindex[filelist.GetCursor()]];
}

void WorkspaceWork::AddFile(ADDFILE af)
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	FileSel fss;
	FileSel *fs = &OutputFs();
	RealizeDirectory(GetCommonDir());
	RealizeDirectory(GetLocalDir());
	switch(af)
	{
	case PACKAGE_FILE: fs = &BasedSourceFs(); fs->BaseDir(GetFileFolder(PackagePathA(active))); break;
	case ANY_FILE:     fs = &AnySourceFs(); break;
	case OUTPUT_FILE:  fs->ActiveDir(GetOutputDir()); break;
#ifdef PLATFORM_POSIX
	case HOME_FILE:    fs->ActiveDir(GetHomeDirectory()); break;
#endif
	case COMMON_FILE:  fs->ActiveDir(GetCommonDir()); break;
	case LOCAL_FILE:   fs->ActiveDir(GetLocalDir()); break;
	}
	if(!fs->ExecuteOpen("Add files to package..")) return;
	int fci = filelist.GetCursor();
	int cs = filelist.GetSbPos();
	int ci = fci >= 0 && fci < fileindex.GetCount() ? fileindex[fci] : -1;
	for(int i = 0; i < fs->GetCount(); i++) {
		Package::File& f = ci >= 0 ? actual.file.Insert(ci++) : actual.file.Add();
		f = (*fs)[i];
		f.readonly = fs->GetReadOnly();
	}
	SaveLoadPackage();
	filelist.SetSbPos(cs);
	filelist.SetCursor(fci >= 0 ? fci : filelist.GetCount() - 1);
	FileSelected();
}

void WorkspaceWork::AddItem(const String& name, bool separator, bool readonly)
{
	int fci = filelist.GetCursor();
	int cs = filelist.GetSbPos();
	int ci = fci >= 0 && fci < fileindex.GetCount() ? fileindex[fci] : -1;
	Package::File& f = ci >= 0 ? actual.file.Insert(ci) : actual.file.Add();
	f = name;
	f.separator = separator;
	f.readonly = readonly;
	SaveLoadPackage();
	filelist.SetSbPos(cs);
	filelist.SetCursor(fci >= 0 ? fci : filelist.GetCount() - 1);
	FileSelected();
}

void WorkspaceWork::AddSeparator()
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	String name;
	if(!EditText(name, "Insert separator", "Name"))
		return;
	AddItem(~name, true, true);
}


class Tpp : public WithTppLayout<TopWindow> {
public:
	void Sync() {
		bool en = group.IsCursor() && IsNull(group.GetKey());
		name_lbl.Enable(en);
		name.Enable(en);
		name_tpp.Enable(en);
	}

	void Load(const char *dir)
	{
		Index<String> exist;
		FindFile ff(AppendFileName(dir, "*.tpp"));
		while(ff) {
			if(ff.IsFolder()) {
				String s = GetFileTitle(ff.GetName());
				group.Add(s, AttrText(s).SetFont(StdFont().Bold()));
				exist.Add(s);
			}
			ff.Next();
		}
		static const char *h[4] = { "src.tpp", "srcdoc.tpp", "srcimp.tpp", "app.tpp" };
		for(int i = 0; i < __countof(h); i++) {
			String s = GetFileTitle(h[i]);
			if(exist.Find(s) < 0)
				group.Add(s, s + " (new)");
		}
		group.Add(Null, "<other new>");
		group.GoBegin();
	}

	String GetName()
	{
		String s;
		if(group.IsCursor()) {
			s = group.GetKey();
			if(IsNull(s))
				s << ~name;
			s << ".tpp";
		}
		return s;
	}

	typedef Tpp CLASSNAME;

	Tpp() {
		CtrlLayoutOKCancel(*this, "Insert topic group");
		group.AddKey();
		group.AddColumn("Group");
		group.WhenSel = THISBACK(Sync);
		name.SetFilter(CharFilterAlpha);
	}
};

void WorkspaceWork::AddTopicGroup()
{
	String package = GetActivePackage();
	if(IsNull(package)) return;
	Tpp dlg;
	dlg.Load(PackageDirectory(package));
	if(dlg.Run() != IDOK) return;
	String g = dlg.GetName();
	if(g.GetCount())
		AddItem(g, false, false);
}

void WorkspaceWork::RemoveFile()
{
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	if(i >= 0 && i < fileindex.GetCount()) {
		int fx = fileindex[i];
		if(actual.file[fx].separator && closed.Find(GetActiveSepfo()) >= 0) {
			int px = fx, c;
			while(--px >= 0 && !actual.file[fx].separator)
				;
			if(px >= 0 && (c = closed.Find(Sepfo(GetActivePackage(), actual.file[px]))) >= 0)
				closed.Unlink(c);
		}
		actual.file.Remove(fx);
	}
	SaveLoadPackage();
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
}

void WorkspaceWork::DelFile()
{
	if(!filelist.IsCursor() || filelist[filelist.GetCursor()].isdir) return;
	String file = SourcePath(GetActivePackage(), GetActiveFileName());
	if(IsFolder(file)) {
		if(!PromptYesNo("Remove the topic group and discard ALL topics?")) return;
		RemoveFile();
		DeleteFolderDeep(file);
	}
	else {
		if(!PromptYesNo("Remove the file from package and discard it ?")) return;
		RemoveFile();
		::DeleteFile(file);
	}
}

void WorkspaceWork::RenameFile()
{
	if(!filelist.IsCursor()) return;
	String n = GetActiveFileName();
	if(!EditText(n, "Rename file", "New name")) return;
	String spath = SourcePath(GetActivePackage(), GetActiveFileName());
	String dpath = SourcePath(GetActivePackage(), n);
	if(!filelist[filelist.GetCursor()].isdir && GetFileLength(spath) >= 0) {
		if(!::MoveFile(spath, dpath)) {
			Exclamation("Failed to rename the file.&&" + GetErrorMessage(GetLastError()));
			return;
		}
	}
	FileRename(dpath);
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	if(i >= 0 && i < fileindex.GetCount())
		(String &)actual.file[i] = n;
	SaveLoadPackage();
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
}

void WorkspaceWork::MoveFile(int d)
{
	int fi = filelist.GetCursor();
	int s = filelist.GetSbPos();
	if(fi < 0 || fi >= fileindex.GetCount())
		return;
	int a = fileindex[fi];
	int b = fileindex[fi + d];
	ShowFile(a);
	ShowFile(b);
	if(a < 0 || a >= actual.file.GetCount() || b < 0 || b >= actual.file.GetCount())
		return;
	Swap(actual.file[a], actual.file[b]);
	ShowFile(a);
	ShowFile(b);
	SavePackage();
	LoadActualPackage();
	filelist.SetSbPos(s);
	for(int i = 0; i < fileindex.GetCount(); i++)
		if(fileindex[i] == b) {
			filelist.SetCursor(i);
			break;
		}
	filelist.Sync();
}

WorkspaceWork::Sepfo WorkspaceWork::GetActiveSepfo()
{
	return Sepfo(GetActivePackage(), GetActiveFileName());
}

void WorkspaceWork::GroupOrFile(Point pos)
{
	if(pos.x < filelist.GetIconWidth())
		Group();
	if(filelist.IsCursor() && !filelist[filelist.GetCursor()].isdir)
		FileSelected();
}

void   WorkspaceWork::Group()
{
	if(!filelist.IsCursor() || !filelist[filelist.GetCursor()].isdir)
		return;
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	Sepfo as = GetActiveSepfo();
	if(closed.Find(as) >= 0)
		closed.UnlinkKey(as);
	else
		closed.Put(as);
	SaveLoadPackage();
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
}

void WorkspaceWork::OpenAllGroups()
{
	for(int i = 0; i < actual.file.GetCount(); i++)
		if(actual.file[i].separator)
			closed.UnlinkKey(Sepfo(GetActivePackage(), actual.file[i]));
	SaveLoadPackage();
}

void WorkspaceWork::CloseAllGroups()
{
	for(int i = 0; i < actual.file.GetCount(); i++)
		if(actual.file[i].separator)
			closed.FindPut(Sepfo(GetActivePackage(), actual.file[i]));
	SaveLoadPackage();
}

void  WorkspaceWork::ShowFile(int pi)
{
	bool open = true;
	Sepfo sf;
	for(int i = 0; i < actual.file.GetCount(); i++) {
		if(actual.file[i].separator) {
			sf = Sepfo(GetActivePackage(), actual.file[i]);
			open = closed.Find(sf) < 0;
		}
		else
		if(i == pi) {
			if(!open) {
				int i = filelist.GetCursor();
				int s = filelist.GetSbPos();
				closed.UnlinkKey(sf);
				SaveLoadPackage();
				filelist.SetSbPos(s);
				filelist.SetCursor(i);
			}
			return;
		}
	}
}

void WorkspaceWork::FileMenu(Bar& menu)
{
	bool sel = filelist.IsCursor() && filelist[filelist.GetCursor()].isdir;
	bool isaux = (actualpackage == tempaux || actualpackage == prjaux || actualpackage == ideaux);
	menu.Add(!isaux, "Insert package directory file(s)", THISBACK1(AddFile, PACKAGE_FILE))
		.Help("Insert file relative to current package");
	menu.Add(!isaux, "Insert topic++ group", THISBACK(AddTopicGroup));
	menu.Add("Insert separator", THISBACK(AddSeparator))
		.Help("Insert text separator line");
	menu.Separator();
	menu.Add("Insert any file(s)", THISBACK1(AddFile, ANY_FILE))
		.Key(K_SHIFT|K_CTRL_I)
		.Help("Insert files from anywhere on disk (discouraged in portable packages)");
	menu.Add(isaux, "Insert output directory file(s)", THISBACK1(AddFile, OUTPUT_FILE))
		.Help("Open file selector in output / intermediate directory for current package");
	menu.Add(isaux, "Insert Common directory file(s)", THISBACK1(AddFile, COMMON_FILE))
		.Help("Open file selector in Common directory for current package");
	menu.Add(isaux, "Insert Local directory file(s)", THISBACK1(AddFile, LOCAL_FILE))
		.Help("Open file selector in Local directory for current package");
#ifdef PLATFORM_POSIX
	menu.Add("Insert home directory file(s)", THISBACK1(AddFile, HOME_FILE))
		.Help("Open file selector in current user's HOME directory");
#endif
	menu.Separator();
	if(!organizer) {
		if(sel)
			menu.Add(closed.Find(GetActiveSepfo()) < 0 ? "Close group\t[double-click]"
			                                           : "Open group\t[double-click]", THISBACK(Group));
		menu.Add("Open all groups", THISBACK(OpenAllGroups));
		menu.Add("Close all groups", THISBACK(CloseAllGroups));
		menu.Separator();
		BuildFileMenu(menu);
		menu.Separator();
	}
	menu.Add("Rename...", THISBACK(RenameFile))
	    .Help("Rename file / separator / topic group");
	menu.Add("Remove", THISBACK(RemoveFile))
		.Key(organizer ? K_DELETE : K_ALT_DELETE)
		.Help("Remove file / separator / topic group from package");
	menu.Add(filelist.IsCursor() && !sel, "Delete", THISBACK(DelFile))
		.Help("Remove file / topic group reference from package & delete file / folder on disk");
	menu.Separator();
	menu.Add(filelist.GetCursor() > 0, "Move up", THISBACK1(MoveFile, -1))
		.Key(organizer ? K_CTRL_UP : K_SHIFT_CTRL_UP)
		.Help("Move current file one position towards package beginning");
	menu.Add(filelist.IsCursor() && filelist.GetCursor() < filelist.GetCount() - 1, "Move down",
	         THISBACK1(MoveFile, 1))
		.Key(organizer ? K_CTRL_DOWN : K_SHIFT_CTRL_DOWN)
		.Help("Move current file one position towards package end");
	menu.Separator();
	menu.Add(IsActiveFile(), "Optimize for speed", THISBACK(ToggleFileSpeed))
	    .Check(IsActiveFile() && ActiveFile().optimize_speed);
	FilePropertiesMenu(menu);
}

void WorkspaceWork::ToggleFileSpeed()
{
	if(IsActiveFile()) {
		ActiveFile().optimize_speed = !ActiveFile().optimize_speed;
		SaveLoadPackageNS();
	}
}

void WorkspaceWork::AddNormalUses()
{
	String p = SelectPackage("Select package");
	if(p.IsEmpty()) return;
	OptItem& m = actual.uses.Add();
	m.text = p;
	SaveLoadPackage();
}

void WorkspaceWork::Move(Vector<String>& v, FileList& ta, int d)
{
	int i = ta.GetCursor();
	if(i < 0 || i + d < 0 || i + d > v.GetCount() - 1) return;
	Swap(v[i], v[i + d]);
	SaveLoadPackage();
	ta.SetCursor(i + d);
}

void WorkspaceWork::RemovePackageMenu(Bar& bar)
{
	if(bar.IsScanKeys() || bar.IsScanHelp() || !bar.IsMenuBar())
		return;
	String active = GetActivePackage();
	int usecnt = 0;
	for(int i = 0; i < package.GetCount(); i++)
	{
		String pn = package[i].name;
		Package prj;
		String pp = PackagePath(pn);
		prj.Load(pp);
		for(int i = 0; i < prj.uses.GetCount(); i++)
			if(prj.uses[i].text == active) {
				usecnt++;
				bar.Add("Remove from '" + UnixPath(pn) + '\'', THISBACK1(RemovePackage, pn))
					.Help(NFormat("Remove package '%s' from uses section in '%s'", active, pp));
			}
	}
	if(usecnt > 1)
	{
		bar.MenuSeparator();
		bar.Add("Remove all uses", THISBACK1(RemovePackage, String(Null)))
			.Help(NFormat("Remove package '%s' from all uses in active project and its submodules", active));
	}
}

void WorkspaceWork::RemovePackage(String from_package)
{
	String active = GetActivePackage();
	if(IsNull(from_package) && !PromptYesNo(NFormat(
		"Remove package [* \1%s\1] from uses sections of all current packages ?", active)))
		return;
	for(int i = 0; i < package.GetCount(); i++)
		if(IsNull(from_package) || package[i].name == from_package)
		{
			String pp = PackagePath(package[i].name);
			RealizePath(pp);
			Package prj;
			prj.Load(pp);
			for(int i = prj.uses.GetCount(); --i >= 0;)
				if(prj.uses[i].text == active)
					prj.uses.Remove(i);
			prj.Save(pp);
		}
	ScanWorkspace();
	SyncWorkspace();
}

void WorkspaceWork::TogglePackageSpeed()
{
	if(!package.IsCursor())
		return;
	actual.optimize_speed = !actual.optimize_speed;
	SaveLoadPackageNS();
}

void WorkspaceWork::PackageMenu(Bar& menu)
{
	if(!menu.IsScanKeys()) {
		String act = UnixPath(GetActivePackage());
		menu.Add(~NFormat("Add package to '%s'", act), IdeImg::package_add(), THISBACK(AddNormalUses));
		RemovePackageMenu(menu);
		if(menu.IsMenuBar()) {
			menu.Separator();
			menu.Add(package.IsCursor(), "Optimize for speed", THISBACK(TogglePackageSpeed))
			    .Check(actual.optimize_speed);
			menu.Separator();
			BuildPackageMenu(menu);
		}
	}
}

WorkspaceWork::WorkspaceWork()
{
	package <<= THISBACK(PackageCursor);
	package.WhenBar = THISBACK(PackageMenu);
	package.NoRoundSize();
	package.Columns(2);
	filelist <<= THISBACK(FileCursor);
	filelist.WhenLeftClickPos = THISBACK(GroupOrFile);
	filelist.WhenLeftDouble = THISBACK(Group);
	filelist.WhenBar = THISBACK(FileMenu);
	filelist.Columns(2);
	filelist.NoRoundSize();
	actualfileindex = -1;
	organizer = false;
	package.BackPaintHint();
	filelist.BackPaintHint();
}

void WorkspaceWork::SerializeClosed(Stream& s)
{
	Workspace wspc;
	wspc.Scan(main);
	Vector<Sepfo> list;
	for(int i = 0; i < wspc.GetCount(); i++) {
		String pk = wspc[i];
		const Package& p = wspc.GetPackage(i);
		for(int i = 0; i < p.GetCount(); i++)
			if(p[i].separator) {
				Sepfo sf(pk, p[i]);
				if(closed.Find(sf) >= 0)
					list.Add(sf);
			}
	}
	s % list;
	closed = list;
}
