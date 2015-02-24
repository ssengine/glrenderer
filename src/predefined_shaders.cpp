#include "device.h"

#include <assert.h>

#define ASSUME_SUCCESS(p) do{if (!p){ delete ret;return NULL;}}while(0)

#ifdef SS_OPENGL_ES
#define FLOAT_PRECISION "precision mediump float;\n"
#else
#define FLOAT_PRECISION
#endif


static ss_gl_render_technique* load_blank_technique(){
	ss_gl_render_technique* ret = new ss_gl_render_technique(1);

	ss_gl_render_pass& pass0 = ret->passes[0];

	int vs = pass0.loadShader(GL_VERTEX_SHADER,
		//vertex shader
		FLOAT_PRECISION
		"attribute vec4 a_position;"
		"void main(){"
		"	gl_Position = a_position;"
		"}"
		);
	ASSUME_SUCCESS(vs);
	int fs = pass0.loadShader(GL_FRAGMENT_SHADER,
		//fragment shader
		FLOAT_PRECISION
		"uniform vec4 u_color;"
		"void main(){"
		"	gl_FragColor = u_color;"
		"}"
		);
	ASSUME_SUCCESS(fs);

	ASSUME_SUCCESS(pass0.link());

	pass0.ps_constants.resize(1);
	pass0.ps_constants[0].uniforms.push_back(
		uniform_declaration(
			glGetUniformLocation(pass0.program, "u_color"),
			SS_FORMAT_FLOAT32_RGBA,
			0
		)
		);

	glDetachShader(pass0.program, vs);
	glDetachShader(pass0.program, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindAttribLocation(pass0.program, 0, "a_position");

	ret->input_elements.push_back(SS_USAGE_POSITION);

	return ret;
}

static ss_gl_render_technique* load_standard_technique(){
	ss_gl_render_technique* ret = new ss_gl_render_technique(1);

	ss_gl_render_pass& pass0 = ret->passes[0];

	int vs = pass0.loadShader(GL_VERTEX_SHADER,
		//vertex shader
		FLOAT_PRECISION
		"attribute vec4 a_color;"
		"attribute vec4 a_position;"
		"attribute vec2 a_texcoord;"
		"varying vec2 v_texcoord;"
		"varying vec4 v_color;"
		"void main(){"
		"	gl_Position = a_position;"
		"	v_color = a_color;"
		"   v_texcoord = a_texcoord;"
		"}"
		);
	ASSUME_SUCCESS(vs);
	int fs = pass0.loadShader(GL_FRAGMENT_SHADER,
		//fragment shader
		FLOAT_PRECISION
		"uniform sampler2D u_texture0;"
		"varying vec2 v_texcoord;"
		"varying vec4 v_color;"
		"void main(){"
		"	gl_FragColor = v_color * texture2D(u_texture0, v_texcoord);"
		"}"
		);
	ASSUME_SUCCESS(fs);

	ASSUME_SUCCESS(pass0.link());

	pass0.ps_constants.resize(0);

	glUseProgram(pass0.program);
	glUniform1i(glGetUniformLocation(pass0.program, "u_texture0"), 0);

	glDetachShader(pass0.program, vs);
	glDetachShader(pass0.program, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindAttribLocation(pass0.program, 0, "a_position");
	glBindAttribLocation(pass0.program, 1, "a_color");
	glBindAttribLocation(pass0.program, 2, "a_texcoord");

	ret->input_elements.push_back(SS_USAGE_POSITION);
	ret->input_elements.push_back(SS_USAGE_DIFFUSE);
	ret->input_elements.push_back(SS_USAGE_TEXCOORD);

	return ret;
}

ss_render_technique* ss_gl_render_device::get_predefined_technique(ss_predefined_technique_type type)
{
	size_t index = (size_t)type;
	if (index < predefined_techiques.size()){
		if (predefined_techiques[type]){
			return predefined_techiques[type];
		}

		//Should not load pass between pass::begin() & pass::end()
		assert(pass == NULL);

		switch (type){
		case SS_PDT_BLANK:
			predefined_techiques[0] = load_blank_technique();
			break;
		case SS_PDT_STANDARD:
			predefined_techiques[2] = load_standard_technique();
			break;
		}

		if (predefined_techiques[type]){
			predefined_techiques[type]->device = this;
		}

		return predefined_techiques[type];
	}
	return NULL;
}

