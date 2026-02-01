#ifndef TIME_H
#define TIME_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Time
{
private:
    double previousTime, currentTime;
    double fps = 60;
    double lastDelta = 0.0;

public:
    Time();
    ~Time();
    double GetDelta();
    double GetLastDelta() const;
    double GetDeltaMilliseconds();
    double GetDeltaFrames();
};
#endif
