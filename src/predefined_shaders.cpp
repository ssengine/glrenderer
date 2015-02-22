#include "device.h"

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
		"void main(){"
		"	gl_FragColor = vec4(0,0,0,1);"
		"}"
		);
	ASSUME_SUCCESS(fs);

	ASSUME_SUCCESS(pass0.link());
	glDetachShader(pass0.program, vs);
	glDetachShader(pass0.program, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindAttribLocation(pass0.program, 0, "a_position");

	ret->input_elements.push_back(SS_USAGE_POSITION);

	return ret;
}

ss_render_technique* ss_gl_render_device::get_predefined_technique(ss_predefined_technique_type type)
{
	size_t index = (size_t)type;
	if (index < predefined_techiques.size()){
		if (predefined_techiques[type]){
			return predefined_techiques[type];
		}

		switch (type){
		case SS_PDT_BLANK:
			predefined_techiques[0] = load_blank_technique();
			break;
		}

		if (predefined_techiques[type]){
			predefined_techiques[type]->device = this;
		}

		return predefined_techiques[type];
	}
	return NULL;
}

