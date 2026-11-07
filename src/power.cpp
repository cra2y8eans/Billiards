#include "power.h"
#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "led.h"
#include "servo.h"
#include "stateManager.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define POWER_ON_DELAY_MS 1500
#define POWER_OFF_DELAY_MS 2000
#define BOOT_DELAY_MS 2000

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
  while (millis() - pressStart < POWER_ON_DELAY_MS) {
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
  vTaskDelay(pdMS_TO_TICKS(BOOT_DELAY_MS));
}

void powerTask(void* pvParameters) {
  while (1) {
    if (isPowerBTNLongPressed) {
      isPowerBTNLongPressed = false;
      ESP_LOGI(TAG, "准备关机");

      buzzerSound(SOUND_SHUTDOWN);
      // 等音乐播完
      uint32_t t0 = millis();
      while (buzzerIsBusy() && millis() - t0 < POWER_OFF_DELAY_MS) {
        vTaskDelay(pdMS_TO_TICKS(20));
      }

      servoPowerOff(); // 舵机收拢并 detach
      ledOff();
      vTaskDelay(pdMS_TO_TICKS(POWER_OFF_DELAY_MS));
      digitalWrite(MosPin, LOW); // 彻底断电

      // 断电后跑不到这里，保险停住
      while (1)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}