#ifndef BASEWINDOW_H
#define BASEWINDOW_H
#include "BaseFrame.h"
#include "shader/ShaderFactory.h"
#include "utility/Camera.h"
#include "time/Time.h"
#include "window/WindowServices.h"

class BaseWindow : public BaseFrame
{
private:
    const WindowServices& services;
    std::shared_ptr<Camera> camera;

protected:
    std::shared_ptr<Logger> GetLogger() const;

public:
    BaseWindow(const WindowServices& services, int x, int y, int width, int height);

    virtual void KeyCallback(int key, int scancode, int action, int mods);
    void SetCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> GetCamera();

    std::shared_ptr<ShaderFactory> GetShaderFactory() const;

    std::shared_ptr<Time> GetTime() const;
};

#endif
