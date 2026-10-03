#pragma once

#include <Arduino.h>

extern volatile bool laserTimeoutOccurred; // 激光测距超时标志
void                 laserInit();
int16_t              getLaserDistance();