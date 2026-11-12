#include "window/AssetPanel.h"
#include "entity/Entity.h"
#include "logger/Logger.h"
#include "utility/Reader.h"

namespace
{
const char* const kShapeNames[] = { "Triangle", "Quad" };
}

AssetPanel::AssetPanel(
    const WindowServices& services,
    SceneDocument& sceneDocument,
    int x,
    int y,
    int width,
    int height)
    : BaseWindow(services, x, y, width, height),
      sceneDocument(sceneDocument)
{
}

AssetPanel::~AssetPanel()
{
    for (const auto& asset : imageList)
    {
        glDeleteTextures(1, &asset->TextureNum);
    }
}

void AssetPanel::Render()
{
    //ClearBackground(0.19f, 0.28f, 0.37f, 1.0f);
    glViewport(GetX(), GetY(), GetWidth(), GetHeight()); 

    {
	    size_t numAssets = imageList.size();
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetY()));
        ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
        ImGui::Begin("Asset Manager", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (ImGui::Button("Add Asset")) {
            if (numAssets >= MAX_ASSETS) {
                ImGui::OpenPopup("Too Many Assets");
            } else {
                loadError.clear();
                ImGui::OpenPopup("Load Asset");
            }
        }

        // Load an image file from disk as a new asset
        bool loadAssetOpen = true;
        if (ImGui::BeginPopupModal("Load Asset", &loadAssetOpen, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Image file path:");
            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();
            const bool submitted = ImGui::InputText("##assetPath", assetPath, IM_ARRAYSIZE(assetPath),
                ImGuiInputTextFlags_EnterReturnsTrue);
            if (!loadError.empty())
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", loadError.c_str());

            if (ImGui::Button("Load") || submitted)
            {
                GLuint texture = 0;
                int width = 0;
                int height = 0;
                if (assetPath[0] == '\0')
                {
                    loadError = "Enter a file path.";
                }
                else if (!LoadTextureFromFile(assetPath, &texture, &width, &height))
                {
                    loadError = std::string("Could not load image: ") + assetPath;
                    GetLogger()->LogMessage(loadError);
                }
                else
                {
                    addAsset(assetPath, texture, width, height);
                    assetPath[0] = '\0';
                    loadError.clear();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // Too many assets
        bool tooManyAssets = true;
        if (ImGui::BeginPopupModal("Too Many Assets", &tooManyAssets))
        {
            ImGui::Text("There are too many assets loaded");
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        if (numAssets == 0) 
        {
            ImGui::Text("No assets loaded.");
        }
        ImGui::Text("Loaded Assets = %zu", numAssets);

        int i = 0;
        for(auto it = imageList.begin(); it < imageList.end();)
        {
            Asset* ia = it->get();
            int assetNum = ia->Num;
            // Each asset needs its own ID scope so its child window, buttons
            // and popups don't collide with those of the other assets.
            ImGui::PushID(assetNum);
            float multiplier = DEFAULT_WIDTH / ia->ImageWidth;
            float xoffset = (i % 2) * (DEFAULT_WIDTH);
            float yoffset = (i / 2) * (DEFAULT_HEIGHT);

            ImGui::SetNextWindowPos(ImVec2(GetX() + 5.0f + xoffset, GetY() + 70.0f + yoffset));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(255, 255, 255, 255));

            ImGui::BeginChild("Asset Manager - Image", ImVec2(DEFAULT_WIDTH, DEFAULT_HEIGHT), 0);
            if (ImGui::Button("X")) {
                if (!deleteAssetDontAskAgain)
                {
                    ImGui::OpenPopup("Delete Asset");
                }
                else
                {
                    deletedId = i;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                addShapeIndex = -1;
                ImGui::OpenPopup("Add Asset");
            }

            // Add modal
            bool addOpen = true;
            if (ImGui::BeginPopupModal("Add Asset", &addOpen))
            {
                ImGui::Text("Add an asset to the scene builder");
                const char* shape = addShapeIndex >= 0 ? kShapeNames[addShapeIndex] : nullptr;
                ImGui::Text("Shape: ");
                if (ImGui::BeginCombo("##combo", shape))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(kShapeNames); n++)
                    {
                        const bool is_selected = (addShapeIndex == n);
                        if (ImGui::Selectable(kShapeNames[n], is_selected))
                        {
                            addShapeIndex = n;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add") && addShapeIndex >= 0)
                {
                    shape = kShapeNames[addShapeIndex];
                    std::shared_ptr<Entity> entity = sceneDocument.AddEntity(shape, glm::vec3(0.0f, 0.5f, 0.0f));
                    entity->SetTexture(*ia);
                  
                    GetLogger()->LogMessage("Added " + std::string(shape) + " with texture " + ia->TextureName);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            // Delete modal
            bool deleteOpen = true;
            if (ImGui::BeginPopupModal("Delete Asset", &deleteOpen))
            {
                ImGui::Text("Are you sure you want to delete this asset?");
                ImGui::Checkbox("Don't ask me again", &deleteAssetDontAskAgain);
                if (ImGui::Button("Yes"))
                {
                    deletedId = i;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::Image((void*)(intptr_t)ia->TextureNum, ImVec2(DEFAULT_WIDTH, ia->ImageHeight * multiplier));
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopID();

            // Delete only after the asset is no longer referenced this frame.
            if (deletedId == i)
            {
                it = deleteAsset(it, assetNum);
                deletedId = -1;
            }
            else
            {
                it++;
                i++;
            }
        }
       
        ImGui::End();
    }
}

int AssetPanel::addAsset(const std::string& filename, GLuint image, int imageWidth, int imageHeight)
{
    const std::string textureName = Reader::FileNameFromPath(filename);
    auto ia = std::make_unique<Asset>();
    ia->Num = getFreeBit();
    ia->TextureName = textureName;
    ia->TextureNum = image;
    ia->ImageWidth = imageWidth;
    ia->ImageHeight = imageHeight;
    const int assetNum = ia->Num;
    imageList.push_back(std::move(ia));
    setBit(assetNum);

    GetLogger()->LogMessage("Added image: " + textureName);
    return assetNum;
}

std::vector<std::unique_ptr<Asset>>::iterator AssetPanel::deleteAsset(std::vector<std::unique_ptr<Asset>>::iterator it, int assetNum)
{
    // Entities that borrowed this texture must stop referencing it before it
    // is deleted, or they would keep a dangling (and later reusable) GL name.
    const GLuint texture = (*it)->TextureNum;
    for (const auto& entity : sceneDocument.GetEntities())
    {
        entity->ReleaseBorrowedTexture(texture);
    }
    glDeleteTextures(1, &(*it)->TextureNum);
    GetLogger()->LogMessage("Deleted image: " + std::to_string(assetNum));
    clearBit(assetNum);
    return imageList.erase(it);
}

int AssetPanel::getFreeBit() const
{
    for (int i = 0; i < MAX_ASSETS; i++)
    {
        if (assetBitmap[i] == 0) {
            return i;
        }
    }
    return 0;
}

void AssetPanel::setBit(int bit)
{
    assetBitmap[bit] = 1;
}

void AssetPanel::clearBit(int bit)
{
    assetBitmap[bit] = 0;
}

bool AssetPanel::LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}
