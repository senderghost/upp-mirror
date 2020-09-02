#ifndef _GlDraw_GlDraw_h_
#define _GlDraw_GlDraw_h_

#include <Draw/Draw.h>
#include <Painter/Painter.h>

#define GLEW_STATIC

#include <plugin/glew/glew.h>

#include <plugin/tess2/tess2.h>

#ifdef PLATFORM_WIN32
#include <plugin/glew/wglew.h>
#endif

#define GL_USE_SHADERS

#define GL_COMB_OPT

#include <GL/gl.h>

#ifdef Complex
#undef Complex
#endif

namespace Upp {

enum {
	TEXTURE_LINEAR     = 0x01,
	TEXTURE_MIPMAP     = 0x02,
	TEXTURE_COMPRESSED = 0x04,
};

GLuint CreateGLTexture(const Image& img, dword flags);

GLuint GetTextureForImage(dword flags, const Image& img, uint64 context = 0); // cached
inline GLuint GetTextureForImage(const Image& img, uint64 context = 0) { return GetTextureForImage(TEXTURE_LINEAR|TEXTURE_MIPMAP, img, context); }

#ifdef GL_USE_SHADERS

enum {
	ATTRIB_VERTEX = 1,
	ATTRIB_COLOR,
	ATTRIB_TEXPOS,
	ATTRIB_ALPHA,
};

class GLProgram {
protected:
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
	int64  serialid;

	void Compile(const char *vertex_shader_, const char *fragment_shader_);
	void Link();

public:
	void Create(const char *vertex_shader, const char *fragment_shader,
	            Tuple2<int, const char *> *bind_attr = NULL, int bind_count = 0);
	void Create(const char *vertex_shader, const char *fragment_shader,
	            int attr1, const char *attr1name);
	void Create(const char *vertex_shader, const char *fragment_shader,
	            int attr1, const char *attr1name,
	            int attr2, const char *attr2name);
	void Create(const char *vertex_shader, const char *fragment_shader,
	            int attr1, const char *attr1name,
	            int attr2, const char *attr2name,
	            int attr3, const char *attr3name);
	void Clear();
	
	int  GetAttrib(const char *name)           { return glGetAttribLocation(program, name); }
	int  GetUniform(const char *name)          { return glGetUniformLocation(program, name); }
	
	void Use();

	GLProgram();
	~GLProgram();
};

extern GLProgram gl_image, gl_image_colored, gl_rect;

#endif

class GLDraw : public SDraw {
	void SetColor(Color c);

	uint64   context;
	
#ifdef GL_COMB_OPT
	struct RectColor : Moveable<RectColor> {
		Rect  rect;
		Color color;
	};
	Vector<RectColor> put_rect;
#endif

	void FlushPutRect();

public:
	void    Flush()                   { FlushPutRect(); }

	virtual void  PutImage(Point p, const Image& img, const Rect& src);
#ifdef GL_USE_SHADERS
	virtual void  PutImage(Point p, const Image& img, const Rect& src, Color color);
#endif
	virtual void  PutRect(const Rect& r, Color color);
	
	void Init(Size sz, uint64 context = 0);
	void Finish();
	
	static void ClearCache();
	static void ResetCache();
	
	GLDraw()        { context = 0; }
	GLDraw(Size sz) { Init(sz); }

	~GLDraw();
};

void GLOrtho(float left, float right, float bottom, float top, float near_, float far_, GLuint u_projection);

};

#include "GLPainter.h"

#endif