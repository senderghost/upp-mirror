#include "CtrlCore.h"

NAMESPACE_UPP

#ifdef PLATFORM_WIN32

#define LLOG(x) // LOG(x)

int  GetClipboardFormatCode(const char *format_id);

int ToWin32CF(const char *s)
{
	return GetClipboardFormatCode(s);
}

String FromWin32CF(int cf)
{
	if(cf == CF_TEXT)
		return "text";
	if(cf == CF_UNICODETEXT)
		return "wtext";
	if(cf == CF_DIB)
		return "dib";
	char h[256];
	GetClipboardFormatNameA(cf, h, 255);
	return h;
}

FORMATETC ToFORMATETC(const char *s)
{
	FORMATETC fmtetc;
	fmtetc.cfFormat = ToWin32CF(s);
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.ptd = NULL;
	fmtetc.tymed = TYMED_HGLOBAL;
	return fmtetc;
}

String AsString(POINTL p)
{
	return String().Cat() << "[" << p.x << ", " << p.y << "]";
}

struct UDropTarget : public IDropTarget {
	ULONG         rc;
	LPDATAOBJECT  data;
	Ptr<Ctrl>     ctrl;
	Index<String> fmt;
	Ptr<Ctrl>     dndctrl;
	Point         dndpos;

	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
	STDMETHOD_(ULONG, AddRef)(void)  { return ++rc; }
	STDMETHOD_(ULONG, Release)(void) { if(--rc == 0) { delete this; return 0; } return rc; }

	STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

	void DnD(POINTL p, bool drop, DWORD *effect, DWORD keys);
	void FreeData();
	void Repeat();
	void EndDrag();
	String Get(const char *fmt) const;

	UDropTarget() { rc = 1; data = NULL; }
	~UDropTarget();
};


bool Has(UDropTarget *dt, const char *fmt)
{
	return dt->fmt.Find(fmt) >= 0;
}

String Get(UDropTarget *dt, const char *fmt)
{
	return dt->Get(fmt);
}

STDMETHODIMP UDropTarget::QueryInterface(REFIID iid, void ** ppv)
{
	if(iid == IID_IUnknown || iid == IID_IDropTarget) {
		*ppv = this;
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

String UDropTarget::Get(const char *fmt) const
{
	FORMATETC fmtetc = ToFORMATETC(fmt);
	STGMEDIUM s;
	if(data->GetData(&fmtetc, &s) == S_OK && s.tymed == TYMED_HGLOBAL) {
		char *val = (char *)GlobalLock(s.hGlobal);
		String data(val, GlobalSize(s.hGlobal));
		GlobalUnlock(s.hGlobal);
		ReleaseStgMedium(&s);
		return data;
    }
	return Null;
}

Ctrl *FindCtrl(Ctrl *ctrl, Point& p)
{
	for(;;) {
		Ctrl *c = ctrl->ChildFromPoint(p);
		if(!c) break;
		ctrl = c;
	}
	return ctrl;
}

void UDropTarget::DnD(POINTL pl, bool drop, DWORD *effect, DWORD keys)
{
	dword e = *effect;
	*effect = DROPEFFECT_NONE;
	if(!ctrl)
		return;
	Point p(pl.x, pl.y);
	PasteClip d;
	d.dt = this;
	d.paste = drop;
	d.accepted = false;
	Point hp = p - ctrl->GetScreenRect().TopLeft();
	Ctrl *c = FindCtrl(ctrl, hp);
	Rect sw = c->GetScreenView();
	if(sw.Contains(p))
		p -= sw.TopLeft();
	else
		c = NULL;
	d.allowed = 0;
	d.action = 0;
	if(e & DROPEFFECT_COPY) {
		d.allowed = DND_COPY;
		d.action = DND_COPY;
	}
	if(e & DROPEFFECT_MOVE) {
		d.allowed |= DND_MOVE;
		if(c->IsDragAndDropSource())
			d.action = DND_MOVE;
	}
	if((keys & MK_CONTROL) && (d.allowed & DND_COPY))
		d.action = DND_COPY;
	if((keys & MK_CONTROL) && (d.allowed & DND_COPY))
		d.action = DND_COPY;
	if((keys & (MK_ALT|MK_SHIFT)) && (d.allowed & DND_MOVE))
		d.action = DND_MOVE;
	if(c != dndctrl) {
		if(dndctrl) dndctrl->DragLeave();
		dndctrl = c;
		if(dndctrl) dndctrl->DragEnter(p, d);
	}
	*effect = DROPEFFECT_NONE;
	if(c) {
		dndpos = p;
		c->DragAndDrop(p, d);
		if(d.IsAccepted()) {
			if(d.action == DND_MOVE)
				*effect = DROPEFFECT_MOVE;
			if(d.action == DND_COPY)
				*effect = DROPEFFECT_COPY;
		}
	}
}

void UDropTarget::Repeat()
{
	if(dndctrl) {
		dndctrl->DragRepeat(dndpos);
		PasteClip d;
		d.dt = this;
		d.prefer = 0;
		d.paste = false;
		d.accepted = false;
		if(dndctrl)
			dndctrl->DragAndDrop(dndpos, d);
	}
}

STDMETHODIMP UDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	LLOG("DragEnter " << pt);
	SetTimeCallback(-Ctrl::GetKbdSpeed(), callback(this, &UDropTarget::Repeat), &dndpos);
	data = pDataObj;
	data->AddRef();
	fmt.Clear();
	IEnumFORMATETC *fe;
	if(!ctrl || pDataObj->EnumFormatEtc(DATADIR_GET, &fe) != NOERROR) {
		*pdwEffect = DROPEFFECT_NONE;
		return NOERROR;
	}
	FORMATETC fmtetc;
	while(fe->Next(1, &fmtetc, 0) == S_OK) {
		fmt.FindAdd(FromWin32CF(fmtetc.cfFormat));
		if(fmtetc.ptd)
			CoTaskMemFree(fmtetc.ptd);
	}
	fe->Release();
	DnD(pt, false, pdwEffect, grfKeyState);
	return NOERROR;
}


STDMETHODIMP UDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	LLOG("DragOver " << pt << " keys: " << grfKeyState);
	DnD(pt, false, pdwEffect, grfKeyState);
	return NOERROR;
}

void UDropTarget::FreeData()
{
	if(data) {
		data->Release();
		data = NULL;
	}
}

void UDropTarget::EndDrag()
{
	if(dndctrl) {
		dndctrl->DragLeave();
		dndctrl = NULL;
	}
	KillTimeCallback(&dndpos);
}

STDMETHODIMP UDropTarget::DragLeave()
{
	LLOG("DragLeave");
	EndDrag();
	FreeData();
	return NOERROR;
}

STDMETHODIMP UDropTarget::Drop(LPDATAOBJECT, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
	LLOG("Drop");
	DnD(pt, true, pdwEffect, grfKeyState);
	EndDrag();
	FreeData();
	return NOERROR;
}

UDropTarget::~UDropTarget()
{
	if(data) data->Release();
	EndDrag();
}

// --------------------------------------------------------------------------------------------

Ptr<Ctrl> sDnDSource;

Ctrl * Ctrl::DragAndDropSource()
{
	return sDnDSource;
}

struct  UDataObject : public IDataObject {
	ULONG                     rc;
	dword                     effect;
	VectorMap<String, String> data;

	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void)  { return ++rc; }
	STDMETHOD_(ULONG, Release)(void) { if(--rc == 0) { delete this; return 0; } return rc; }

	STDMETHOD(GetData)(FORMATETC *fmtetc, STGMEDIUM *medium);
	STDMETHOD(GetDataHere)(FORMATETC *, STGMEDIUM *);
	STDMETHOD(QueryGetData)(FORMATETC *fmtetc);
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *, FORMATETC *pformatetcOut);
	STDMETHOD(SetData)(FORMATETC *fmtetc, STGMEDIUM *medium, BOOL release);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ief);
	STDMETHOD(DAdvise)(FORMATETC *, DWORD, IAdviseSink *, DWORD *);
	STDMETHOD(DUnadvise)(DWORD);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA *);

	UDataObject() { rc = 1; effect = 0; }
};

struct UEnumFORMATETC : public IEnumFORMATETC {
	ULONG        rc;
	int          ii;
	UDataObject *data;

	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(void)  { return ++rc; }
	STDMETHOD_(ULONG, Release)(void) { if(--rc == 0) { delete this; return 0; } return rc; }

	STDMETHOD(Next)(ULONG n, FORMATETC *fmtetc, ULONG *fetched);
	STDMETHOD(Skip)(ULONG n);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC **newEnum);

	UEnumFORMATETC()  { ii = 0; rc = 1; }
	~UEnumFORMATETC() { data->Release(); }
};

struct UDropSource : public IDropSource {
	ULONG rc;
	Image move, copy;

	STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
	STDMETHOD_(ULONG, AddRef)(void)  { return ++rc; }
	STDMETHOD_(ULONG, Release)(void) { if(--rc == 0) { delete this; return 0; } return rc; }

	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHOD(GiveFeedback)(DWORD dwEffect);

	UDropSource() { rc = 1; }
};

STDMETHODIMP UDataObject::QueryInterface(REFIID iid, void ** ppv)
{
	if(iid == IID_IUnknown || iid == IID_IDataObject) {
		*ppv = this;
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

void SetMedium(STGMEDIUM *medium, const String& data)
{
	int sz = data.GetCount();
	HGLOBAL hData = GlobalAlloc(0, sz + 4);
	if (hData) {
		char *ptr = (char *) GlobalLock(hData);
		memcpy(ptr, ~data, sz);
		memset(ptr + sz, 0, 4);
		GlobalUnlock(hData);
		medium->tymed = TYMED_HGLOBAL;
		medium->hGlobal = hData;
		medium->pUnkForRelease = 0;
	}
}

STDMETHODIMP UDataObject::GetData(FORMATETC *fmtetc, STGMEDIUM *medium)
{
	String fmt = FromWin32CF(fmtetc->cfFormat);
	String *s = data.FindPtr(fmt);
	if(s) {
		SetMedium(medium, s->GetCount() ? *s : sDnDSource ? sDnDSource->GetDropData(fmt) : String());
		return S_OK;
	}
	return DV_E_FORMATETC;
}

STDMETHODIMP UDataObject::GetDataHere(FORMATETC *, STGMEDIUM *)
{
    return DV_E_FORMATETC;
}

STDMETHODIMP UDataObject::QueryGetData(FORMATETC *fmtetc)
{
	return data.Find(FromWin32CF(fmtetc->cfFormat)) >= 0 ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP UDataObject::GetCanonicalFormatEtc(FORMATETC *, FORMATETC *pformatetcOut)
{
    pformatetcOut->ptd = NULL;
    return E_NOTIMPL;
}

static int CF_PERFORMEDDROPEFFECT = RegisterClipboardFormat("Performed DropEffect");

STDMETHODIMP UDataObject::SetData(FORMATETC *fmtetc, STGMEDIUM *medium, BOOL release)
{
	if(fmtetc->cfFormat == CF_PERFORMEDDROPEFFECT && medium->tymed == TYMED_HGLOBAL) {
        DWORD *val = (DWORD*)GlobalLock(medium->hGlobal);
        effect = *val;
        GlobalUnlock(medium->hGlobal);
        if(release)
            ReleaseStgMedium(medium);
        return S_OK;
    }
	return E_NOTIMPL;
}

STDMETHODIMP UDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ief)
{
	UEnumFORMATETC *ef = new UEnumFORMATETC;
	ef->data = this;
	AddRef();
	*ief = ef;
	return S_OK;
}

STDMETHODIMP UDataObject::DAdvise(FORMATETC *, DWORD, IAdviseSink *, DWORD *)
{
	return OLE_E_ADVISENOTSUPPORTED;
}


STDMETHODIMP UDataObject::DUnadvise(DWORD)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP UDataObject::EnumDAdvise(LPENUMSTATDATA FAR*)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP UEnumFORMATETC::QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC) {
		*ppvObj = this;
		AddRef();
		return NOERROR;
    }
	*ppvObj = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP UEnumFORMATETC::Next(ULONG n, FORMATETC *t, ULONG *fetched) {
	if(t == NULL)
		return E_INVALIDARG;
	if(fetched) *fetched = 0;
	while(ii < data->data.GetCount() && n > 0) {
		if(fetched) (*fetched)++;
		n--;
		*t++ = ToFORMATETC(data->data.GetKey(ii++));
	}
	return n ? S_FALSE : NOERROR;
}

STDMETHODIMP UEnumFORMATETC::Skip(ULONG n) {
	ii += n;
	if(ii >= data->data.GetCount())
		return S_FALSE;
	return NOERROR;
}

STDMETHODIMP UEnumFORMATETC::Reset()
{
    ii = 0;
    return NOERROR;
}

STDMETHODIMP UEnumFORMATETC::Clone(IEnumFORMATETC **newEnum)
{
	if(newEnum == NULL)
		return E_INVALIDARG;
	UEnumFORMATETC *ef = new UEnumFORMATETC;
	ef->data = data;
	data->AddRef();
	ef->ii = ii;
	*newEnum = ef;
	return NOERROR;
}

STDMETHODIMP UDropSource::QueryInterface(REFIID riid, void **ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IDropSource) {
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP UDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if(fEscapePressed)
		return DRAGDROP_S_CANCEL;
	else
	if(!(grfKeyState & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON)))
		return DRAGDROP_S_DROP;
	Ctrl::ProcessEvents();
	return NOERROR;
}

STDMETHODIMP UDropSource::GiveFeedback(DWORD dwEffect)
{
	DUMP(dwEffect);
	Image m = IsNull(move) ? copy : move;
	if((dwEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY) {
		if(!IsNull(copy)) m = copy;
	}
	else
	if((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE) {
		if(!IsNull(move)) m = move;
	}
	else
		m = Image::No();
	Ctrl::OverrideCursor(m);
	Ctrl::SetMouseCursor(m);
	return S_OK;
}


Image MakeDragImage(const Image& arrow, const Image& arrow98, Image sample)
{
	if(IsWin2K()) {
		ImageBuffer b;
		if(IsNull(sample)) {
			sample = CtrlCoreImg::DndData();
			b = sample;
			Over(b, Point(0, 0), arrow, arrow.GetSize());
		}
		else {
			b.Create(128, 128);
			memset(~b, 0, sizeof(RGBA) * b.GetLength());
			Size ssz = sample.GetSize();
			Over(b, Point(2, 22), sample, sample.GetSize());
			for(int y = 20; y < 96; y++) {
				RGBA *s = b[y];
				RGBA *e = s + 96;
				while(s < e)
					(s++)->a >>= 1;
				e += 32;
				int q = 128;
				while(s < e) {
					s->a = (s->a * q) >> 8;
					q -= 4;
					s++;
				}
			}
			int qq = 128;
			for(int y = 96; y < 128; y++) {
				RGBA *s = b[y];
				RGBA *e = s + 96;
				while(s < e) {
					s->a = (s->a * qq) >> 8;
					s++;
				}
				e += 32;
				int q = 255;
				while(s < e) {
					s->a = (s->a * q * qq) >> 16;
					q -= 8;
					s++;
				}
				qq -= 4;
			}
			RGBA *s = b[21] + 1;
			RGBA c1 = Blue();
			RGBA c2 = White();
			for(int a = 255; a > 0; a -= 3) {
				c1.a = c2.a = a;
				*s++ = c1;
				Swap(c1, c2);
			}
			s = b[21] + 1;
			c1 = Black();
			c2 = White();
			for(int a = 255; a > 0; a -= 8) {
				c1.a = c2.a = a;
				*s = c1;
				s += b.GetWidth();
				Swap(c1, c2);
			}
		}
		Over(b, Point(0, 0), arrow, arrow.GetSize());
		return b;
	}
	else
		return arrow98;
}

int Ctrl::DoDragAndDrop(const char *fmts, const Image& sample, dword actions,
                        const VectorMap<String, String>& data)
{
	UDataObject *obj = new UDataObject;
	obj->data <<= data;
	if(fmts) {
		Vector<String> f = Split(fmts, ';');
		for(int i = 0; i < f.GetCount(); i++)
			obj->data.GetAdd(f[i]);
	}
	UDropSource *dsrc = new UDropSource;
	DWORD result = 0;
	Image m = Ctrl::OverrideCursor(CtrlCoreImg::DndMove());
	if(actions & DND_COPY) dsrc->copy = MakeDragImage(CtrlCoreImg::DndCopy(), CtrlCoreImg::DndCopy98(), sample);
	if(actions & DND_MOVE) dsrc->move = MakeDragImage(CtrlCoreImg::DndMove(), CtrlCoreImg::DndMove98(), sample);
	sDnDSource = this;
	HRESULT r = DoDragDrop(obj, dsrc,
	                       (actions & DND_COPY ? DROPEFFECT_COPY : 0) |
	                       (actions & DND_MOVE ? DROPEFFECT_MOVE : 0), &result);
	DWORD re = obj->effect;
	obj->Release();
	dsrc->Release();
	Ctrl::OverrideCursor(m);
	Ctrl::SyncCaret();
	Ctrl::CheckMouseCtrl();
	if(r == DRAGDROP_S_DROP) {
		if(((result | re) & DROPEFFECT_MOVE) == DROPEFFECT_MOVE && (actions & DND_MOVE))
			return DND_MOVE;
		if(((result | re) & DROPEFFECT_COPY) == DROPEFFECT_COPY && (actions & DND_COPY))
			return DND_COPY;
    }
	return DND_NONE;
}

void ReleaseUDropTarget(UDropTarget *dt)
{
	dt->Release();
}

UDropTarget *NewUDropTarget(Ctrl *ctrl)
{
	UDropTarget *dt = new UDropTarget;
	dt->ctrl = ctrl;
	return dt;
}

#endif

END_UPP_NAMESPACE
