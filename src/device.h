#pragma once

#include <ssengine/ssengine.h>

#include <ssengine/render/device.h>

#include <array>


#ifdef WIN32
#define GLEW_STATIC
#include <glew.h>
#include <wglew.h>
#endif

#include <vector>
#include <string>

//TODO: per-index data

struct ss_gl_render_input_element
	: ss_render_input_element
{
	ss_gl_render_input_element() : disabled(false){}

	unsigned int			location;		//slot for attribute location
	bool					disabled;
};

struct ss_gl_render_input_layout
	: ss_render_input_layout
{
	void rebind(size_t id);
	void rebindVB(size_t slot);
	void use(ss_gl_render_input_layout* old);

	std::vector<ss_gl_render_input_element> elements;
	struct ss_gl_render_device*				device;
};

struct uniform_declaration{
	int					location;
	ss_render_format	format;
	int					offset;
	int					reserved;

	uniform_declaration(int l, ss_render_format f, int ofs)
		: location(l), format(f), offset(ofs), reserved(0)
	{

	}
};

struct constant_declaration{
	std::vector<uniform_declaration> uniforms;
};

struct auto_shader{
	auto_shader(int v)
	: shader(v)
	{
	}
	~auto_shader(){
		if (shader != 0){
			glDeleteShader(shader);
		}
	}
	bool operator !(){
		return shader == 0;
	}
	operator int(){
		return shader;
	}

	int shader;
};

struct ss_gl_render_pass
	: ss_render_pass
{
	ss_gl_render_pass();
	virtual ~ss_gl_render_pass();

	virtual void begin();
	virtual void end();

	int loadShader(int type, const char* src);
	bool link();

	void rebindPSCB(size_t slot);
	void rebindAllPSCB();

	int program;

	struct ss_gl_render_technique* tech;

	std::vector<constant_declaration>  ps_constants;
};

struct ss_gl_render_technique
	: ss_render_technique
{
	ss_gl_render_technique(size_t passCount);
	virtual ~ss_gl_render_technique();
	virtual size_t pass_count(){
		return count;
	}
	virtual ss_render_pass* get_pass(size_t index){
		return &passes[index];
	}
	virtual ss_render_input_layout* create_input_layout(
		ss_render_input_element* elements,
		size_t		count);

	ss_gl_render_pass*	passes;
	size_t				count;

	// must sort by usage (for input_layout compatible).
	std::vector<ss_render_input_usage> input_elements;

	ss_gl_render_device* device;
};

struct ss_gl_buffer_memory :
	ss_buffer_memory
{
	ss_gl_buffer_memory(
			size_t count, void* buf);
	virtual ~ss_gl_buffer_memory();

	virtual void* lock(){
		return buf;
	}

	virtual void unlock(){
	}

	virtual void copy(size_t ofs, void* src, size_t sz);

	size_t					size;
	void*					buf;
};

struct ss_gl_vertex_bind_info
{
	ss_buffer*	buffer;
	unsigned int		stride;
	unsigned int		offset;

	bool operator!=(const ss_gl_vertex_bind_info& other) const{
		return !((*this) == other);
	}
	bool operator==(const ss_gl_vertex_bind_info& other) const{
		return buffer == other.buffer &&
			stride == other.stride &&
			offset == other.offset;
	}
};


struct ss_gl_texture2d :
	ss_texture2d
{
	ss_gl_texture2d();
	virtual ~ss_gl_texture2d();

	unsigned int		name;
};

struct ss_gl_sampler_info
{
	ss_texture* texture;
	//TODO: set sampler state

	bool operator!=(const ss_gl_sampler_info& other) const{
		return !((*this) == other);
	}
	bool operator==(const ss_gl_sampler_info& other) const{
		return texture == other.texture;
	}
};

struct ss_gl_render_device
	: ss_render_device
{
	ss_gl_render_device();
	virtual ~ss_gl_render_device();

	virtual void clear_color(const ss_color& color);

	virtual void clear(int flags = SS_CF_COLOR);
	virtual void present();

	virtual void set_viewport(int left, int top, int width, int height);

	virtual void set_primitive_type(ss_primitive_type mt);
	virtual void draw(int count, int from);
	virtual void draw_index(int count, int from, int base);

	virtual ss_buffer_memory* create_memory_buffer(
		size_t bytes);

	virtual ss_render_technique* get_predefined_technique(ss_predefined_technique_type type);

	virtual void set_input_layout(ss_render_input_layout* layout);

	virtual void set_vertex_buffer(
			size_t start,
			size_t num,
			ss_buffer* const * buffer,
			const unsigned int* strides,
			const unsigned int* offset
		);

	virtual void unset_vertex_buffer(
		size_t start,
		size_t num
		);

	virtual void set_ps_constant_buffer(
		size_t start,
		size_t num,
		ss_buffer* const * buffer
		);
	virtual void unset_ps_constant_buffer(
		size_t start,
		size_t num
		);

	ss_texture2d* create_texture2d(
		size_t width, size_t height,
		ss_render_format format,
		const void* data);

	virtual void set_ps_texture2d_resource(
		size_t start,
		size_t num,
		ss_texture2d* const * textures
		);

	virtual void unset_ps_texture2d_resource(
		size_t start,
		size_t num
		);

//private:
	ss_primitive_type	pt;
	int					gl_pt;

	std::array<ss_gl_render_technique*, 3>		predefined_techiques;

	std::vector<ss_gl_vertex_bind_info>			vertex_buffers;

	std::vector<ss_buffer*>			ps_constant_buffers;

	std::vector<ss_gl_sampler_info>				ps_shader_resources;

	ss_gl_render_input_layout*	input_layout;

	ss_gl_render_pass*			pass;
	int							program;

	void release_predefined_techniques();
#ifdef WIN32
public:
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