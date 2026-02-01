#ifndef ASSETPANEL_H
#define ASSETPANEL_H
#include "BaseWindow.h"
#include "stb.h"
#include <vector>
#include <bitset>
#include "SceneBuilder.h"
#define MAX_ASSETS 32

class AssetPanel : public BaseWindow
{
private:
    const float DEFAULT_WIDTH = 100.0f;
    const float DEFAULT_HEIGHT = 100.0f;
    std::bitset<MAX_ASSETS> assetBitmap;
    // bitmap functions
    int getFreeBit() const;
    void setBit(int bit);
    void clearBit(int bit);

    std::vector<Asset *> imageList;
    int addAsset(char *filename, GLuint image, int imageWidth, int imageHeight);
    std::vector<Asset *>::iterator deleteAsset(std::vector<Asset *>::iterator it, int assetNum);

public:
    AssetPanel(int x, int y, int width, int height);
    ~AssetPanel();
    void Render();
    bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
};

#endif
