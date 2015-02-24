#include "device.h"
#include <ssengine/log.h>


ss_vertex_buffer_memory* ss_gl_render_device::create_memory_vertex_bufer(
	size_t bytes){
	return new ss_gl_vertex_buffer_memory(bytes, 
		malloc(bytes));
}

ss_gl_vertex_buffer_memory::ss_gl_vertex_buffer_memory(
	size_t _bytes, void* _buf)
	: size(_bytes), buf(_buf)
{
}

ss_gl_vertex_buffer_memory::~ss_gl_vertex_buffer_memory()
{
	free(buf);
}

void ss_gl_render_device::set_vertex_buffer(
	size_t start,
	size_t num,
	ss_vertex_buffer* const * buffer,
	const unsigned int* strides,
	const unsigned int* offset
	){
	if (start + num > vertex_buffers.size()){
		SS_LOGW("Vertex buffer index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		ss_gl_vertex_bind_info temp = {
			buffer[i],
			strides[i],
			offset[i]
		};
		if (vertex_buffers[i] != temp){
			vertex_buffers[i] = temp;
			if (input_layout){
				input_layout->rebindVB(pos);
			}
		}
	}
}

void ss_gl_render_device::unset_vertex_buffer(
	size_t start,
	size_t num
	){
	if (start + num > vertex_buffers.size()){
		SS_LOGW("Vertex buffer index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		ss_gl_vertex_bind_info temp = {
			NULL,
			0,
			0
		};
		if (vertex_buffers[i] != temp){
			vertex_buffers[i] = temp;
			if (input_layout){
				input_layout->rebindVB(pos);
			}
		}
	}
}

ss_constant_buffer_memory* ss_gl_render_device::create_memory_constant_buffer(
	ss_render_format type,
	size_t count){
	return new ss_gl_constant_buffer_memory(type, count,
		malloc(ss_render_format_sizeof(type) * count));
}

ss_gl_constant_buffer_memory::ss_gl_constant_buffer_memory(
	ss_render_format _type,
	size_t _count, void* _buf)
	: type(_type), count(_count), buf(_buf)
{
}

ss_gl_constant_buffer_memory::~ss_gl_constant_buffer_memory()
{
	free(buf);
}

void ss_gl_render_device::set_ps_constant_buffer(
	size_t start,
	size_t num,
	ss_constant_buffer* const * buffer
	){
	if (start + num > ps_constant_buffers.size()){
		SS_LOGW("PS Constant buffer index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		if (ps_constant_buffers[i] != buffer[i]){
			ps_constant_buffers[i] = buffer[i];

			if (pass != nullptr){
				pass->rebindPSCB(i);
			}
		}
	}
}

void ss_gl_render_device::unset_ps_constant_buffer(
	size_t start,
	size_t num
	){
	if (start + num > vertex_buffers.size()){
		SS_LOGW("PS Constant buffer index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		if (ps_constant_buffers[i] != nullptr){
			ps_constant_buffers[i] = nullptr;
		}
	}
}