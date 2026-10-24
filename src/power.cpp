#include "power.h"
#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "servo.h"
#include "stateManager.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "power";

static const uint8_t powerBTNPin = 10;
static const uint8_t MosPin      = 3;

void powerOn() {
  pinMode(MosPin, OUTPUT);
  pinMode(powerBTNPin, INPUT);
  digitalWrite(MosPin, HIGH); // 自锁
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(500));

  uint32_t pressStart = millis();
  while (millis() - pressStart < 2000) {
    if (digitalRead(powerBTNPin) == HIGH) {
      Serial.println("按下时长不够，关机");
      digitalWrite(MosPin, LOW);
      while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  // 按满 2 秒 → 响开机音
  Serial.println("开机");
  buzzerSound(SOUND_STARTUP); // 非阻塞
  vTaskDelay(pdMS_TO_TICKS(2000));
}

void powerTask(void* pvParameters) {
  while (1) {
    if (isPowerBTNLongPressed) {
      buzzerSound(SOUND_SHUTDOWN);
      vTaskDelay(pdMS_TO_TICKS(800));
      while (1) {
        isPowerBTNLongPressed = false;
        servoPowerOff();           // 舵机平滑回 home 并松力
        digitalWrite(MosPin, LOW); // 断电
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}