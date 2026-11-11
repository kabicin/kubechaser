#ifndef WINDOWSERVICES_H
#define WINDOWSERVICES_H

#include <memory>

class Logger;
class ShaderFactory;
class Time;

struct WindowServices
{
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ShaderFactory> shaderFactory;
    std::shared_ptr<Time> time;
};

#endif
