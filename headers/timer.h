#ifndef TIMER_H
#define TIMER_H

#include "raylib.h"

typedef struct
{
	double startTime;
	double lifeTime;
} Timer;

void start_timer(Timer *timer, double lifetime);
bool timer_done(Timer timer);

#endif