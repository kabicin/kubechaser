#ifndef ASSET_H
#define ASSET_H
#include <string>
struct Asset {
    int Num = -1;
    std::string TextureName = "N/A";
    GLuint TextureNum = -1;
    int ImageWidth = 0;
    int ImageHeight = 0;
};
#endif
