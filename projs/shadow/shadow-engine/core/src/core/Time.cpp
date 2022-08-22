#include "core/Time.h"
#include <chrono>

int Time::NOW = 0;//SDL_GetPerformanceCounter();
int Time::LAST = 0;
double Time::deltaTime_ms = 0;
double Time::deltaTime = 0;
double Time::startTime = 0;
double Time::timeSinceStart = 0;

void Time::UpdateTime()
{
    if (startTime == 0)
        startTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    timeSinceStart = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
}
