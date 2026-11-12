#ifndef ASSETPANEL_H
#define ASSETPANEL_H
#include "BaseWindow.h"
#include "stb.h"
#include <memory>
#include <vector>
#include <bitset>
#include <string>
#include "scene/SceneDocument.h"
#define MAX_ASSETS 32

class AssetPanel : public BaseWindow
{
private:
    SceneDocument& sceneDocument;
    const float DEFAULT_WIDTH = 100.0f;
    const float DEFAULT_HEIGHT = 100.0f;
    std::bitset<MAX_ASSETS> assetBitmap;
    // bitmap functions
    int getFreeBit() const;
    void setBit(int bit);
    void clearBit(int bit);

    std::vector<std::unique_ptr<Asset>> imageList;

    // UI state
    bool deleteAssetDontAskAgain = false;
    int deletedId = -1;            // list position of the asset to delete this frame
    char assetPath[1024] = "";     // "Load Asset" modal input
    std::string loadError;         // "Load Asset" modal error message
    int addShapeIndex = -1;        // "Add Asset" modal selection into kShapeNames
    int addAsset(const std::string& filename, GLuint image, int imageWidth, int imageHeight);
    std::vector<std::unique_ptr<Asset>>::iterator deleteAsset(std::vector<std::unique_ptr<Asset>>::iterator it, int assetNum);

public:
    AssetPanel(
        const WindowServices& services,
        SceneDocument& sceneDocument,
        int x,
        int y,
        int width,
        int height);
    ~AssetPanel();
    void Render();
    bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
};

#endif
