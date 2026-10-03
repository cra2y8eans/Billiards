#pragma once
#include <Arduino.h>

extern volatile bool
    isServoUpShortPressed,   // 舵机向上短按标志
    isServoUpLongPressed,    // 舵机向上长按标志
    isServoDownShortPressed, // 舵机向下短按标志
    isServoDownLongPressed,  // 舵机向下长按标志
    isPowerBTNShortPressed,  // 电源按钮短按标志
    isPowerBTNLongPressed;   // 电源按钮长按标志

void buttonInit();
void buttonTask(void* pvParameters);
