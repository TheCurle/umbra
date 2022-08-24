#pragma once

class Time
{

public:
    static int NOW;
	static int LAST;

	static double deltaTime;
	static double deltaTime_ms;

    static double timeSinceStart;
    static double startTime;

	static void UpdateTime();
};
