#ifndef TIME_H
#define TIME_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Time
{
private:
    double previousTime = 0.0, currentTime = 0.0;
    double fps = 60;
    double lastDelta = 0.0;

public:
    Time();
    ~Time();

    void Update();

    double GetDelta() const;
    double GetDeltaMilliseconds() const;
    double GetDeltaFrames() const;
};
#endif
