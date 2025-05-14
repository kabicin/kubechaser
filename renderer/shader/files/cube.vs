#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 Position;
out vec3 Normal;


void main() {
	gl_Position = Projection * View * Model * vec4(position, 1.0f);
	Position = vec3(Model * vec4(position, 1.0));
	Normal = normal;
}