#include "device.h"
#include <set>
#include <ssengine/log.h>

#include <algorithm>
#include <iterator>

#include <assert.h>

//TODO: check if format is right (between input_layout and buffer).

void ss_gl_render_input_layout::use(ss_gl_render_input_layout* old){
	std::vector<int> usedSlots;
	for (size_t i = 0; i < elements.size(); i++){
		// check whether vertex is binded.
		auto& ele = elements[i];
		ss_gl_vertex_bind_info& vbi = device->vertex_buffers[ele.slot];
		ele.disabled = vbi.buffer == NULL;
		if (!ele.disabled){
			usedSlots.push_back(ele.location);
		}
	}
	std::sort(usedSlots.begin(), usedSlots.end());
	
	std::vector<int> oldUsedSlots;
	if (old){
		for (size_t i = 0; i < old->elements.size(); i++){
			if (!old->elements[i].disabled){
				oldUsedSlots.push_back(old->elements[i].location);
			}
		}
		std::sort(oldUsedSlots.begin(), oldUsedSlots.end());
	}

	std::vector<int> out;
	// Close arrays for unused input 
	std::set_difference(
		oldUsedSlots.begin(), oldUsedSlots.end(),
		usedSlots.begin(), usedSlots.end(),
		std::insert_iterator<std::vector<int> >(out, out.begin()));

	for (auto itor = out.begin(); itor != out.end(); ++itor){
		glDisableVertexAttribArray(*itor);
	}
	out.clear();
	// Open arrays for new input.
	std::set_difference(
		usedSlots.begin(), usedSlots.end(),
		oldUsedSlots.begin(), oldUsedSlots.end(),
		std::insert_iterator<std::vector<int> >(out, out.begin()));

	for (auto itor = out.begin(); itor != out.end(); ++itor){
		glEnableVertexAttribArray(*itor);
	}
	out.clear();

	for (size_t i = 0; i < elements.size(); i++){
		rebind(i);
	}
}

inline size_t ss_render_format_ele_size(ss_render_format type){
	static size_t values[] = {
		0, 4
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 2){
		return values[iType];
	}
	return 0;
}

inline int ss_render_format_ele_type(ss_render_format type){
	static int values[] = {
		0, GL_FLOAT
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 2){
		return values[iType];
	}
	return 0;
}

inline GLboolean ss_render_format_normalized(ss_render_format type){
	static GLboolean values[] = {
		0, GL_FALSE
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 2){
		return values[iType];
	}
	return 0;
}

void ss_gl_render_input_layout::rebind(size_t i){
	ss_gl_render_input_element& ele = elements[i];
	unsigned int location = ele.location;
	ss_gl_vertex_bind_info& vbi = device->vertex_buffers[ele.slot];

	//TODO: if vertex_buffer_memory:

	if (vbi.buffer){
		if (ele.disabled){
			ele.disabled = false;
			glEnableVertexAttribArray(location);
		}
		const char* buffer = reinterpret_cast<const char*>(
			static_cast<ss_gl_vertex_buffer_memory*>(vbi.buffer)->buf
			);

		const char* pointer = buffer + vbi.offset + ele.alignOffset;

		glVertexAttribPointer(
			location,
			ss_render_format_ele_size(ele.format),
			ss_render_format_ele_type(ele.format),
			ss_render_format_normalized(ele.format),
			vbi.stride,
			pointer
			);
	}
	else {
		if (!ele.disabled){
			ele.disabled = false;
			glDisableVertexAttribArray(location);
		}
	}
}

void ss_gl_render_input_layout::rebindVB(size_t slot){
	for (size_t i = 0; i < elements.size(); i++){
		if (elements[i].slot == slot){
			rebind(i);
		}
	}
}

ss_gl_render_pass::ss_gl_render_pass()
	: program(glCreateProgram())
{
	
}

ss_gl_render_pass::~ss_gl_render_pass(){
	if (program != 0){
		::glDeleteProgram(program);
		program = 0;
	}
}

void ss_gl_render_pass::begin(){
	assert(tech->device->pass == NULL);
	if (tech->device->pass != this){
		tech->device->pass = this;
	}
	if (tech->device->program != program){
		tech->device->program = program;
		glUseProgram(program);
	}
	rebindAllPSCB();
}

void ss_gl_render_pass::end(){
	assert(tech->device->pass == this);
	if (tech->device->pass == this){
		tech->device->pass = NULL;
	}
}

int ss_gl_render_pass::loadShader(int type, const char* src){
	const GLuint shader = glCreateShader(type);
	if (!shader) {
		return false;
	}
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	int compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled){
		int infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		char* buf = new char[infoLen+1];
		glGetShaderInfoLog(shader, infoLen, &infoLen, buf);
		SS_LOGE("GL Shader Compile Error: \n%s", buf);
		delete[] buf;
		return false;
	}
	glAttachShader(program, shader);
	return shader;
}

bool ss_gl_render_pass::link(){
	glLinkProgram(program);

	int linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked){
		int infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		char* buf = new char[infoLen + 1];
		glGetProgramInfoLog(program, infoLen, &infoLen, buf);
		SS_LOGE("GL Program Link Error: \n%s", buf);
		delete[] buf;
		return false;
	}
	return true;
}

void ss_gl_render_pass::rebindPSCB(size_t slot){
	if (slot < ps_constants.size()){
		ss_gl_constant_buffer_memory* buf = (ss_gl_constant_buffer_memory*)tech->device->ps_constant_buffers[slot];

		constant_declaration& cons = ps_constants[slot];
		for (auto itor = cons.uniforms.begin();
			itor != cons.uniforms.end();
			++itor){
			const char* buffer = reinterpret_cast<const char*>(buf->buf) + itor->offset;
			switch (itor->format){
			case SS_FORMAT_FLOAT32_RGBA:
				{
					const float* values = reinterpret_cast<const float*>(buffer);
					glUniform4f(itor->location, values[0], values[1], values[2], values[3]);
				}
				break;
			default:
				break;
			}
		}
	}
}

void ss_gl_render_pass::rebindAllPSCB(){
	for (size_t i = 0; i < ps_constants.size(); ++i){
		rebindPSCB(i);
	}
}

ss_gl_render_technique::ss_gl_render_technique(size_t passCount)
	: count(passCount),
	passes(new ss_gl_render_pass[passCount])
{
	for (size_t i = 0; i < count; i++){
		passes[i].tech = this;
	}
}

ss_gl_render_technique::~ss_gl_render_technique(){
	delete[] passes;
}

ss_render_input_layout* ss_gl_render_technique::create_input_layout(
	ss_render_input_element* elements,
	size_t		count
	)
{
	if (count > device->vertex_buffers.size()){
		SS_LOGE("Input layout element count exceed max limit: %d.", (int)device->vertex_buffers.size());
		return NULL;
	}
	std::vector<ss_gl_render_input_element> ret_elements;

	std::set<int> usedLocations;

	for (size_t i = 0; i < count; ++i){
		ss_gl_render_input_element ele;
		ele.usage = elements[i].usage;
		ele.index = elements[i].index;
		ele.format = elements[i].format;
		ele.alignOffset = elements[i].alignOffset;
		ele.slot = elements[i].slot;
		
		// find slot
		auto itor = std::find(input_elements.begin(), input_elements.end(), elements[i].usage);
		if (itor == input_elements.end()){
			// unused input
			SS_LOGW("Unused input element %d", i);
			ele.location = -1;
		}
		else {
			ele.location = itor - input_elements.begin();
			if (usedLocations.find(ele.location) != usedLocations.end()){
				SS_LOGE("Duplicated input at %d", i);
				return NULL;
			}
			usedLocations.insert(ele.location);
		}
		ret_elements.push_back(ele);
	}

	for (size_t i = 0; i < input_elements.size(); i++){
		if (usedLocations.find(i) == usedLocations.end()){
			SS_LOGW("Unbinded input element for location: %d", (int)i);
		}
	}

	ss_gl_render_input_layout* ret = new ss_gl_render_input_layout();
	ret->elements.swap(ret_elements);
	ret->device = this->device;

	return ret;
}
