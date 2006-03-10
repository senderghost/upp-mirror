#include "LayDes.h"

struct EnumProperty : public EditorProperty<DropList> {
	virtual void     SetData(const Value& v);
	virtual String   Save(byte) const              { return ~editor; }
	virtual void     Read(CParser& p, byte)        { SetData(ReadPropertyParam(p)); }

	EnumProperty(VectorMap<String, String>& e) {
		Add(editor.HSizePos(100, 2).TopPos(2));
		for(int i = 0; i < e.GetCount(); i++)
			editor.Add(e.GetKey(i), e[i]);
		SetData(defval = e.GetKey(0));
	}
};

void  EnumProperty::SetData(const Value& v)
{
	if(!editor.HasKey(v))
		editor.Add(v, v);
	editor <<= v;
}

bool ParseTemplate(String& type, String& temp)
{
	int q = type.Find('<');
	if(q < 0)
		return false;
	temp = type.Mid(q + 1);
	type.Trim(q);
	q = temp.Find('>');
	if(q >= 0)
		temp.Trim(q);
	Swap(temp, type);
	return true;
}

void LayoutItem::Create(const String& _type)
{
	Invalidate();
	property.Clear();
	type = _type;
	pos.x = Ctrl::PosLeft(0, 10);
	pos.y = Ctrl::PosTop(0, 10);
	String tp = type;
	String tm;
	if(ParseTemplate(tp, tm))
		CreateProperties(tm, 0);
	CreateProperties(tp, 1);
}

void LayoutItem::CreateProperties(const String& classname, int level)
{
	int q = LayoutTypes().Find(classname);
	if(q < 0)
		q = LayoutTypes().Find("Unknown");
	if(q < 0)
		return;
	const LayoutType& c = LayoutTypes()[q];
	for(int i = 0; i < c.property.GetCount(); i++) {
		const TypeProperty& r = c.property[i];
		if(IsNull(r.type))
			CreateProperties(r.name, level + 1);
		else {
			ItemProperty *n = ItemProperty::Create(r.type);
			if(!n) {
				q = LayoutEnums().Find(r.type);
				if(q >= 0)
					n = new EnumProperty(LayoutEnums()[q]);
			}
			if(!n)
				n = new RawProperty;
			int q = -1;
			for(int i = 0; i < property.GetCount(); i++)
				if(r.name == property[i].name)
					q = i;
			int l = q >= 0 ? property[q].level : level + r.level;
			ItemProperty& ip = q >= 0 ? property.Set(q, n) : property.Add(n);
			ip.level = l;
			ip.name = r.name;
			if(!IsNull(r.defval))
				try {
					CParser p(r.defval);
					ip.Read(p, CHARSET_WIN1252);
					ip.defval = ~ip;
				}
				catch(CParser::Error e) {
					PutConsole(e + "\n");
				}
			else
				ip.defval = ~ip;
			ip.help = r.help;
		}
	}
}

void LayoutItem::ReadProperties(CParser& p, byte charset, bool addunknown)
{
	do {
		if(p.Id("LeftPosZ") || p.Id("LeftPos")) {
			Point pt = ReadPoint(p);
			pos.x = Ctrl::PosLeft(pt.x, pt.y);
		}
		else
		if(p.Id("RightPosZ") || p.Id("RightPos")) {
			Point pt = ReadPoint(p);
			pos.x = Ctrl::PosRight(pt.x, pt.y);
		}
		else
		if(p.Id("HSizePosZ") || p.Id("HSizePos")) {
			Point pt = ReadPoint(p);
			pos.x = Ctrl::PosSize(pt.x, pt.y);
		}
		else
		if(p.Id("HCenterPosZ") || p.Id("HCenterPos")) {
			Point pt = ReadPoint(p);
			pos.x = Ctrl::PosCenter(pt.x, pt.y);
		}
		else
		if(p.Id("TopPosZ") || p.Id("TopPos")) {
			Point pt = ReadPoint(p);
			pos.y = Ctrl::PosLeft(pt.x, pt.y);
		}
		else
		if(p.Id("BottomPosZ") || p.Id("BottomPos")) {
			Point pt = ReadPoint(p);
			pos.y = Ctrl::PosRight(pt.x, pt.y);
		}
		else
		if(p.Id("VSizePosZ") || p.Id("VSizePos")) {
			Point pt = ReadPoint(p);
			pos.y = Ctrl::PosSize(pt.x, pt.y);
		}
		else
		if(p.Id("VCenterPosZ") || p.Id("VCenterPos")) {
			Point pt = ReadPoint(p);
			pos.y = Ctrl::PosCenter(pt.x, pt.y);
		}
		else {
			String name = p.ReadId();
			int q = FindProperty(name);
			if(q < 0)
				if(addunknown) {
					q = property.GetCount();
					property.Add(new RawProperty).name = name;
				}
				else {
					p.PassChar('(');
					ReadPropertyParam(p);
					p.PassChar(')');
				}
			if(q >= 0) {
				ItemProperty& ip = property[q];
				p.PassChar('(');
				ip.Read(p, charset);
				p.PassChar(')');
			}
		}
	}
	while(p.Char('.'));
}

int  LayoutItem::FindProperty(const String& s) const
{
	for(int i = 0; i < property.GetCount(); i++)
		if(property[i].name == s)
			return i;
	return -1;
}

String LayoutItem::SaveProperties(byte charset) const
{
	String out;
	Vector<int> o = GetSortOrder(property, FieldRelation(&ItemProperty::level, StdLess<int>()));
	for(int i = 0; i < o.GetCount(); i++) {
		const ItemProperty& ip = property[o[i]];
		if(ip.GetData() != ip.defval)
			out << '.' << ip.name << '(' << ip.Save(charset) << ')';
	}
	switch(pos.x.GetAlign()) {
	case Ctrl::LEFT:   out << Format(".LeftPosZ(%d, %d)", pos.x.GetA(), pos.x.GetB()); break;
	case Ctrl::RIGHT:  out << Format(".RightPosZ(%d, %d)", pos.x.GetA(), pos.x.GetB()); break;
	case Ctrl::SIZE:   out << Format(".HSizePosZ(%d, %d)", pos.x.GetA(), pos.x.GetB()); break;
	case Ctrl::CENTER: out << Format(".HCenterPosZ(%d, %d)", pos.x.GetB(), pos.x.GetA()); break;
	}
	switch(pos.y.GetAlign()) {
	case Ctrl::TOP:    out << Format(".TopPosZ(%d, %d)", pos.y.GetA(), pos.y.GetB()); break;
	case Ctrl::BOTTOM: out << Format(".BottomPosZ(%d, %d)", pos.y.GetA(), pos.y.GetB()); break;
	case Ctrl::SIZE:   out << Format(".VSizePosZ(%d, %d)", pos.y.GetA(), pos.y.GetB()); break;
	case Ctrl::CENTER: out << Format(".VCenterPosZ(%d, %d)", pos.y.GetB(), pos.y.GetA()); break;
	}
	out.Remove(0);
	return out;
}

String LayoutItem::Save(byte charset, int i) const
{
	String out;
	if(type.IsEmpty())
		out << "\tUNTYPED(";
	else
		out << "\tITEM(" << type << ", ";
	String var = variable.IsEmpty() ? Format("dv___%d", i) : variable;
	out << var << ", " << SaveProperties(charset) << ")\r\n";
	return out;
}

void LayoutItem::UnknownPaint(Draw& w)
{
	DrawFrame(w, 0, 0, csize.cx, csize.cy, SGray);
	int q = FindProperty("SetLabel");
	if(q >= 0 && IsString(~property[q]))
		DrawSmartText(w, 0, 0, csize.cx, ToUtf8((WString)~property[q]));
	Size tsz = w.GetTextSize(type, Arial(11));
	w.DrawRect(csize.cx - tsz.cx, csize.cy - tsz.cy, tsz.cx, tsz.cy, SGray);
	w.DrawText(csize.cx - tsz.cx, csize.cy - tsz.cy, type, Arial(11), SWhite);
}

void LayoutItem::CreateMethods(EscValue& ctrl, const String& type, bool copy) const
{
	int q = LayoutTypes().Find(type);
	if(q < 0)
		q = LayoutTypes().Find("Unknown");
	if(q < 0)
		return;
	const LayoutType& m = LayoutTypes()[q];
	for(q = 0; q < m.property.GetCount(); q++)
		if(IsNull(m.property[q].type))
			CreateMethods(ctrl, m.property[q].name, copy);
	for(q = 0; q < m.methods.GetCount(); q++) {
		ctrl.MapSet(m.methods.GetKey(q), m.methods[q]);
		if(copy)
			ctrl.MapSet("Ctrl" + m.methods.GetKey(q), m.methods[q]);
	}
}

EscValue LayoutItem::CreateEsc() const
{
	EscValue ctrl;
	String tp = type;
	String tm;
	if(ParseTemplate(tp, tm)) {
		CreateMethods(ctrl, tp, true);
		if(ctrl.IsMap())
			ctrl.MapSet("CtrlPaint", ctrl.MapGet("Paint"));
		CreateMethods(ctrl, tm, false);
	}
	else
		CreateMethods(ctrl, tp, false);
	for(int q = 0; q < property.GetCount(); q++) {
		EscValue w;
		const Value& v = ~property[q];
		if(IsType<Font>(v))
			w = EscFont(v);
		if(IsString(v))
			w = (WString)v;
		if(IsNumber(v))
			w = (double)v;
		if(IsType<Color>(v))
			w = EscColor(v);
		ctrl.MapSet(property[q].name, w);
	}
	ctrl.MapSet("type", (WString)type);
	ctrl.MapSet("GetSize", ReadLambda(Format("() { return Size(%d, %d); }", csize.cx, csize.cy)));
	ctrl.MapSet("GetRect", ReadLambda(Format("() { return Rect(0, 0, %d, %d); }", csize.cx, csize.cy)));
	return ctrl;
}

EscValue LayoutItem::ExecuteMethod(const char *method, Vector<EscValue>& arg) const
{
	try {
		EscValue self = CreateEsc();
		return ::Execute(UscGlobal(), &self, method, arg, 50000);
	}
	catch(CParser::Error& e) {
		PutConsole(e + "\n");
	}
	return EscValue();
}

EscValue LayoutItem::ExecuteMethod(const char *method) const
{
	Vector<EscValue> arg;
	return ExecuteMethod(method, arg);
}

Size LayoutItem::GetMinSize()
{
	return SizeEsc(ExecuteMethod("GetMinSize"));
}

Size LayoutItem::GetStdSize()
{
	return SizeEsc(ExecuteMethod("GetStdSize"));
}

void LayoutItem::Paint(Draw& w, Size sz, bool sample)
{
	if(csize != sz) {
		csize = sz;
		DrawingDraw dw(csize);
		int q = LayoutTypes().Find(type);
		if(q < 0)
			q = LayoutTypes().Find("Unknown");
		if(q < 0)
			UnknownPaint(dw);
		else {
			try {
				const LayoutType& m = LayoutTypes()[q];
				EscValue ctrl = CreateEsc();
				if(ctrl.MapGet("Paint").IsLambda()) {
					if(sample) {
						ctrl.MapSet("SetLabel", (WString)type);
						Vector<EscValue> arg;
						::Execute(UscGlobal(), &ctrl, "Sample", arg, 50000);
					}
					Vector<EscValue> arg;
					EscValue draw;
					new EscDraw(draw, dw);
					arg.Add(draw);
					::Execute(UscGlobal(), &ctrl, "Paint", arg, 50000);
				}
				else
					UnknownPaint(dw);
				cache = dw;
			}
			catch(CParser::Error e) {
				PutConsole(e);
				DrawingDraw edw;
				edw.DrawRect(0, 0, csize.cx, csize.cy, SRed);
				edw.DrawText(2, 2, e, Arial(11).Bold(), SYellow);
				cache = edw;
			}
		}
	}
	w.DrawDrawing(sz, cache);
}

Image GetTypeIcon(const String& type, int cx, int cy, int i, Color bg)
{
	ASSERT(i >= 0 && i < 2);
	int q = LayoutTypes().Find(type);
	if(q < 0)
		return Null;
	LayoutType& p = LayoutTypes()[q];
	Image& icon = p.icon[i];
	if(p.iconsize[i] != Size(cx, cy)) {
		p.iconsize[i] = Size(cx, cy);
		LayoutItem m;
		m.Create(type);
		Size stdsize = m.GetStdSize();
		if(stdsize.cx == 0 || stdsize.cy == 0)
			return Null;
		Image ci = Image(stdsize);
		ImageDraw w(ci);
		w.DrawRect(stdsize, bg);
		m.Paint(w, stdsize, true);
		if(stdsize.cx * cy > stdsize.cy * cx)
			cy = stdsize.cy * cx / stdsize.cx;
		else
			cx = stdsize.cx * cy / stdsize.cy;
		PixelArray dest(cx, cy, -3);
		PixelCopyAntiAlias(dest, Size(cx, cy), ImageToPixelArray(ci, ScreenInfo(), -3), stdsize);
		icon = PixelArrayToImage(dest);
	}
	return icon;
}
