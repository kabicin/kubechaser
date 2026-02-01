vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, bool usingTexture)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // combine results
    vec3 ambient, diffuse, specular;
    if (usingTexture) {
        ambient  = light.ambient  * texture(material.diffuseMap, uv_fs_in).rgb;
        diffuse  = light.diffuse  * diff * texture(material.diffuseMap, uv_fs_in).rgb;
        specular = light.specular * spec * texture(material.diffuseMap, uv_fs_in).rgb;
    } else {
        ambient = light.ambient * material.diffuse;
        diffuse = light.diffuse * diff * material.diffuse;
        specular = light.specular * spec *  material.diffuse;
    }
    return (ambient + specular + diffuse);
}