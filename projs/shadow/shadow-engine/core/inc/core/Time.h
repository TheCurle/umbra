#pragma once

class Time
{

public:
    static int NOW;
	static int LAST;

	static double deltaTime;
	static double deltaTime_ms;

    static size_t timeSinceStart;
    static size_t startTime;

	static void UpdateTime();
};
