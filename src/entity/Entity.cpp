#include "entity/Entity.h"
#include "utility/Reader.h"
#include <iostream>

Entity::Entity()
{
    _id = acquireId();
}

Entity::~Entity()
{
    releaseTexture();
    releaseId(_id);
}

void Entity::releaseTexture()
{
    if (ownsTexture && TEX != 0)
    {
        glDeleteTextures(1, &TEX);
    }
    TEX = 0;
    ownsTexture = false;
}

int Entity::GetId()
{
    return _id;
}

void Entity::MoveUp(glm::vec3 trans)
{
    ca.translate = glm::vec3(ca.translate.x, ca.translate.y, trans.z);
}

void Entity::MoveBack(glm::vec3 trans)
{
    ca.translate = glm::vec3(ca.translate.x, ca.translate.y, trans.z);
}

void Entity::Translate(glm::vec3 trans)
{
    ca.translate += trans;
}

void Entity::SetTranslate(glm::vec3 trans)
{
    ca.translate = trans;
}

glm::vec3 Entity::GetTranslate() const
{
    return ca.translate;
}

void Entity::Scale(double scaleFactor)
{
    ca.scale *= static_cast<float>(scaleFactor);
}

void Entity::Scale(glm::vec3 givenScaleVector)
{
    ca.scale *= givenScaleVector;
}

void Entity::SetScale(glm::vec3 newScale)
{
    ca.scale = newScale;
}

void Entity::RotateX(float degrees)
{
    ca.rotateXdeg = degrees;
}

void Entity::RotateY(float degrees)
{
    ca.rotateYdeg = degrees;
}

void Entity::RotateZ(float degrees)
{
    ca.rotateZdeg = degrees;
}

void Entity::RotateXInc(float degrees)
{
    ca.rotateXdeg += degrees;
}

void Entity::RotateYInc(float degrees)
{
    ca.rotateYdeg += degrees;
}

void Entity::RotateZInc(float degrees)
{
    ca.rotateZdeg += degrees;
}

int Entity::acquireId()
{
    // usedIds is sorted, so the first gap is the smallest free ID
    int id = 0;
    for (int used : usedIds)
    {
        if (used != id) {
            break;
        }
        id++;
    }
    usedIds.insert(id);
    return id;
}

void Entity::releaseId(int id)
{
    usedIds.erase(id);
}

std::set<int> Entity::usedIds;

CameraAttributes Entity::GetCameraAttributes(){ return ca; }

bool Entity::AddTexture(const std::string& imagePath)
{
    // stb: load texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    if (!data) {
        const char* reason = stbi_failure_reason();
        std::cerr << "ERROR: Could not load texture " << imagePath
                  << ": " << (reason ? reason : "unknown error") << std::endl;
        return false;
    }

    releaseTexture();
    glGenTextures(1, &TEX);
    ownsTexture = true;
    glBindTexture(GL_TEXTURE_2D, TEX);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    textureName = Reader::FileNameFromPath(imagePath);
    usingTexture = true;
    return true;
}

void Entity::SetTexture(Asset asset)
{
    textureName = asset.TextureName;
    usingTexture = true;
    releaseTexture();
    TEX = asset.TextureNum; // borrowed: AssetPanel owns this texture
}

GLuint Entity::GetTexture()
{
    return TEX;
}

std::string Entity::GetName()
{
    return "Entity";
}

std::string Entity::GetTextureName()
{
    return textureName;
}
