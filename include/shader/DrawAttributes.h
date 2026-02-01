#ifndef DRAWATTRIBUTES_H
#define DRAWATTRIBUTES_H
#include "shader/MaterialAttributes.h"

struct DrawAttributes
{
	DrawAttributes()
	{
		wireframe = false;
		tesselate = false;
		phong_tesselate = false;
		normal = false;
		texture = false;
		passthrough = false;
		color = false;
		useVertexColor = true;
		solidColor = glm::vec3(1.0f, 1.0f, 1.0f);
		materialAttributes = MaterialAttributes();

		// light attributes
		lightSwitch = 0x7;
		lightBlinn = false;
	}
	bool wireframe;
	bool tesselate;
	bool phong_tesselate;
	bool normal;
	bool texture;
	bool passthrough;
	bool color;
	bool useVertexColor;
	glm::vec3 solidColor;

	MaterialAttributes materialAttributes;

	// light attributes
	unsigned int lightSwitch;
	bool lightBlinn;
};
#endif
