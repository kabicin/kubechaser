#include "time/Time.h"

Time::Time()
{
    currentTime = glfwGetTime();
}

Time::~Time()
{

}

// returns the time delta in seconds
double Time::GetDelta()
{
    previousTime = currentTime;
    currentTime = glfwGetTime();
    lastDelta = currentTime - previousTime;
    return lastDelta;
}

double Time::GetLastDelta() const
{
    return lastDelta;
}

// returns the time delta in milliseconds
double Time::GetDeltaMilliseconds()
{
    double deltaSeconds = GetDelta();
    return deltaSeconds * 1000;
}

// returns the number of frames elapsed since the last window update
double Time::GetDeltaFrames()
{
    return fps * lastDelta;
}

double Time::GetLastDeltaFrames() const
{
    return fps * lastDelta;
}
