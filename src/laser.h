#pragma once

#include <Arduino.h>

extern volatile bool laserTimeoutOccurred; // 激光测距超时标志
extern volatile bool laserError;           // 激光测距错误标志
void                 laserInit();
int16_t              getLaserDistance();