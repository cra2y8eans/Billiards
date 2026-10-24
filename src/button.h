#pragma once
#include <Arduino.h>

extern volatile bool
    isPowerBTNShortPressed, // 电源按钮短按标志
    isPowerBTNLongPressed;  // 电源按钮长按标志

void buttonInit();
void buttonTask(void* pvParameters);
