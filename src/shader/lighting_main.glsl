vec3 norm = normalize(normal_fs_in);
vec3 viewDir = normalize(viewPos - pos_fs_in);

// phase 1: Directional lighting
vec3 result = vec3(0.0f, 0.0f, 0.0f);

if(and(lightSwitch, 1) == 1){
    result = CalcDirLight(dirLight, norm, viewDir, usingTexture);
}
if(and(lightSwitch, 1<<1) == 1<<1){
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, pos_fs_in, viewDir, usingTexture);
}
if(and(lightSwitch, 1<<2) == 1<<2){
    result += CalcSpotLight(spotlight, norm, pos_fs_in, viewDir, blinn, usingTexture);
}

if (lightSwitch == 0) {
    result = material.diffuse;
}
FinalColor = vec4(result, 1.0);
