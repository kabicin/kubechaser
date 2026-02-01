vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, bool usingTexture)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
    // combine results
    vec3 ambient, diffuse, specular;
    if (usingTexture) {
        ambient  = light.ambient  * vec3(texture(material.diffuseMap, uv_fs_in));
        diffuse  = light.diffuse  * diff * vec3(texture(material.diffuseMap, uv_fs_in));
        specular = light.specular * spec * vec3(texture(material.diffuseMap, uv_fs_in));
    } else {
        ambient = light.ambient * material.diffuse;
        diffuse = light.diffuse * diff * material.diffuse;
        specular = light.specular * spec *  material.diffuse;
    }
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}