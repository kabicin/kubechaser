vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, bool givenBool, bool usingTexture){

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    if (theta > light.cutOff)
    {
        float diff = max(dot(normal, lightDir), 0.0);
    
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0; 
        if(givenBool)
        {
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 4.0);
        }
        else
        {
            vec3 reflectDir = reflect(-lightDir, normal);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess / 4.0);
        }

        vec3 ambient, diffuse, specular;
        if (usingTexture) {
            ambient = light.ambient * texture(material.diffuseMap, uv_fs_in).rgb;
            diffuse = light.diffuse * diff * texture(material.diffuseMap, uv_fs_in).rgb;
            specular = light.specular * spec * texture(material.diffuseMap, uv_fs_in).rgb;
        } else {
            ambient = light.ambient * material.diffuse;
            diffuse = light.diffuse * diff * material.diffuse;
            specular = light.specular * spec *  material.diffuse;
        }

        // attenuation
        float distance    = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

        ambient  *= attenuation * intensity;
        diffuse   *= attenuation * intensity;
        specular *= attenuation * intensity;

        vec3 result = ambient + diffuse + specular;
        return result;
    } else {
        if (usingTexture) {
            return light.ambient * texture(material.diffuseMap, uv_fs_in).rgb;
        }
        return light.ambient * material.diffuse;
    }
}