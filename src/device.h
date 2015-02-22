#pragma once

#include <ssengine/ssengine.h>

#include <ssengine/render/device.h>

#ifdef WIN32
#include <Windows.h>
#endif

struct ss_gl_render_device
	: ss_render_device
{
	ss_gl_render_device(){}
	~ss_gl_render_device();

	virtual void clear_color(const ss_color& color);

	virtual void clear(int flags = SS_CF_COLOR);
	virtual void present();

	virtual void destroy();

	virtual void draw(int count, int from);
	virtual void draw_index(int count, int from, int base);

#ifdef WIN32
	bool init(HWND hwnd);
private:
	HWND	hwnd;
	HDC		hdc;
	HGLRC	hrc;
#else
	//TODO: EGLContext and other stuffs.
#endif
};

extern "C"{
	SS_EXPORT ss_render_device* ss_device_factory(ss_device_type dt, uintptr_t hwnd);
}