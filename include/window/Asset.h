#ifndef ASSET_H
#define ASSET_H
#include <string>
#include <glad/glad.h>

struct Asset {
    int Num = -1;
    std::string TextureName = "N/A";
    GLuint TextureNum = 0; // 0 is GL's reserved "no texture" name
    int ImageWidth = 0;
    int ImageHeight = 0;
};
#endif
