#version 410 core

out vec4 FinalColor;

in vec3 Position; 
in vec3 Normal; 
in vec2 TexturePosition;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPos;

struct Material {
	sampler2D diffuse1;
	sampler2D diffuse2;
	sampler2D specular;
	float shininess;
};

struct Light {
    vec3 position;
	vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
	// FinalColor = mix(texture(material.diffuse1, TexturePosition), texture(material.diffuse2, TexturePosition), 0.2);

	// ambience
    vec3 ambient = light.ambient * vec3(mix(texture(material.diffuse1, TexturePosition), texture(material.diffuse2, TexturePosition), 0.2));

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - Position);
	// vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * vec3(mix(texture(material.diffuse1, TexturePosition), texture(material.diffuse2, TexturePosition), 0.2)));

	// specular
	vec3 viewDir = normalize(cameraPos - Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexturePosition)); 

	vec3 result = ambient + diffuse + specular;
	FinalColor = vec4(result, 1.0f);

}