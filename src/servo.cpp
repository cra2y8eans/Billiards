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
#include <freertos/task.h>

static const char*   TAG                   = "servo";
static const uint8_t servoPin              = 1;
static const uint8_t servoUpPin            = 8;
static const uint8_t servoDownPin          = 9;
static const int     ballDistanceThreshold = 900;

static const int homeAngle  = 190; // 关机收拢
static const int readyAngle = 120; // 就绪位
static const int spotAngle  = 30;  // 摆球位

// 每走 1 度等多少 ms（越大越慢）
#define SERVO_DEGREE_DELAY_MS 15

static Servo  myServo;
static int    currentAngle = homeAngle;
volatile bool hasBall      = false;

static void servoMoveTo(int target) {
  target = constrain(target, 0, 270);
  if (target == currentAngle) return;

  if (target > currentAngle) {
    for (int a = currentAngle; a <= target; a++) {
      myServo.write(a);
      vTaskDelay(pdMS_TO_TICKS(SERVO_DEGREE_DELAY_MS));
    }
  } else {
    for (int a = currentAngle; a >= target; a--) {
      myServo.write(a);
      vTaskDelay(pdMS_TO_TICKS(SERVO_DEGREE_DELAY_MS));
    }
  }
  currentAngle = target;
}

void servoInit() {
  pinMode(servoUpPin, INPUT_PULLUP);
  pinMode(servoDownPin, INPUT_PULLUP);

  ESP32PWM::allocateTimer(1);
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, SERVO_PULSE_MIN, SERVO_PULSE_MAX);

  servoMoveTo(readyAngle);
  currentAngle = readyAngle;
  stateSet(SYSTEM_PAUSED);
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void servoUp() {
  if (digitalRead(servoUpPin) == LOW) {
    currentAngle += 1;
    if (currentAngle > 270) currentAngle = 270;
    myServo.write(currentAngle);
  }
}

void servoDown() {
  if (digitalRead(servoDownPin) == LOW) {
    currentAngle -= 1;
    if (currentAngle < 0) currentAngle = 0;
    myServo.write(currentAngle);
  }
}

void servoPowerOff() {
  if (currentAngle == homeAngle) return;
  servoMoveTo(homeAngle);
  vTaskDelay(pdMS_TO_TICKS(500));
  myServo.detach();
}

void servoTask(void* pv) {
  while (1) {
    if (getCurrentState() != SYSTEM_RUNNING) {
      servoUp();
      servoDown();
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    int d = getLaserDistance();
    if (d < 0) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    hasBall = (d < ballDistanceThreshold);
    if (hasBall) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    ESP_LOGI(TAG, "无球，开始摆球");
    vTaskDelay(pdMS_TO_TICKS(2000));
    servoMoveTo(spotAngle);
    vTaskDelay(pdMS_TO_TICKS(2000));
    servoMoveTo(readyAngle);
    vTaskDelay(pdMS_TO_TICKS(2000));

    d = getLaserDistance();
    if (d < 0) continue;

    hasBall = (d < ballDistanceThreshold);
    if (!hasBall) {
      servoMoveTo(readyAngle);
      buzzerSound(SOUND_ERROR);
      ESP_LOGW(TAG, "料仓空，系统暂停，等待装填");
      if (getCurrentState() != SYSTEM_PAUSED) stateSet(SYSTEM_EMPTY);
    }
  }
}