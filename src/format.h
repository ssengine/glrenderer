#pragma once

#include <ssengine/render/device.h>

inline size_t ss_render_format_ele_size(ss_render_format type){
	static size_t values[] = {
		0, 4, 4, 2
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 3){
		return values[iType];
	}
	return 0;
}

inline int ss_render_format_ele_type(ss_render_format type){
	static int values[] = {
		0, GL_FLOAT, GL_UNSIGNED_BYTE, GL_FLOAT
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 3){
		return values[iType];
	}
	return 0;
}

inline GLboolean ss_render_format_normalized(ss_render_format type){
	static GLboolean values[] = {
		0, GL_FALSE, GL_FALSE, GL_FALSE
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 3){
		return values[iType];
	}
	return 0;
}

inline int ss_render_format_format(ss_render_format type){
	static GLint values[] = {
		0, GL_RGBA, GL_RGBA
	};
	int iType = (int)type;
	if (iType >= 0 && iType <= 2){
		return values[iType];
	}
	return 0;
}