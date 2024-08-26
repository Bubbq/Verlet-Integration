#include "headers/timer.h"

void start_timer(Timer *timer, double lifetime)
{
	timer->startTime = GetTime();
	timer->lifeTime = lifetime;
}

bool timer_done(Timer timer)
{
	return GetTime() - timer.startTime >= timer.lifeTime;
}