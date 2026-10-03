#include "power.h"
#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "servo.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "power";

static const uint8_t powerBTNPin = 10; // 电源开关按钮
static const uint8_t MosPin      = 3;  // 电源MOS管控制引脚

// void powerOn() {
//   pinMode(MosPin, OUTPUT);
//   pinMode(powerBTNPin, INPUT);
//   digitalWrite(MosPin, HIGH);
//   unsigned long pressedTime = millis();
//   while (1) {
//     Serial.begin(115200);
//     if (digitalRead(powerBTNPin) == LOW) {
//       unsigned releaseTime = millis();
//       if (releaseTime - pressedTime >= 2000) {
//         Serial.println("   开机");
//         break;
//       } else {
//         digitalWrite(MosPin, LOW);
//         Serial.println("按下时长不够，关机！");
//         delay(3000);
//       }
//     }
//   }
//   delay(3000);
// }

void powerOn() {
  pinMode(MosPin, OUTPUT);
  pinMode(powerBTNPin, INPUT);
  digitalWrite(MosPin, HIGH); // 自锁
  Serial.begin(115200);
  delay(50);
  uint32_t pressStart = millis();
  while (millis() - pressStart < 2000) {
    if (digitalRead(powerBTNPin) == LOW) {
      Serial.println("按下时长不够，关机");
      digitalWrite(MosPin, LOW);
      while (1)
        delay(1000);
    }
    delay(10);
  }
  // 按满 2 秒 → 响一声，告诉用户可以松手了
  Serial.println("开机");
  buzzerSound(SOUND_STARTUP); // 非阻塞，立即返回
}

void powerTask(void* pvParameters) {
  while (1) {
    if (isPowerBTNLongPressed) {
      ESP_LOGI(TAG, "关机中……");
      // 执行关机动作
      buzzerSound(SOUND_SHUTDOWN); // 播放关机提示音
      servoPowerOff();
      vTaskDelay(pdMS_TO_TICKS(200)); // 等待 1 秒，确保提示音播放完毕
      digitalWrite(MosPin, LOW);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
