#pragma once

#include <Arduino.h>

void  batteryInit();

/** 电池检测任务：周期采样、低电量报警、更新 sysRGB */
void  batteryCheck(void* pvParameter);

/** 电量百分比，范围 0.0 ~ 100.0 */
float getBatteryLevel();

/** 电池电压，单位 mV */
float getBatteryVoltage();