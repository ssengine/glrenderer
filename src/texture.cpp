#include "device.h"
#include "format.h"

#include <ssengine/log.h>

ss_gl_texture2d::ss_gl_texture2d(){
	glGenTextures(1, &name);
}

ss_gl_texture2d::~ss_gl_texture2d(){
	glDeleteTextures(1, &name);
}

ss_texture2d* ss_gl_render_device::create_texture2d(
	size_t width, size_t height,
	ss_render_format format,
	const void* data)
{
	ss_gl_texture2d* ret = new ss_gl_texture2d();

	//TODO: use cached state to avoid glGet:
	int old;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);

	glBindTexture(GL_TEXTURE_2D, ret->name);

	glTexImage2D(GL_TEXTURE_2D, 0,
        ss_render_format_internal_format(format),
		width, height,
		0,
		ss_render_format_format(format),
		ss_render_format_ele_type(format),
		data
		);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, old);

	return ret;
}


void ss_gl_render_device::set_ps_texture2d_resource(
	size_t start,
	size_t num,
	ss_texture2d* const * textures
	){
	if (start + num > ps_shader_resources.size()){
		SS_LOGW("PS Resource index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		ss_gl_sampler_info temp = {
			textures[i]
		};
		ss_gl_sampler_info& old = ps_shader_resources[i];

		ss_gl_texture2d* tex = static_cast<ss_gl_texture2d*>(textures[i]);

		if (old != temp){
			if (old.texture != temp.texture){
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, tex->name);
			}
			ps_shader_resources[i] = temp;
		}
	}
}

void ss_gl_render_device::unset_ps_texture2d_resource(
	size_t start,
	size_t num
	){
	if (start + num > ps_shader_resources.size()){
		SS_LOGW("PS Resource index exceed.");
		return;
	}
	for (size_t i = 0; i < num; i++){
		size_t pos = start + i;
		ss_gl_sampler_info temp = {
			nullptr
		};
		ss_gl_sampler_info& old = ps_shader_resources[i];

		if (old != temp){
			if (old.texture != temp.texture){
				//No need to unbind texture for OpenGL;
				//glActiveTexture(GL_TEXTURE0 + i);
			}
			ps_shader_resources[i] = temp;
		}
	}
}
