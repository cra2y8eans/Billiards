#include "battery.h"
#include "Filters.h"
#include "buzzer.h"
#include "esp_log.h"
#include "led.h"
#include "servo.h"
#include "stateManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define K_OHM 1000

#define BATTERY_READING_INTERVAL (3 * 60 * 1000) // 满电时采样间隔 3 分钟
#define BATTERY_ALERT_INTERVAL (30 * 1000)       // 低电量报警间隔 30 秒

static const char*   TAG    = "bat";
static const uint8_t batPin = 0; // 电池电压采样引脚
static const float   R1     = 100.0 * K_OHM;
static const float   R2     = 47.0 * K_OHM;
static const float   VMAX   = 8.4 * K_OHM; // mV，满电电压
static const float   VMIN   = 6.0 * K_OHM; // mV，空电电压

static volatile float batVoltage    = 0.0f;
static volatile float batPercentage = 0.0f;
static portMUX_TYPE   batteryMux    = portMUX_INITIALIZER_UNLOCKED;

static Filters::LowPass batFilter(0.3f);

/** 读取一次电压并更新 batVoltage / batPercentage，返回 0.0~1.0 的电量比例 */
static float batteryReading() {
  taskENTER_CRITICAL(&batteryMux);
  batVoltage    = batFilter.update(analogReadMilliVolts(batPin)) * (R1 + R2) / R2;
  batPercentage = (batVoltage - VMIN) / (VMAX - VMIN) * 100.0f;
  batPercentage = constrain(batPercentage, 0.0f, 100.0f);
  taskEXIT_CRITICAL(&batteryMux);
  return batPercentage / 100.0f;
}

void batteryCheck(void* pvParameter) {
  while (1) {
    if (getCurrentState() != SYSTEM_RUNNING) {
      vTaskDelay(pdMS_TO_TICKS(100)); // 暂停时低频轮询
      continue;
    }

    float batteryLevel = batteryReading();
    // 低电量报警
    if (batPercentage <= 10.0f) {
      static uint32_t lastAlertTime = 0;
      if (millis() - lastAlertTime > BATTERY_ALERT_INTERVAL) {
        ESP_LOGW(TAG, "电量过低：%.1f%%，请及时充电", batPercentage);
        buzzerSound(SOUND_WARNING);
        lastAlertTime = millis();
      }
    }

    // 电量越低采样越频繁：满电 3 分钟 → 空电 1 秒
    uint32_t interval = (uint32_t)(batteryLevel * BATTERY_READING_INTERVAL);
    if (interval > BATTERY_READING_INTERVAL) interval = BATTERY_READING_INTERVAL;
    if (interval < 1000) interval = 1000;
    vTaskDelay(pdMS_TO_TICKS(interval));
  }
}

// ===================== 接口 =====================
void batteryInit() {
  analogReadResolution(12);
  pinMode(batPin, INPUT);
  batteryReading();
}

float getBatteryLevel() {
  float v;
  taskENTER_CRITICAL(&batteryMux);
  v = batPercentage;
  taskEXIT_CRITICAL(&batteryMux);
  return v;
}

float getBatteryVoltage() {
  float v;
  taskENTER_CRITICAL(&batteryMux);
  v = batVoltage;
  taskEXIT_CRITICAL(&batteryMux);
  return v;
}