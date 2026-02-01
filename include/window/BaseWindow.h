#ifndef BASEWINDOW_H
#define BASEWINDOW_H
#include "BaseFrame.h"
#include "shader/ShaderFactory.h"
#include "utility/Camera.h"
#include "time/Time.h"

class BaseWindow : public BaseFrame
{
private:
    static std::shared_ptr<ShaderFactory> factory;
    static std::shared_ptr<Time> time;
    std::shared_ptr<Camera> camera;

public:
    BaseWindow(int x, int y, int width, int height);

    virtual void KeyCallback(int key, int scancode, int action, int mods);
    void SetCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> GetCamera();

    static void AttachShaderFactory(std::shared_ptr<ShaderFactory> factory);
    std::shared_ptr<ShaderFactory> GetShaderFactory();

    static void AttachTime(std::shared_ptr<Time> time);
    std::shared_ptr<Time> GetTime();
};

#endif
