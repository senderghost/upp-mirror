#include "CtrlCore.h"

NAMESPACE_UPP

void Ctrl::DragEnter(Point p, PasteClip& d)         {}
void Ctrl::DragAndDrop(Point p, PasteClip& d)       {}
void Ctrl::DragRepeat(Point p) {}
void Ctrl::DragLeave() {}

String Ctrl::GetDropData(const String& fmt) const
{
	return GetSelection(fmt);
}

String Ctrl::GetSelection(const String& fmt) const
{
	return Null;
}

bool   Has(UDropTarget *dt, const char *fmt);
String Get(UDropTarget *dt, const char *fmt);

bool PasteClip::IsAvailable(const char *fmt) const
{
	return dt ? UPP::Has(dt, fmt) : IsClipboardAvailable(fmt);
}

String PasteClip::Get(const char *fmt) const
{
	return dt ? UPP::Get(dt, fmt) : ReadClipboard(fmt);
}

bool   PasteClip::Accept(const char *fmt)
{
	Vector<String> f = Split(fmt, ';');
	for(int i = 0; i < f.GetCount(); i++)
		if(IsAvailable(f[i])) {
			accepted = true;
			if(paste) {
				data = Get(f[i]);
				return true;
			}
			break;
		}
	return false;
}

PasteClip::PasteClip()
{
	prefer = 0;
	paste = true;
	accepted = false;
}

PasteClip& Ctrl::Clipboard()
{
	static PasteClip p;
	return p;
}

String ClipFmtsText()
{
	return "wtext;text";
}

bool AcceptText(PasteClip& clip)
{
	return clip.Accept("wtext;text");
}

int Ctrl::DoDragAndDrop(const char *fmts, const Image& sample, dword actions)
{
	VectorMap<String, String> dummy;
	return DoDragAndDrop(fmts, sample, actions, dummy);
}

int Ctrl::DoDragAndDrop(const VectorMap<String, String>& data, const Image& sample, dword actions)
{
	return DoDragAndDrop("", sample, actions, data);
}

END_UPP_NAMESPACE
