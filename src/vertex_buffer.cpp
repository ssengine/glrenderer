#include "device.h"

ss_vertex_buffer_memory* ss_gl_render_device::create_memory_vertex_bufer(
	ss_render_format type,
	size_t count){
	return new ss_gl_vertex_buffer_memory(type, count, 
		malloc(ss_render_format_sizeof(type) * count));
}

ss_gl_vertex_buffer_memory::ss_gl_vertex_buffer_memory(
	ss_render_format _type,
	size_t _count, void* _buf)
	: type(_type), count(_count), buf(_buf)
{
}

ss_gl_vertex_buffer_memory::~ss_gl_vertex_buffer_memory()
{
	free(buf);
}

