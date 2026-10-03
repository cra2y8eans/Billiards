#pragma once

#include <Arduino.h>

#define SERVO_ANGLE_RANGE 270 // 舵机角度范围
#define SERVO_PULSE_MIN 500   // 舵机最小脉冲宽度
#define SERVO_PULSE_MAX 2500  // 舵机最大脉冲宽度
#define ANGLE_MIN 0           // 测试下限
#define ANGLE_MAX 270         // 舵机行程上限  ← 关键


void servoInit();
void servoWake();
void servoUp();
void servoDown();
void servoPowerOff();
void servoTask(void* pvParameters);