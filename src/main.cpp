#include "battery.h"
#include "button.h"
#include "buzzer.h"
#include "laser.h"
#include "led.h"
#include "power.h"
#include "servo.h"
#include "stateManager.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void setup() {
  buzzerInit();
  xTaskCreate(buzzerUpdate, "buzzer", 2048, NULL, 3, NULL);

  powerOn();
  laserInit();
  servoInit();
  ledInit();
  buttonInit();
  batteryInit();


  // 优先级：舵机(3) > 按钮(2) = 电源(2) = LED(2) > 电池(1)
  xTaskCreate(servoTask, "servo", 4096, NULL, 3, NULL);
  xTaskCreate(buttonTask, "btn", 2048, NULL, 2, NULL);
  xTaskCreate(powerTask, "power", 2048, NULL, 2, NULL);
  xTaskCreate(ledUpdate, "led", 2048, NULL, 2, NULL);
  xTaskCreate(batteryCheck, "bat", 3072, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}