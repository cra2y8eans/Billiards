#include "button.h"
#include "buzzer.h"
#include "led.h"
#include "power.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void setup() {
  buzzerInit();
  xTaskCreate(buzzerUpdate, "buzzer", 2048, NULL, 3, NULL);
  powerOn();
}

void loop() {
}
