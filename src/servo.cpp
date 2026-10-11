#include "servo.h"
#include "battery.h"
#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "laser.h"
#include "led.h"
#include "stateManager.h"
#include <ESP32Servo.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

static const char*   TAG                   = "servo"; // 日志标签
static const uint8_t servoPin              = 1;       // 舵机控制引脚
static const uint8_t servoUpPin            = 8;       // 舵机向上
static const uint8_t servoDownPin          = 9;       // 舵机向下
static const int     ballDistanceThreshold = 600;     // 球距离阈值，单位：毫米
static const int     homeAngle             = 180;     // 回Home角度
static const int     readyAngle            = 90;      // 就绪角度
static const int     spotAngle             = 10;      // 摆球角度
static Servo         myServo;                         // 创建舵机对象
static int           currentAngle = homeAngle;        // 记录当前角度

static uint16_t angleToMicroseconds(int angle) {
  // 将角度转换为微秒脉冲宽度
  angle = constrain(angle, 0, 270);
  return map(angle, 0, 270, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
}

void servoInit() {
  ESP32PWM::allocateTimer(1);                                 // 舵机定时器
  myServo.setPeriodHertz(50);                                 // 设定舵机频率
  myServo.attach(servoPin, SERVO_PULSE_MIN, SERVO_PULSE_MAX); // 绑定引脚和最大、最小频率
  myServo.writeMicroseconds(angleToMicroseconds(readyAngle));
}

void servoUp() {
  if (digitalRead(servoUpPin) == LOW) {
    currentAngle += 3; // 每 20ms 走 3°
    if (currentAngle > 270) currentAngle = 270;
    myServo.writeMicroseconds(angleToMicroseconds(currentAngle));
  }
}

void servoDown() {
  if (digitalRead(servoDownPin) == LOW) {
    currentAngle -= 3;
    if (currentAngle < 0) currentAngle = 0;
    myServo.writeMicroseconds(angleToMicroseconds(currentAngle));
  }
}

void servoPowerOff() {
  myServo.writeMicroseconds(angleToMicroseconds(homeAngle));
  myServo.detach();
}

void servoTask(void* pv) {
  while (1) {
    // ============ 暂停：手动模式 ============
    if (getCurrentState() != SYSTEM_RUNNING) {
      servoUp();
      servoDown();
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    // ============ 检测 ============
    int  d       = getLaserDistance();
    bool hasBall = (d >= 0 && d < ballDistanceThreshold);

    if (hasBall) {
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    // ============ 无球：摆一次 ============
    myServo.writeMicroseconds(angleToMicroseconds(spotAngle));
    currentAngle = spotAngle;
    vTaskDelay(pdMS_TO_TICKS(1500));

    myServo.writeMicroseconds(angleToMicroseconds(homeAngle));
    currentAngle = homeAngle;
    vTaskDelay(pdMS_TO_TICKS(1500));

    // ============ 摆完再检测 ============
    vTaskDelay(pdMS_TO_TICKS(1000)); // 等球稳定
    d       = getLaserDistance();
    hasBall = (d >= 0 && d < ballDistanceThreshold);

    if (!hasBall) {
      buzzerSound(SOUND_ERROR);
      ESP_LOGW(TAG, "料仓空，系统暂停，等待装填");
      stateSet(SYSTEM_PAUSED);
    }
    // 有球 → 什么都不做，回到循环顶继续检测
  }
}
