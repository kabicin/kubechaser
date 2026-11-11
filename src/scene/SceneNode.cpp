#include "scene/SceneNode.h"
#include "scene/SceneDocument.h"

SceneNode::SceneNode() 
{
}

SceneNode::~SceneNode()
{
    for (int i = 0; i < staticObjects.size(); i++)
    {
        staticObjects[i] = nullptr;
    }
    for (int i = 0; i < dynamicObjects.size(); i++)
    {
        dynamicObjects[i] = nullptr;
    }
    staticObjects.clear();
    dynamicObjects.clear();
    sceneDocument = nullptr;
}

void SceneNode::AddStaticObject(std::shared_ptr<Entity> entity)
{
    staticObjects.push_back(entity);
}

void SceneNode::AddDynamicObject(std::shared_ptr<Entity> entity)
{
    dynamicObjects.push_back(entity);
}

void SceneNode::AttachSceneDocument(SceneDocument* document)
{
    this->sceneDocument = document;
}

SceneDocument* SceneNode::GetSceneDocument() const
{
    return sceneDocument;
}

void SceneNode::Render(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
    glClearColor(0, 0, 0, 1);
    // Drawing Entities
    for (int i = 0; i < staticObjects.size(); i++)
    {
        staticObjects[i]->Draw(factory, camera);
    }
    for (int i = 0; i < dynamicObjects.size(); i++)
    {
        dynamicObjects[i]->Draw(factory, camera);
    }
    if (sceneDocument != nullptr)
    {
        for (const auto& entity : sceneDocument->GetEntities())
        {
            if (entity)
            {
                entity->Draw(factory, camera);
            }
        }
    }
}
