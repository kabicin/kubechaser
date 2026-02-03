#ifndef GRID_H
#define GRID_H
#include <glad/glad.h>
#include "glm.h"
#include "Vertex.h"
#include "shader/ShaderFactory.h"
#include "utility/Camera.h"
#include "shader/DrawAttributes.h"
#include <vector>
class Grid
{
private:
	static constexpr float GridColor = 0.7f;
	GLuint VBO[3], VAO[3];              // cartesian cross section (RGB)
	GLuint neutral_VBO[3], neutral_VAO[3]; // cartesian cross section (grid color)
	GLuint lines_VBO[3], lines_VAO[3];  // gridColor colored lines
	GLuint instance_VBO = 0;            // per-instance offsets
	int num_lines[3];                   // number of lines per axis
	void initialize(int axis);
	void initializeLines(const glm::vec3& cartesianScale, int axis);
	CameraAttributes ca;
	DrawAttributes da;
	float gridScale = 10.0f;
	glm::vec3 gridLinesScale = glm::vec3(5.0f, 5.0f, 5.0f);
	glm::vec4 neutralColor = glm::vec4(GridColor, GridColor, GridColor, 1.0f);

public:
	enum GridAxisMask {
		GridAxis_X = 1 << 0,
		GridAxis_Y = 1 << 1,
		GridAxis_Z = 1 << 2
	};
	Grid(const std::shared_ptr<Camera>& camera);
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera, bool withCross=false, int axisMask=GridAxis_X | GridAxis_Y | GridAxis_Z, bool drawLines=true, bool drawNeutralAxes=true);
	void DrawLinesInstanced(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera, const std::vector<glm::vec3>& offsets);
	void SetScale(float scale);
	void SetNeutralColor(const glm::vec4& color);
};
#endif
