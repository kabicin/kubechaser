#version 410 core

out vec4 FinalColor;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPos;
uniform bool onClick;
uniform vec3 onClickColor;

in vec3 Position; 
in vec3 RawPosition;
in vec3 Normal; 

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
	// ambience
    vec3 ambient = light.ambient * material.ambient;

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	// specular
	vec3 viewDir = normalize(cameraPos - Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular); 

	vec3 result = ambient + diffuse + specular;
	if (onClick) {
		FinalColor = vec4(onClickColor, 1.0f);
	} else {
		FinalColor = vec4(result, 1.0f);
	}
}