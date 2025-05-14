#version 410 core

out vec4 FinalColor;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPosition;

in vec3 Position; 
in vec3 Normal; 

void main() {
	// vec3 diffuse = 0.5 * lightColor;
	// vec3 lightDirection = normalize(Position - cameraPosition);
	// float distance = length(lightDirection);
	// distance = distance * distance;
	// float NdotL = dot(Normal, lightDirection);
	// float diffuseIntensity = normalize(NdotL);

	// vec3 blinnphong = diffuseIntensity * objectColor * 1.0 / distance;
	FinalColor = vec4(objectColor, 1.0f);
}