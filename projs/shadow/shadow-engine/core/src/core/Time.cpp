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
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_ms = time_point_cast<milliseconds>(now);

    auto value = now_ms.time_since_epoch();
    double duration = value.count();

    if (startTime == 0)
        startTime = duration;
    timeSinceStart = duration - startTime;
}
