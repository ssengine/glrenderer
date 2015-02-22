#include "device.h"

#include <ssengine/log.h>

#include <gl/GL.h>

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

void ss_gl_render_device::clear_color(const ss_color& color){
	glClearColor(color.r, color.g, color.b, color.a);
}

void ss_gl_render_device::clear(int flags){
	if (flags > 0 && flags <= 7){
		glClear(s_clear_flag_wrapper[flags]);
	}
}

//define DllMain for windows
#ifdef WIN32
#include <Windows.h>
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

	return true;
}

void ss_gl_render_device::destroy(){
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hrc);
	hrc = NULL;

	ReleaseDC(hwnd, hdc);
	hdc = NULL;
	hwnd = NULL;

	delete this;
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
