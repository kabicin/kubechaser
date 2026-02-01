#include "entity/Entity.h"

Entity::Entity()
{
    int emptyBit = getFreeBit();
    setBit(emptyBit);
    _id = emptyBit;
}

Entity::~Entity()
{
    clearBit(_id);
}

int Entity::GetId()
{
    return _id;
}

void Entity::MoveUp(glm::vec3 trans)
{
    double x = ca.translate.x, y = ca.translate.y;
    ca.translate = glm::vec3(x, y, trans.z);
}

void Entity::MoveBack(glm::vec3 trans)
{
    double x = translate.x, y = translate.y;
    ca.translate = glm::vec3(x, y, trans.z);
}

void Entity::Translate(glm::vec3 trans)
{
    ca.translate += trans;
}

void Entity::SetTranslate(glm::vec3 trans)
{
    ca.translate = trans;
}

void Entity::Scale(double scaleFactor)
{
    ca.scale = glm::vec3(scale.x * scaleFactor, scale.y * scaleFactor, scale.z * scaleFactor);
}

void Entity::Scale(glm::vec3 givenScaleVector)
{
    ca.scale = glm::vec3(scale.x * givenScaleVector.x, scale.y * givenScaleVector.y, scale.z * givenScaleVector.z);
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

int Entity::getFreeBit()
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entityBitmap[i] == 0) {
            return i;
        }
    }
    return 0;
}

void Entity::setBit(int bit)
{
    entityBitmap[bit] = 1;
}

void Entity::clearBit(int bit)
{
    entityBitmap[bit] = 0;
}

std::bitset<MAX_ENTITIES> Entity::entityBitmap;

CameraAttributes Entity::GetCameraAttributes(){ return ca; }

void Entity::AddTexture(const std::string& imagePath)
{
    usingTexture = true;

    glGenTextures(1, &TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);
    
    // stb: load texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    stbi_image_free(data);

    // set texture name
    const size_t start = imagePath.rfind("\\");
    if (std::string::npos != start)
    {
        textureName = imagePath.substr(start + 1, std::string::npos);
    }
    else {
        textureName = "Name not found";
    }
}

void Entity::SetTexture(Asset asset)
{
    textureName = asset.TextureName;
    usingTexture = true;
    TEX = asset.TextureNum;
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
