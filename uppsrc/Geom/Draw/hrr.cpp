//////////////////////////////////////////////////////////////////////
// hrr: high resolution raster.

#include "GeomDraw.h"
#pragma hdrstop

#include "hrr.h"

#include <plugin/jpg/jpg.h>
#include <plugin/gif/gif.h>
#ifndef flagHRRNOPNG
#include <plugin/png/png.h>
#endif

#define LLOG(x) // LOG(x)

inline Stream& operator % (Stream& strm, Color& color)
{
	dword dw = color.GetRaw();
	strm % dw;
	if(strm.IsLoading())
		color = Color::FromRaw(dw);
	return strm;
}

inline Stream& operator % (Stream& strm, Rectf& rc)
{
	strm % rc.left % rc.top % rc.right % rc.bottom;
	return strm;
}

static int64 Unpack64(dword i)
{
	if(!(i & 0x80000000))
		return i;
	return int64(i & 0x7fffffff) << 8;
}

static dword CeilPack64(int64 i)
{
	if(i < 0x7fffffff)
		return (dword)i;
	if(i < INT64(0x3fffffff00))
		return (dword)((i + INT64(0x80000000ff)) >> 8);
	return 0xffffffff;
}

One<ImageEncoder> HRR::StdCreateEncoder(const HRRInfo& info)
{
	switch(info.GetMethod())
	{
	case HRRInfo::METHOD_JPG: return new JpgEncoder(info.GetQuality());
	case HRRInfo::METHOD_GIF: return new GifEncoder;
	case HRRInfo::METHOD_RLE: return new RleEncoder;
//	case HRRInfo::METHOD_ZIM: return new ZImageEncoder;
#ifndef flagNOHRRPNG
	case HRRInfo::METHOD_PNG: return new PngEncoder;
#endif
	default:              return 0;
	}
}

Vector<int> HRRInfo::EnumMethods()
{
	Vector<int> out;
	out << METHOD_JPG << METHOD_GIF << METHOD_RLE << METHOD_PNG; // << METHOD_ZIM;
	return out;
}


enum { wAlphaBlend = 200 };

static void Mask1Blt(byte *dest, const byte *src, const byte *mask, int count)
{
	while(count >= 4)
	{
		if(mask[0]) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; }
		if(mask[1]) { dest[3] = src[3]; dest[4] = src[4]; dest[5] = src[5]; }
		if(mask[2]) { dest[6] = src[6]; dest[7] = src[7]; dest[8] = src[8]; }
		if(mask[3]) { dest[9] = src[9]; dest[10] = src[10]; dest[11] = src[11]; }
		dest += 12;
		src += 12;
		mask += 4;
		count -= 4;
	}
	if(count & 2)
	{
		if(mask[0]) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; }
		if(mask[1]) { dest[3] = src[3]; dest[4] = src[4]; dest[5] = src[5]; }
		dest += 6;
		src += 6;
		mask += 2;
	}
	if(count & 1)
		if(mask[0]) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; }
}

static void Mask1Copy(PixelArray& dest, const PixelArray& src, const PixelArray& mask)
{
	ASSERT(mask.bpp == 8 && src.bpp == 24 && dest.bpp == 24);
	Size size = dest.GetSize();
	ASSERT(src.GetSize() == size && mask.GetSize() == size);
	for(int i = 0; i < size.cy; i++)
		Mask1Blt(dest.GetUpScan(i), src.GetUpScan(i), mask.GetUpScan(i), size.cx);
}

static void StreamAlphaBlend(Stream& stream, Rect& dest, Rect& src, int& alpha,
							 AlphaArray& image, Color& blend_bgnd)
{
	int version = 2;
	stream / version / alpha;
	Pack16(stream, dest);
	Pack16(stream, src);
	stream % image;
	if(version >= 2)
		stream % blend_bgnd;
	else if(stream.IsLoading())
	{
		alpha = tabs(alpha);
		blend_bgnd = Null;
	}
}

static void DrawAlphaBlend(Draw& draw, Rect dest, Rect src, int alpha, AlphaArray& image, Color blend_bgnd)
{
	ASSERT(alpha >= 0);

	Rect clip = draw.GetClip(), dclip = dest & clip, dclip0 = dclip.Size();
	if(dclip.IsEmpty() || alpha == 0)
		return;

	Color c0 = (image.pixel.palette.GetCount() >= 1 ? image.pixel.palette[0] : Color(Null));
	Color c1 = (image.pixel.palette.GetCount() >= 2 ? image.pixel.palette[1] : Color(Null));
	bool mono_pixel = (image.pixel.IsMono() && (IsNull(c0) || IsNull(c1)));
	if(mono_pixel && IsNull(c0) && IsNull(c1))
		return;

	if(draw.IsDrawing())
	{
		StreamAlphaBlend(draw.DrawingOp(wAlphaBlend), dest, src, alpha, image, blend_bgnd);
		return;
	}

	PixelArray in_blend;
	if(alpha < 100 && IsNull(blend_bgnd))
	{
#ifdef PLATFORM_WIN32
		in_blend = ImageToPixelArray(DrawToImage(draw, dest), draw, -3);
#else
		in_blend = DrawableToPixelArray(draw.GetDrawable(), dest, false, -3, 4);
#endif
	}
	bool resize = (src.Size() != dest.Size() || (dest.Size() != dclip.Size() && draw.Dots()));

	if(mono_pixel)
	{
		if(resize)
		{
			PixelArray new_data = PixelArray::Mono(dclip.Size(), 8);
			PixelCopyAntiAliasMaskOnly(new_data, dest - dclip.TopLeft(), image.pixel, src, false, false, dclip0);
			new_data.palette = image.pixel.palette;
			image.pixel = new_data;
			src = dclip0;
			dest = dclip;
		}
		if(!in_blend.IsEmpty())
		{
			PixelArray copy_blend;
			copy_blend <<= in_blend;
			PixelKillMask(copy_blend, image.pixel, Nvl(c0, c1), !IsNull(c0));
			PixelAlphaBlend(copy_blend, src, in_blend, Point(0, 0), alpha);
			copy_blend.Paint(draw, src, dest);
		}
		else
			image.pixel.Paint(draw, src, dest, c0, c1);
		return;
	}

	if(resize)
	{ // scale image offhand
		if(image.pixel.GetBPP() > 8)
			PixelSetConvert(image.pixel, -3);
		if(!image.HasAlpha())
		{
			PixelArray new_data(dclip.Size(), -3);
			PixelCopyAntiAlias(new_data, dest - dclip.TopLeft(), image.pixel, src, dclip0);
			image.pixel = new_data;
		}
		else
		{
			AlphaArray new_image(dclip.Size(), -3);
			PixelCopyAntiAliasMaskOut(new_image, dest - dclip.TopLeft(), image, src, false, false, dclip0);
			image = new_image;
		}
		src = dclip0;
		dest = dclip;
	}
	if(!in_blend.IsEmpty())
	{ // blend with display contents
		if(image.HasAlpha())
		{
			PixelSetConvert(image.pixel, -3);
			Mask1Copy(image.pixel, in_blend, image.alpha);
		}
		PixelAlphaBlend(image.pixel, src, in_blend, Point(0, 0), alpha);
		image.pixel.Paint(draw, src, dest);
	}
	else {
		if(alpha < 100)
			PixelAlphaBlend(image.pixel, blend_bgnd, alpha, src);
		if(image.HasAlpha())
			image.Paint(draw, src, dest);
		else
			image.pixel.Paint(draw, src, dest);
	}
//	RTIMING("DrawAlphaBlend (raw)");
}

static void wsAlphaBlend(Draw& draw, Stream& stream, const DrawingPos& pos)
{
	Rect src, dest;
	int alpha;
	AlphaArray image;
	Color blend_bgnd;
	StreamAlphaBlend(stream, dest, src, alpha, image, blend_bgnd);
	DrawAlphaBlend(draw, pos(dest), src, alpha, image, blend_bgnd);
}

static DrawerRegistrator MK__s(wAlphaBlend, wsAlphaBlend);

static int GetMaskInfo(const PixelArray& mask)
{
	ASSERT(mask.bpp == 8);
	Size size = mask.GetSize();
	if(size.cx == 0 || size.cy == 0)
		return 0;
	if(*mask.GetUpScan(0))
	{ // check full
		for(int i = 0; i < size.cy; i++)
			for(const byte *p = mask.GetUpScan(i), *e = p + size.cx; p < e; p++)
				if(!*p)
					return 2;
		return 1;
	}
	else
	{
		for(int i = 0; i < size.cy; i++)
			for(const byte *p = mask.GetUpScan(i), *e = p + size.cx; p < e; p++)
				if(*p)
					return 2;
		return 0;
	}
}

static String EncodeMask(const PixelArray& mask, bool write_size)
{
	ASSERT(mask.bpp == 8);
	String out;
	if(write_size)
	{
		char temp[4];
		PokeIW(temp + 0, mask.GetWidth());
		PokeIW(temp + 2, mask.GetHeight());
		out.Cat(temp, 4);
	}
	int full = out.GetLength();
	Size size = mask.GetSize();
	for(int i = 0; i < size.cy; i++)
	{
		const byte *p = mask.GetUpScan(i), *e = p + size.cx;
		int start = out.GetLength();
		while(p < e)
		{
			bool init0 = false;
			if(*p)
			{ // full part
				const byte *b = p;
				while(++p < e && *p)
					;
				int n = p - b;
				while(n > 253)
				{
					out.Cat(255);
					out.Cat(2);
					n -= 253;
				}
				if(n > 0)
					out.Cat(n + 2);
			}
			else
				init0 = true;
			if(p < e)
			{
				const byte *b = p;
				while(++p < e && !*p)
					;
				if(p < e)
				{
					if(init0)
						out.Cat(2);
					int n = p - b;
					while(n > 253)
					{
						out.Cat(255);
						out.Cat(2);
						n -= 253;
					}
					if(n > 0)
					{
						out.Cat(n + 2);
					}
				}
			}
		}
		if(out.GetLength() > start)
			full = out.GetLength();
		out.Cat(1);
	}
	if(full < out.GetLength())
		out.Trim(full);
	return out;
}

static void DecodeMask(PixelArray& mask, String s, bool read_size)
{
	Size size = mask.GetSize();
	const byte *p = s;
	if(read_size)
	{
		size.cx = PeekIW(p);
		size.cy = PeekIW(p + 2);
		mask.CreateMono(size, 8);
		p += 4;
	}
	ASSERT(mask.bpp == 8);
	PixelSet(mask, mask.GetRect(), 0);
	for(int i = 0; i < size.cy && *p; i++)
	{
		byte *d = mask.GetUpScan(i), *e = d + size.cx;
		while(*p >= 2 && d < e)
		{
			int n1 = *p++ - 2;
			if(e - d <= n1)
			{
				memset(d, 1, e - d);
				break;
			}
			memset(d, 1, n1);
			d += n1;
			if(*p >= 2)
			{
				n1 = *p++ - 2;
				if(e - d <= n1)
					break;
				d += n1;
			}
		}
		while(*p >= 2)
			p++;
		if(*p)
			p++;
	}
}

/*
static String EncodeMask(const RawImage& mask)
{
	ASSERT(mask.bpp == 8);
	String out;
	int full = 0;
	Size size = mask.GetSize();
	for(int i = 0; i < size.cy; i++)
	{
		const byte *p = mask.GetUpScan(i), *e = p + size.cx;
		int start = out.GetLength();
		while(p < e)
		{
			const byte *b = p;
			while(++p < e && *p)
				;
			if(p >= e)
				break;
			int n = p - b;
			while(n > 253)
			{
				out.Cat(255);
				out.Cat(2);
				n -= 253;
			}
			out.Cat(n + 2);
			b = p;
			while(++p < e && !*p)
				;
			n = p - b;
			while(n > 253)
			{
				out.Cat(255);
				out.Cat(2);
				n -= 253;
			}
			if(n > 0 || p < e)
				out.Cat(n + 2);
		}
		if(out.GetLength() > start)
			full = out.GetLength();
		out.Cat(1);
	}
	if(full < out.GetLength())
		out.Trim(full);
	return out;
}

static void DecodeMask(RawImage& mask, const String& s)
{
	ASSERT(mask.bpp == 8);
	Size size = mask.GetSize();
	mask.Set(1);
	const byte *p = s;
	for(int i = 0; i < size.cy && *p; i++)
	{
		byte *d = mask.GetUpScan(i), *e = d + size.cx;
		while(*p >= 2 && d < e)
		{
			int n1 = *p++ - 2;
			if(e - d <= n1)
				break;
			d += n1;
			if(*p < 2)
				break;
			n1 = *p++ - 2;
			if(e - d <= n1)
			{
				memset(d, 0, e - d);
				break;
			}
			memset(d, 0, n1);
			d += n1;
		}
		while(*p >= 2)
			p++;
		if(*p)
			p++;
	}
}
*/

HRRInfo::HRRInfo()
: levels(0)
, log_rect(0, 0, 0, 0)
, map_rect(0, 0, 0, 0)
, background(White)
, method(METHOD_JPG)
, quality(50)
, mono(false)
, mono_black(Black)
, mono_white(White)
{
}

HRRInfo::HRRInfo(Rectf log_rect, int levels, Color background, int method, int quality,
	bool mono, Color mono_black, Color mono_white)
: log_rect(log_rect), levels(levels), background(background), method(method), quality(quality)
, mono(mono), mono_black(mono_black), mono_white(mono_white)
{
	double wadd = log_rect.Height() - log_rect.Width();
	map_rect = log_rect;
	if(wadd >= 0)
		map_rect.right += wadd;
	else
		map_rect.top += wadd;
}

void HRRInfo::Serialize(Stream& stream)
{
	int outver = (stream.IsStoring() && !mono && method != METHOD_ZIM && method != METHOD_BZM ? 4 : 5);
	int version = StreamHeading(stream, outver, 2, 5, "HRRInfo");
	if(version >= 2)
		stream / levels % background % log_rect % map_rect;
	if(version >= 3)
		stream / method;
	else if(stream.IsLoading())
		method = METHOD_JPG;
	if(version >= 4)
		stream / quality;
	else if(stream.IsLoading())
		quality = 0;
	if(version >= 5)
		stream % mono % mono_black % mono_white;
	else if(stream.IsLoading())
	{
		mono = false;
		mono_black = Null;
		mono_white = Null;
	}
}

Vector<int> HRRInfo::EnumQualities(int method)
{
	Vector<int> out;
	switch(method)
	{
	case METHOD_JPG:
		{
			for(int i = 10; i <= 100; i += 10)
				out << i;
		}
		break;

	case METHOD_GIF:
	case METHOD_RLE:
	case METHOD_PNG:
	case METHOD_ZIM:
	case METHOD_BZM:
		out << 0;
		break;

	default:
		NEVER();
		break;
	}
	return out;
}

VectorMap<int, String> HRRInfo::GetPackMap()
{
	VectorMap<int, String> out;
	Vector<int> methods = EnumMethods();
	for(int m = 0; m < methods.GetCount(); m++)
	{
		Vector<int> qualities = EnumQualities(methods[m]);
		if(qualities.IsEmpty())
			qualities.Add(0);
		for(int q = 0; q < qualities.GetCount(); q++)
			out.FindAdd(Pack(methods[m], qualities[q]), GetName(methods[m], qualities[q]));
	}
	return out;
}

String HRRInfo::GetName(int method, int quality)
{
	String out;
	switch(method)
	{
	case METHOD_JPG:
		out << "JPEG " << (quality ? quality : DFLT_JPG_QUALITY) << "%";
		break;

	case METHOD_GIF:
		out << "PAL";
		break;

	case METHOD_RLE:
		out << "RLE";
		break;

	case METHOD_PNG:
		out << "PNG";
		break;

	case METHOD_ZIM:
		out << "ZIM";
		break;

	case METHOD_BZM:
		out << "BZM";
		break;

	default:
		out << "?? (" << method << ")";
	}
	return out;
}

double HRRInfo::GetEstimatedFileSize(int _levels, int method, int quality)
{
	int images = 0;
	for(int i = 0; i < _levels; i++)
		images += 1 << (2 * i);
	int dir_size = images * sizeof(int) // offset table
		+ 256; // estimated heading size
	double data_size = images * double(UNIT * UNIT);
	switch(method)
	{
	case METHOD_JPG:
		data_size *= (quality ? quality : DFLT_JPG_QUALITY) / 400.0; // guessed JPEG size
		break;

	case METHOD_GIF:
		data_size /= 2;
		break;

	case METHOD_RLE:
		data_size /= 1.5;
		break;

	case METHOD_PNG:
		data_size /= 1.6;
		break;

	case METHOD_ZIM:
		data_size /= 1.6;
		break;

	case METHOD_BZM:
		data_size /= 1.8;
		break;

	default:
		NEVER();
		break;
	}
	return data_size;
}

//////////////////////////////////////////////////////////////////////
// HRR::Block::

void HRR::Block::Init(Size s, Color color, bool mono)
{
//	static TimingInspector ti("HRR::Block::Init");
//	ti.Start();
	size = s;
	if(!mono)
	{
		block.pixel.Create(size, -3);
		PixelSet(block.pixel, size, Nvl(color, Black));
	}
	if(mono || IsNull(color))
	{
		block.alpha.Create(size, 8);
		PixelSet(block.alpha, size, 1);
	}
}

//////////////////////////////////////////////////////////////////////
// HRR::

One<ImageEncoder> (*HRR::CreateEncoder)(const HRRInfo& info) = HRR::StdCreateEncoder;

static const Size SUNIT(HRRInfo::UNIT, HRRInfo::UNIT);
static const Rect RUNIT(0, 0, HRRInfo::UNIT, HRRInfo::UNIT);

HRR::HRR()
{
	cache_sizeof_limit = DEFAULT_CACHE_SIZEOF_LIMIT;
}

HRR::HRR(const char *path, bool read_only)
{
	Open(path, read_only);
}

bool HRR::Open(const char *path, bool read_only)
{
	Close();
	if(!path || !*path || !stream.Open(path, read_only ? stream.READ : stream.READWRITE))
		return false;
	stream.SetLoading();
	Serialize();
	if(stream.IsError() || info.levels <= 0
		|| info.map_rect.Width() <= 0 || info.map_rect.Height() <= 0)
	{
		Close();
		return false;
	}
	return true;
}

void HRR::Close()
{
	stream.Close();
	pixel_directory.Clear();
	pixel_cache.Clear();
	mask_directory.Clear();
	mask_cache.Clear();
	directory_sizeof = 0;
	cache_sizeof = 0;
	info = HRRInfo();
}

void HRR::FlushCache(int limit, int keep_pixel, int keep_mask)
{
	int min_pixel = (keep_pixel >= 0 ? 1 : 0);
	int min_mask = (keep_mask >= 0 ? 1 : 0);
	while(cache_sizeof > cache_sizeof_limit
		&& !(pixel_cache.GetCount() <= min_pixel && mask_cache.GetCount() <= min_mask))
	{
		if(pixel_cache.GetCount() > min_pixel)
		{
			int ind = 0;
			if(keep_pixel > 0)
				keep_pixel--;
			else if(keep_pixel == 0)
				ind++;
			cache_sizeof -= pixel_cache[ind].SizeOfInstance();
			pixel_cache.Remove(ind);
			if(cache_sizeof <= cache_sizeof_limit)
				return;
		}
		if(mask_cache.GetCount() > min_mask)
		{
			int ind = 0;
			if(keep_mask > 0)
				keep_mask--;
			else if(keep_mask == 0)
				ind++;
			cache_sizeof -= mask_cache[ind].SizeOfInstance();
			mask_cache.Remove(ind);
		}
	}
}

void HRR::ClearCache()
{
	pixel_cache.Clear();
	mask_cache.Clear();
	cache_sizeof = 0;
}

static Size GetLogBlockSize(Rectf box_rc, Rectf map_rc)
{
	Size part_size(HRRInfo::UNIT, HRRInfo::UNIT);
	if(box_rc.left >= map_rc.right)
		return Size(0, 0);
	if(box_rc.right >= map_rc.right)
		part_size.cx = fround(part_size.cx * (map_rc.right - box_rc.left) / box_rc.Width());
	if(box_rc.bottom <= map_rc.top)
		return Size(0, 0);
	else if(box_rc.top < map_rc.top)
		part_size.cy = fround(part_size.cy * (box_rc.bottom - map_rc.top) / box_rc.Height());
	return part_size;
}

static Color BlendColor(Color a, int percent, Color b)
{
	return Color(
		b.GetR() + iscale(a.GetR() - b.GetR(), percent, 100),
		b.GetG() + iscale(a.GetG() - b.GetG(), percent, 100),
		b.GetB() + iscale(a.GetB() - b.GetB(), percent, 100));
}

static int StopMsec(int start = 0)
{
	return GetTickCount() - start;
}

void HRR::Paint(Draw& draw, Rect dest, Rectf src,
	int alpha, int max_pixel, Color mono_black, Color mono_white, Color blend_bgnd)
{
	LLOG("HRR::Paint: alpha = " << alpha
		<< ", max_pixel = " << max_pixel << ", mono_black = " << mono_black
		<< ", mono_white = " << mono_white << ", blend_bgnd = " << blend_bgnd
		<< ", dest = " << dest << ", src = " << src << BeginIndent);
	Swap(dest.top, dest.bottom);
	draw.Clip(dest);
	Paint(draw, MatrixfScale(src, dest), Null, alpha, max_pixel, mono_black, mono_white, blend_bgnd);
	draw.End();
	LLOG(EndIndent << "// HRR::Paint");
}

void HRR::Paint(Draw& draw, const Matrixf& trg_pix, GisTransform transform,
	int alpha, int max_pixel, Color mono_black, Color mono_white, Color blend_bgnd)
{
	LLOG("HRR::Paint: alpha = " << alpha
		<< ", max_pixel = " << max_pixel << ", mono_black = " << mono_black
		<< ", mono_white = " << mono_white << ", blend_bgnd = " << blend_bgnd
		<< ", trg_pix = " << trg_pix << BeginIndent);

	int ticks = StopMsec();
	ASSERT(alpha >= 0);

	if(alpha == 0 || info.IsEmpty() || !IsOpen()) {
		LLOG(EndIndent << "//HRR::Paint, empty case");
		return;
	}

	bool do_transform = !transform.IsIdentity();
	bool is_straight = !do_transform && fabs(trg_pix.x.y) <= 1e-10 && fabs(trg_pix.y.x) <= 1e-10;
	bool use_pixel = (IsNull(mono_black) && IsNull(mono_white));

	if(info.mono && use_pixel) {
		mono_black = info.mono_black;
		mono_white = info.mono_white;
		use_pixel = (IsNull(mono_black) && IsNull(mono_white));
		if(use_pixel) {
			LLOG(EndIndent << "//HRR::Paint, null color, empty case");
			return;
		}
	}

	bool use_bg = (alpha < 100 && IsNull(blend_bgnd) || do_transform);
	bool use_alpha = !use_pixel || IsNull(info.background);
	bool is_bw = (!IsNull(mono_black) && !IsNull(mono_white));
	bool out_pixel = (use_pixel || is_bw);
	bool out_alpha = (use_pixel ? IsNull(info.background) : !is_bw);

	LLOG("[" << StopMsec(ticks) << "] use_bg = " << use_bg << ", use_pixel = " << use_pixel << ", use_alpha = " << use_alpha
		<< ", is_bw = " << is_bw << ", out_pixel = " << out_pixel << ", out_alpha = " << out_alpha);

	Matrixf pix_trg = MatrixfInverse(trg_pix);
	Rect clip = draw.GetClip();
	Rectf csrc = info.log_rect & transform.SourceExtent(Rectf(clip) * pix_trg);
//	Pointf scale = Sizef(1, -1) * Sizef(dest.Size()) / Sizef(src.Size());
//	Pointf delta = Pointf(dest.TopLeft()) - src.BottomLeft() * scale;
//	Rectf  csrc  = src & info.log_rect;
	Rect   cdest = RectfToRect(transform.TargetExtent(csrc) * trg_pix) & clip;
//	Rect   cdest = RectfToRect(csrc * scale + delta);
//	Swap(cdest.top, cdest.bottom);
//	DrawRectMinusRect(draw, dest, cdest, info.background);
	if(cdest.IsEmpty())
	{ // intersection is less than 1 pixel wide / high
		LLOG(EndIndent << "//HRR::Paint: empty destination, exiting");
		return;
	}
	double r = fpmax(Sizef(cdest.Size()) * Sizef(info.map_rect.Size()) / csrc.Size()) / info.UNIT;
	if(draw.Dots())
		r /= 5; // ad hoc conversion from screen space to dot resolution
	int level = 0;
	for(; level < info.levels - 1 && r > max_pixel; r /= 2, level++)
		;
//	DUMP(level);

	if(!IsNull(mono_black))
		mono_black = BlendColor(mono_black, alpha, Nvl(info.background, White));
	if(!IsNull(mono_white))
		mono_white = BlendColor(mono_white, alpha, Nvl(info.background, White));

	AlphaArray out_blend;
	if(use_bg) {
		if(out_pixel) {
			out_blend.pixel.Create(cdest.Size(), -3);
			PixelSet(out_blend.pixel, out_blend.pixel.GetRect(), Nvl(info.background, Black));
		}
		if(out_alpha) {
			out_blend.alpha.CreateMono(cdest.Size());
			out_blend.alpha.palette[0] = mono_black;
			out_blend.alpha.palette[1] = mono_white;
			PixelSet(out_blend.alpha, out_blend.alpha.GetRect(), 1);
		}
	}
	LOG("out blend: pixel = " << out_blend.pixel.GetSize() << " / " << out_blend.pixel.GetBPP()
		<< ", alpha = " << out_blend.alpha.GetSize() << " / " << out_blend.alpha.GetBPP());

	// calculate interest area in Q-tree blocks
	int total = 1 << level;
	Rectf blocks = (csrc - info.map_rect.BottomLeft()) / info.map_rect.Size() * double(total);
	Rect rc(ffloor(blocks.left), ffloor(-blocks.bottom), fceil(blocks.right), fceil(-blocks.top));
	rc &= Rect(0, 0, total, total);

	// prepare clipping & image loader
	draw.Clip(cdest);
	One<ImageEncoder> encoder;
	if(!info.mono) {
		encoder = CreateEncoder(info);
		if(encoder.IsEmpty()) {
			draw.DrawText(cdest.left, cdest.top,
				String().Cat() << "Unsupported HRR encoding: " << info.GetMethod(), StdFont());
			draw.End();
			LLOG(EndIndent << "//HRR:x: encoder not found, exiting");
			return;
		}
	}

	// adjust transform parameters to convert from Q-tree space to device coords
//	delta += info.map_rect.BottomLeft() * scale;
//	scale *= Sizef(1, -1) * info.map_rect.Size() / double(1 << level);

#ifdef _DEBUG
//	int ti = 0;
#endif

	SegmentTreeInfo seginfo;
	seginfo.src_trg = transform;
	seginfo.trg_pix = trg_pix;
	seginfo.trg_pix.a -= cdest.TopLeft();
	seginfo.antialias = true;
	seginfo.branch = 0;
	seginfo.max_depth = HRRInfo::HALF_BITS - 1;
	double trg_dv = 2 / sqrt(fabs(Determinant(trg_pix)));
	Rect rclip = clip - cdest.TopLeft();
	for(int y = rc.top; y < rc.bottom; y++)
		for(int x = rc.left; x < rc.right; x++)
		{
			LLOG("[" << StopMsec(ticks) << "] block = [" << x << ", " << y << "]");
			seginfo.img_src = GetPixMapMatrix(level, x, y);
			seginfo.img_src.x.x /= HRRInfo::UNIT;
			seginfo.img_src.y.y /= HRRInfo::UNIT;
			Matrixf src_img = MatrixfInverse(seginfo.img_src);
			Rect src = RectfToRect((RUNIT * seginfo.img_src & csrc) * src_img).Inflated(2) & RUNIT;
			Rectf map = src * seginfo.img_src;
			Rect dest = (transform.TargetExtent(map) * trg_pix).Inflated(1) & Rectf(clip);
//			Rect dest = RectfToRect(Rectf(x, y, x + 1, y + 1) * scale + delta);
//			Rect clip = dest & draw.GetClip();
			if(dest.IsEmpty())
				continue;
			Rect rdest = dest - cdest.TopLeft();
			LinearSegmentTree tleft, ttop, tright, tbottom;
			PlanarSegmentTree tplanar;
			if(!is_straight) {
				seginfo.max_deviation = trg_dv;
				tleft = CreateLinearTree(src.TopLeft(), src.BottomLeft(), seginfo);
				ttop = CreateLinearTree(src.TopLeft(), src.TopRight(), seginfo);
				tright = CreateLinearTree(src.TopRight(), src.BottomRight(), seginfo);
				tbottom = CreateLinearTree(src.BottomLeft(), src.BottomRight(), seginfo);
				tplanar = CreatePlanarTree(tleft, ttop, tright, tbottom, seginfo);
			}
//			Rect src = (clip - dest.TopLeft()) * SUNIT / dest.Size();
//			src.Inflate(2);
//			src &= RUNIT;
			int pixel_offset = pixel_directory[level][x + y * total];
			int cimg = -1;
			if(pixel_offset && use_pixel && (cimg = pixel_cache.Find(pixel_offset)) < 0) {
//				Stream64Stream pixel_stream(stream, Unpack64(pixel_offset));
				stream.Seek(Unpack64(pixel_offset));
				PixelArray ni = encoder -> LoadArray(stream).pixel;
				if(ni.IsEmpty())
				{
					String warn = NFormat("Failed to load block [%d, %d].", x, y);
					Size sz = draw.GetTextSize(warn);
					draw.DrawRect(Rect(dest.CenterPoint(), Size(1, 1)).Inflated(sz + 2), Color(255, 192, 192));
					draw.DrawText((dest.left + dest.right - sz.cx) >> 1, (dest.top + dest.bottom - sz.cy) >> 1,
						warn, StdFont(), Black);
					continue;
				}
				cimg = pixel_cache.GetCount();
				cache_sizeof += ni.SizeOfInstance();
				pixel_cache.Add(pixel_offset) = ni;
				FlushCache(cache_sizeof_limit, cimg, -1);
				cimg = pixel_cache.Find(pixel_offset);
#ifdef _DEBUG
//				static int part_id = 0;
//				JpgEncoder().SaveArrayFile(Format("h:\\temp\\part%d.jpg", ++part_id), new_image);
#endif
//				if(!out_blend.IsEmpty())
//					PixelSetConvert(new_image.pixel, -3);
			}
			int mask_offset = mask_directory[level][x + y * total];
			int cmask = -1;
			if(mask_offset && use_alpha) {
				if((cmask = mask_cache.Find(mask_offset)) < 0) {
					stream.Seek(Unpack64(mask_offset));
					cmask = mask_cache.GetCount();
					PixelArray& new_mask = mask_cache.Add(mask_offset);
					int len = stream.GetIL();
					String data;
					ASSERT(len >= 0 && len < HRRInfo::UNIT * (HRRInfo::UNIT + 1) + 1);
					stream.Get(data.GetBuffer(len), len);
					data.ReleaseBuffer(len);
					if(version < 5) {
						Size sz(0, 0);
						if(cimg >= 0)
							sz = pixel_cache[cimg].GetSize();
						else if(pixel_offset) {
							int csize = size_cache.Find(pixel_offset);
							if(csize < 0) {
								if(size_cache.GetCount() >= 10000)
									size_cache.Clear();
								int64 pixpos = Unpack64(pixel_offset);
								if(pixpos > stream.GetSize())
									stream.SetSize(pixpos);
//								stream.Seek(pixpos);
								csize = size_cache.GetCount();
//								Stream64Stream pixel_stream(stream, pixpos);
								stream.Seek(pixpos);
								size_cache.Add(pixel_offset, encoder -> InfoImage(stream).size);
							}
							sz = size_cache[csize];
						}
						if(sz.cx <= 0 || sz.cy <= 0)
							continue;
						new_mask = PixelArray::Mono(sz);
					}
					DecodeMask(new_mask, data, version >= 5);
					cache_sizeof += new_mask.SizeOfInstance();
					FlushCache(cache_sizeof_limit, cimg, cmask);
					if(cimg >= 0) cimg = pixel_cache.Find(pixel_offset);
					cmask = mask_cache.Find(mask_offset);
				}
				mask_cache[cmask].palette[0] = mono_black;
				mask_cache[cmask].palette[1] = mono_white;
			}
			if(cimg < 0 && cmask < 0) {
				LLOG("[" << StopMsec(ticks) << "] pixel off, mask off");
				if(!is_straight && !IsNull(info.background))
					AlphaTransformPaint(out_blend, AlphaArray(), tplanar, tleft, ttop, tright, tbottom, seginfo, info.background);
				else if(use_pixel)
					draw.DrawRect(dest, info.background);
			}
			else if(cimg < 0) {
				const PixelArray& mask = mask_cache[cmask];
				LLOG("[" << StopMsec(ticks) << "] pixel off, mask on");
				if(!use_bg) {
					LLOG("[" << StopMsec(ticks) << "] !use_bg -> direct mask blend");
					AlphaArray out_part;
					out_part.pixel <<= mask;
					DrawAlphaBlend(draw, dest, src, 100, out_part, blend_bgnd);
				}
				else if(!is_straight) {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> twisted mask blend");
					AlphaTransformPaint(out_blend.pixel, out_blend.alpha, PixelArray(), mask,
						tplanar, tleft, ttop, tright, tbottom, seginfo, LtRed());
				}
				else if(is_bw) {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> buffered colored mask blend");
					PixelCopyAntiAlias(out_blend.pixel, rdest, mask, src, rclip);
				}
				else {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> buffered mask only blend");
					PixelCopyAntiAliasMaskOnly(out_blend.alpha, rdest, mask, src, true, false, rclip);
				}
			}
			else if(cmask < 0) {
				LLOG("[" << StopMsec(ticks) << "] pixel on, mask off");
				if(!use_bg) {
					LLOG("[" << StopMsec(ticks) << "] !use_bg -> direct maskless data blend");
					AlphaArray out_part;
					out_part.pixel <<= pixel_cache[cimg];
					DrawAlphaBlend(draw, dest, src, alpha, out_part, Nvl(info.background, blend_bgnd));
				}
				else if(!is_straight) {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> twisted data only blend");
					AlphaTransformPaint(out_blend.pixel, out_blend.alpha, pixel_cache[cimg], PixelArray(),
						tplanar, tleft, ttop, tright, tbottom, seginfo, LtRed());
				}
				else {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> buffered data only blend");
					PixelCopyAntiAlias(out_blend.pixel, rdest, pixel_cache[cimg], src, rclip);
					if(!out_blend.alpha.IsEmpty())
						PixelSet(out_blend.alpha, rdest, 0);
				}
			}
			else {
				LLOG("[" << StopMsec(ticks) << "] pixel on, mask on");
				if(!use_bg) {
					LLOG("[" << StopMsec(ticks) << "] !use_bg -> direct masked data blend");
					AlphaArray out_part;
					out_part.pixel <<= pixel_cache[cimg];
					if(IsNull(blend_bgnd))
						out_part.alpha <<= mask_cache[cmask];
					else {
						PixelSetConvert(out_part.pixel, -3);
						PixelKillMask(out_part.pixel, mask_cache[cmask], blend_bgnd);
					}
					PixelAlphaBlend(out_part.pixel, Nvl(info.background, blend_bgnd), alpha, src);
					DrawAlphaBlend(draw, dest, src, 100, out_part, blend_bgnd);
				}
				else if(!is_straight) {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> twisted full blend");
					AlphaTransformPaint(out_blend.pixel, out_blend.alpha, pixel_cache[cimg], mask_cache[cmask],
						tplanar, tleft, ttop, tright, tbottom, seginfo, LtRed());
				}
				else {
					LLOG("[" << StopMsec(ticks) << "] use_bg -> indirect masked data blend");
					PixelCopyAntiAliasMaskOut(out_blend.pixel, out_blend.alpha, rdest,
						pixel_cache[cimg], mask_cache[cmask], src, true, false, rclip);
				}
			}
		}

	if(use_bg)
	{
		if(!out_pixel)
		{
			LLOG("[" << StopMsec(ticks) << "] use_bg, mask flush");
			out_blend.pixel = out_blend.alpha;
			out_blend.alpha.Clear();
		}
		else
		{
			LLOG("[" << StopMsec(ticks) << "] use_bg, pixel & mask flush");
		}
		DrawAlphaBlend(draw, cdest, out_blend.GetRect(), alpha, out_blend, blend_bgnd);
	}
	draw.End();
	LLOG(EndIndent << "[" << StopMsec(ticks) << "] //HRR::Paint");
}

int HRR::GetProgressCount(int levels, bool downscale)
{
	ASSERT(levels > 0);
	int images = 0;
	if(downscale)
		images = 1 << (2 * (levels - 1));
	else
		for(int i = 0; i < levels; i++)
			images += 1 << (2 * i);
	return images;
}

bool HRR::Create(const HRRInfo& _info, const char *path)
{
	ASSERT(_info.levels > 0);
	Close();
	if(!stream.Open(path, stream.CREATE))
		return false;
	info = _info;
	map_offset = 0;
	Serialize();
	return true;
}

static void StreamHRRString(Stream& stream, String& string)
{
	int version = 1, len = string.GetLength();
	stream / version / len;
	if(version > 1 || stream.IsLoading() && (unsigned)len > stream.GetLeft())
	{
		stream.SetError();
		return;
	}
	if(stream.IsStoring())
		stream.SerializeRaw((byte *)(const char *)string, len);
	else
	{
		stream.SerializeRaw((byte *)string.GetBuffer(len), len);
		string.ReleaseBuffer(len);
	}
}

void HRR::Serialize()
{
	int outver = (stream.IsStoring() && (info.mono || info.method >= info.METHOD_ZIM) ? 5 : 4);
	version = StreamHeading(stream, outver, 1, 5, "\r\n"
		"High-Resolution Raster\r\n"
		"Copyright �1999 Cybex Development, spol. s r.o.\r\n");
	if(version >= 1)
		stream % info;
	if(version >= 2)
		stream % map_offset;
	else
		map_offset = 0;
	if(version >= 1)
	{
		if(stream.IsLoading() || pixel_directory.IsEmpty() || mask_directory.IsEmpty())
		{
			directory_sizeof = 0;
			pixel_directory.Clear();
			pixel_directory.SetCount(info.levels);
			for(int i = 0; i < info.levels; i++)
			{
				int c = 1 << (2 * i);
				pixel_directory[i].SetCount(c, 0);
				directory_sizeof += c * 2 * sizeof(int);
			}
			mask_directory <<= pixel_directory;
		}
		if(version <= 3 || !info.mono)
			for(int l = 0; l < info.levels; l++)
				stream.SerializeRaw((byte *)pixel_directory[l].Begin(),
					sizeof(pixel_directory[0][0]) * pixel_directory[l].GetCount());
		if(version >= 3 && (IsNull(info.background) || info.mono))
			for(int m = 0; m < info.levels; m++)
				stream.SerializeRaw((byte *)mask_directory[m].Begin(),
					sizeof(mask_directory[0][0]) * mask_directory[m].GetCount());
	}
	if(map_offset && version > 3)
	{
		int64 mappos = Unpack64(map_offset);
		if(stream.IsStoring() && stream.GetSize() < mappos) {
			stream.Seek(stream.GetSize());
			stream.Put(0, (int)(mappos - stream.GetSize()));
		}
		if(stream.IsStoring() || mappos >= 0 && mappos < stream.GetSize()) {
			stream.Seek(mappos);
			int count = map.GetCount();
			stream / count;
			for(int i = 0; i < count; i++)
			{
				String key;
				String val;
				if(stream.IsStoring())
				{
					key = map.GetKey(i);
					val = map[i];
				}
				StreamHRRString(stream, key);
				StreamHRRString(stream, val);
				if(stream.IsLoading())
					map.Add(key, val);
			}
		}
	}
	else
		map.Clear();
}

void HRR::Write(Writeback drawback, bool downscale)
{
	ASSERT(stream.IsOpen());
	if(map_offset > 0)
	{
		int64 mu = Unpack64(map_offset);
		stream.Seek(mu);
		stream.SetSize(mu);
		map_offset = 0;
	}
	One<ImageEncoder> encoder = CreateEncoder(info);
	if(!encoder)
		throw Exc(String().Cat() << "Unsupported HRR encoding: " << info.GetMethod());

	bool abort = false;
	try
	{
		Write(drawback, downscale, 0, 0, 0, *encoder, 0);
	}
	catch(AbortExc)
	{
		abort = true;
	}
	map_offset = CeilPack64(stream.GetPos());
	stream.Seek(0);
	Serialize(); // update header
	if(abort)
		throw AbortExc();
}

Matrixf HRR::GetPixMapMatrix(int level, int x, int y) const
{
	double fac = 1 << level;
	double xx = info.map_rect.Width() / fac, yy = -info.map_rect.Height() / fac;
	return Matrixf(xx, 0, 0, yy, info.map_rect.left + xx * x, info.map_rect.bottom + yy * y);
}

int64 HRR::GetFileWriteSize() const
{
	ASSERT(stream.IsOpen());
	return stream.GetSize();
}

Rectf HRR::GetLogBlockRect(int level, const Rect& rc) const
{
	return Rectf(rc) * GetPixMapMatrix(level, 0, 0);
/*	Rectf r(rc);
	double fac = 1.0 / (1 << level);
	r = r * fac;
	double t = r.bottom; r.bottom = 1 - r.top; r.top = 1 - t;
	return r * info.map_rect.Size() + info.map_rect.TopLeft();
*/
}

bool HRR::Write(Writeback drawback, bool downscale, int level, int px, int py,
				ImageEncoder& format, Block *put)
{
	static const Size SUNIT(info.UNIT, info.UNIT);
	Block block(*this);
//	TIMING("HRR::Write");

	if(level >= info.levels - info.LCOUNT)
	{ // generate all at once
//		TIMING("HRR::Write(short step)");
//		static TimingInspector __part("HRR::Write(part)");
//		__part.Start();
		int count = info.levels - level - 1;
		// step & render individual images
		block.Init(SUNIT << count, info.background, info.mono);
//		__part.End();
		block.level = level + count;
		block.area = RectC(px << count, py << count, 1 << count, 1 << count);
		block.log_area = GetLogBlockRect(block.level, block.area);
		bool done = drawback(block);
		if(!done && downscale)
			return false;

		while(count >= 0)
		{
			int n = 1 << count;
			for(Size a(0, 0); a.cy < n; a.cy++)
				for(a.cx = 0; a.cx < n; a.cx++)
				{
					Point src = a * info.UNIT;
					Size part_size = GetLogBlockSize(GetLogBlockRect(level + count, RectC(a.cx, a.cy, 1, 1)), info.log_rect);
					if(part_size.cx <= 0 || part_size.cy <= 0)
						continue;

					AlphaArray part = AlphaCrop(block.block, Rect(src, part_size));
					int lin = (int)((px << count) + a.cx + (((py << count) + a.cy) << (count + level)));
//					TIMING("HRR::Write / save (direct)");
					if(info.mono || IsNull(info.background))
					{
						int kind = GetMaskInfo(part.alpha);
						if(kind != 1 && !info.mono)
						{
							int pixoff = CeilPack64(stream.GetPos());
							pixel_directory[level + count][lin] = pixoff;
							int64 pixpos = Unpack64(pixoff);
							if(stream.GetSize() < pixpos)
								stream.Put(0, (int)(pixpos - stream.GetSize()));
							stream.Seek(pixpos);
//							Stream64Stream pixstream(stream, pixpos);
							format.SaveArray(stream, part);
						}
						if(kind == 2 || (kind == 1 && info.mono))
						{
							String s = EncodeMask(part.alpha, version >= 5);
							ASSERT(s.GetLength() >= 4);
							int maskoff = CeilPack64(stream.GetPos());
							mask_directory[level + count][lin] = maskoff;
							int64 maskpos = Unpack64(maskoff);
							if(stream.GetSize() < maskpos)
								stream.Put(0, (int)(maskpos - stream.GetSize()));
							stream.Seek(maskpos);
							stream.PutIL(s.GetLength());
							stream.Put(s, s.GetLength());
						}
					}
					else
					{
						int pixoff = CeilPack64(stream.GetPos());
						pixel_directory[level + count][lin] = pixoff;
						int64 pixpos = Unpack64(pixoff);
						if(stream.GetSize() < pixpos)
							stream.Put(0, (int)(pixpos - stream.GetSize()));
						stream.Seek(pixpos);
//						Stream64Stream pixstream(stream, pixpos);
						format.SaveArray(stream, part);
					}
				}
			if(--count >= 0) // reduce image
				if(downscale)
				{
					AlphaArray new_data;
					Size sz = SUNIT << count;
					if(!info.mono)
						new_data.pixel.Create(sz, -3);
					if(info.mono || IsNull(info.background))
						new_data.alpha.CreateMono(sz);
					PixelCopyAntiAliasMaskOut(new_data, sz, block.block, block.size, false, false);
					block.block = new_data;
				}
				else
				{
					block.Init(SUNIT << count, info.background, info.mono);
					block.level = level + count;
					block.area = RectC(px << count, py << count, 1 << count, 1 << count);
					drawback(block);
				}
		}
	}
	else
	{ // too big - bisect to generate higher level
//		TIMING("HRR::Write (long step)");
		Block *ptr = 0;
		if(downscale)
		{
			Size part_size = GetLogBlockSize(GetLogBlockRect(level, RectC(px, py, 1, 1)), info.log_rect);
			if(part_size.cx <= 0 || part_size.cy <= 0)
				return false;
			block.Init(part_size, info.background, info.mono);
			ptr = &block;
		}
		bool done = Write(drawback, downscale, level + 1, 2 * px + 0, 2 * py + 0, format, ptr);
		done     |= Write(drawback, downscale, level + 1, 2 * px + 1, 2 * py + 0, format, ptr);
		done     |= Write(drawback, downscale, level + 1, 2 * px + 0, 2 * py + 1, format, ptr);
		done     |= Write(drawback, downscale, level + 1, 2 * px + 1, 2 * py + 1, format, ptr);
		if(!done && downscale)
			return false;
		if(!downscale)
		{
			block.Init(SUNIT, info.background, info.mono);
			block.level = level;
			block.area = RectC(px, py, 1, 1);
			block.log_area = GetLogBlockRect(block.level, block.area);
			drawback(block);
		}
		int lin = px + (py << level);
//		TIMING("HRR::Write / save (indirect)");
		if(info.mono || IsNull(info.background))
		{
			int kind = GetMaskInfo(block.block.alpha);
			if(kind != 1 && !info.mono)
			{
				int pixoff = CeilPack64(stream.GetPos());
				pixel_directory[level][lin] = pixoff;
				int64 pixpos = Unpack64(pixoff);
				if(stream.GetSize() < pixpos)
					stream.Put(0, (int)(pixpos - stream.GetSize()));
				stream.Seek(pixpos);
				//Stream64Stream pixstream(stream, pixpos);
				format.SaveArray(stream, block.block);
			}
			if(kind == 2 || (kind == 1 && info.mono))
			{
				String s = EncodeMask(block.block.alpha, version >= 5);
				ASSERT(s.GetLength() >= 4);
				int maskoff = CeilPack64(stream.GetPos());
				mask_directory[level][lin] = maskoff;
				int64 maskpos = Unpack64(maskoff);
				if(stream.GetSize() < maskpos)
					stream.Put(0, (int)(maskpos - stream.GetSize()));
				stream.Seek(maskpos);
				stream.PutIL(s.GetLength());
				stream.Put(s, s.GetLength());
			}
		}
		else
		{
			int pixoff = CeilPack64(stream.GetPos());
			pixel_directory[level][lin] = pixoff;
			int64 pixpos = Unpack64(pixoff);
			while(stream.GetSize() < pixpos)
				stream.Put(0, (int)min<int64>(pixpos - stream.GetSize(), 1 << 24));
			stream.Seek(pixpos);
			//Stream64Stream pixstream(stream, pixpos);
			format.SaveArray(stream, block.block);
		}
	}
	if(put)
	{
//		TIMING("HRR::Write / put");
		Rect org = RectC((px & 1 ) << info.HALF_BITS, (py & 1) << info.HALF_BITS,
			1 << info.HALF_BITS, 1 << info.HALF_BITS);
		PixelCopyAntiAliasMaskOut(put -> block, org, block.block, RUNIT, false, false);
	}
	return true;
}

void HRR::SetMap(String key, String value)
{
	if(IsNull(value))
	{
		int i = map.Find(key);
		if(i >= 0)
			map.Remove(i);
	}
	else
		map.GetAdd(key) = value;
}

void HRR::FlushMap()
{
	ASSERT(stream.IsOpen());
	if(map_offset == 0)
		map_offset = CeilPack64(stream.GetSize());
	stream.Seek(0);
	stream.SetStoring();
	Serialize();
}

int HRR::SizeOfInstance() const
{
	return sizeof(*this) + directory_sizeof + cache_sizeof;
}
