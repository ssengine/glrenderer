#include "device.h"

//TODO: check device release before resource release.

//TODO: cache GL state for performance.

#include <ssengine/log.h>

static int s_clear_flag_wrapper[] = {
	0,
	GL_COLOR_BUFFER_BIT,
	GL_DEPTH_BUFFER_BIT,
	GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
};

#define MAX_SUPPORED_PRIMITIVES (5)
static int s_primitive_wrapper[MAX_SUPPORED_PRIMITIVES + 1] = {
	0,
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP
};

ss_gl_render_device::ss_gl_render_device()
	: pt(SS_MT_NULL), gl_pt(0),
		input_layout(nullptr),
		program(0), pass(nullptr)
{
	for (auto itor = predefined_techiques.begin();
		itor != predefined_techiques.end();
		++itor){
		*itor = nullptr;
	}

	//GL_MAX_VERTEX_ATTRIBS
	int maxVertexAttribs = 16;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

	vertex_buffers.resize(maxVertexAttribs);

	int maxFragmentUniforms = 256;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxFragmentUniforms);

	ps_constant_buffers.resize(maxFragmentUniforms);

	int maxShaderResources = 16;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxShaderResources);
	// limit shader resource count to 32 for compatible.
	maxShaderResources = maxShaderResources > 32 ? 32 : maxShaderResources;
	ps_shader_resources.resize(maxShaderResources);
}

void ss_gl_render_device::release_predefined_techniques(){
	for (auto itor = predefined_techiques.begin();
		itor != predefined_techiques.end();
		++itor){
		if (*itor){
			delete *itor;
		}
	}
}

void ss_gl_render_device::clear_color(const ss_color& color){
	glClearColor(color.r, color.g, color.b, color.a);
}

void ss_gl_render_device::clear(int flags){
	if (flags > 0 && flags <= 7){
		glClear(s_clear_flag_wrapper[flags]);
	}
}

void ss_gl_render_device::set_viewport(int left, int top, int width, int height){
	glViewport(left, top, width, height);
}

void ss_gl_render_device::set_primitive_type(ss_primitive_type _pt)
{
	if (pt != _pt){
		pt = _pt;
		int iPt = (int)_pt;
		if (iPt > 0 && iPt <= MAX_SUPPORED_PRIMITIVES){
			gl_pt = s_primitive_wrapper[iPt];
		}
	}
}

void ss_gl_render_device::draw(int count, int from){
	if (program != 0 && pass == nullptr){
		program = 0;
		glUseProgram(0);
	}
	if (gl_pt != 0){
		glDrawArrays(gl_pt, from, count);
	}
}

void ss_gl_render_device::draw_index(int count, int from, int base){
	if (program != 0 && pass == nullptr){
		program = 0;
		glUseProgram(0);
	}
	if (gl_pt != 0){
		
	}
}

void ss_gl_render_device::set_input_layout(ss_render_input_layout* layout){
	ss_gl_render_input_layout* _layout = (ss_gl_render_input_layout*)layout;
	if (input_layout != _layout){
		ss_gl_render_input_layout* old = input_layout;
		input_layout = _layout;
		if (input_layout){
			input_layout->use(old);
		}
	}
}

//define DllMain for windows
#ifdef WIN32
BOOL WINAPI DllMain(
	_In_  HINSTANCE hinstDLL,
	_In_  DWORD fdwReason,
	_In_  LPVOID lpvReserved
	){
	return TRUE;
}
#endif

#ifdef WIN32
//TODO: use EGLEmulator for Win32

#ifdef NDEBUG
#pragma comment(lib, "glew32s.lib")
#else
#pragma comment(lib, "glew32sd.lib")
#endif
#pragma comment(lib, "Opengl32.lib")

void ss_gl_render_device::present(){
	SwapBuffers(hdc);
}

bool ss_gl_render_device::init(HWND hwnd){
	HDC hdc = ::GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int iPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, iPixelFormat, &pfd);

	HGLRC hrc = wglCreateContext(hdc);

	if (hrc == nullptr){
		SS_LOGE("wglCreateContext failed: %d", (int)GetLastError());
		ReleaseDC(hwnd, hdc);
		return false;
	}

	wglMakeCurrent(hdc, hrc);

	this->hwnd = hwnd;
	this->hdc = hdc;
	this->hrc = hrc;

	glewInit();

	return true;
}

ss_gl_render_device::~ss_gl_render_device(){
	release_predefined_techniques();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hrc);
	hrc = NULL;

	ReleaseDC(hwnd, hdc);
	hdc = NULL;
	hwnd = NULL;

}

// Create device from WGL
ss_render_device* ss_device_factory(ss_device_type dt, uintptr_t hwnd)
{
	if (dt == SS_DT_OPENGL || dt == SS_DT_ANY){
		ss_gl_render_device* ret = new ss_gl_render_device();
		if (!ret->init((HWND)hwnd)){
			delete ret;
			return NULL;
		}

		return ret;
	}
	return NULL;
}

#else
//TODO: EGL and other stuff.
#endif
