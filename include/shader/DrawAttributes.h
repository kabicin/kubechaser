#ifndef DRAWATTRIBUTES_H
#define DRAWATTRIBUTES_H
#include "shader/MaterialAttributes.h"
#include <iosfwd>
#include <string>

struct DrawAttributes
{
	DrawAttributes()
	{
		wireframe = false;
		tesselate = false;
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
		instanced = false;
	}
	bool wireframe;
	bool tesselate;
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
	bool instanced;
	
	friend std::ostream& operator<<(std::ostream& os, const DrawAttributes& da);
	
	void serialize(std::ostream& os) const;
	static DrawAttributes deserialize(std::istream& is);

	// Writes the attributes as a JSON object. Nested lines are prefixed with
	// `indent` so the object can be embedded in a larger document.
	void serializeJSON(std::ostream& os, const std::string& indent = "") const;
};

#endif
