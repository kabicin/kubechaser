#ifndef GRID_H
#define GRID_H
#include <glad/glad.h>
#include "glm.h"
#include "Vertex.h"
#include "shader/ShaderFactory.h"
#include "utility/Camera.h"
#include "shader/DrawAttributes.h"
class Grid
{
private:
	static constexpr float GridColor = 0.7f;
	GLuint VBO[3], VAO[3];              // cartesian cross section
	GLuint lines_VBO[3], lines_VAO[3];  // gridColor colored lines
	int num_lines[3];                   // number of lines per axis
	void initialize(int axis);
	void initializeLines(const glm::vec3& cartesianScale, int axis);
	CameraAttributes ca;
	DrawAttributes da;

public:
	Grid(const std::shared_ptr<Camera>& camera);
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera, bool withCross=false);
};
#endif
