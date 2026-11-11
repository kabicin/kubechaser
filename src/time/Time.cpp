#include "time/Time.h"

Time::Time()
{
    currentTime = glfwGetTime();
    previousTime = currentTime;
}

Time::~Time()
{

}

void Time::Update()
{
    previousTime = currentTime;
    currentTime = glfwGetTime();
    lastDelta = currentTime - previousTime;
}

double Time::GetDelta() const
{
    return lastDelta;
}

double Time::GetDeltaMilliseconds() const
{
    return lastDelta * 1000;
}

double Time::GetDeltaFrames() const
{
    return fps * lastDelta;
}
