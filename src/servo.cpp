#include "servo.h"
#include "button.h"
#include "esp_log.h"
#include "laser.h"
#include "stateManager.h"
#include <ESP32Servo.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

static const char*       TAG                   = "servo"; // 日志标签
static const uint8_t     servoPin              = 1;       // 舵机控制引脚
static const int         ballDistanceThreshold = 600;     // 球距离阈值，单位：毫米
static const int         homeAngle             = 10;      // 回Home角度
static const int         readyAngle            = 90;      // 就绪角度
static const int         spotAngle             = 10;      // 摆球角度
static Servo             myServo;                         // 创建舵机对象
static SemaphoreHandle_t xServoRunSem = nullptr;          // 舵机运行信号量

static uint16_t angleToMicroseconds(int angle) {
  // 将角度转换为微秒脉冲宽度
  angle = constrain(angle, 0, 270);
  return map(angle, 0, 270, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
}

// 外部（stateManager）调用这个来唤醒任务
void servoWake() {
  if (xServoRunSem) xSemaphoreGive(xServoRunSem);
}

void servoInit() {
  ESP32PWM::allocateTimer(1);                                 // 舵机定时器
  myServo.setPeriodHertz(50);                                 // 设定舵机频率
  myServo.attach(servoPin, SERVO_PULSE_MIN, SERVO_PULSE_MAX); // 绑定引脚和最大、最小频率
  xServoRunSem = xSemaphoreCreateBinary();                    // 创建二值信号量，初始为空（不给通行证）
}

void servoUp() {
}

void servoDown() {
}

void servoPowerOff() {
  myServo.writeMicroseconds(angleToMicroseconds(homeAngle));
  myServo.detach();
}

void servoTask(void* pv) {
  while (1) {
    // ★ 只在暂停时才阻塞等信号量
    if (getCurrentState() != SYSTEM_RUNNING) {
      xSemaphoreTake(xServoRunSem, portMAX_DELAY); // 阻塞等唤醒
      continue;                                    // 醒来后回到顶部重新检查
    }

    // ---- RUNNING：正常干活 ----
    int d = getLaserDistance();
    if (d < 0 || d < ballDistanceThreshold) {
      vTaskDelay(pdMS_TO_TICKS(50)); // 有球/超时：歇一下
      continue;                      // 回到顶部，会重新检查状态
    }

    // 无球：摆球 + 回位
    myServo.writeMicroseconds(angleToMicroseconds(spotAngle));
    vTaskDelay(pdMS_TO_TICKS(1500));
    myServo.writeMicroseconds(angleToMicroseconds(homeAngle));
    vTaskDelay(pdMS_TO_TICKS(1500));
    // 回到顶部，会重新检查状态
  }
}
