#include "stateManager.h"
#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "laser.h"
#include "led.h"
#include "servo.h"
#include <battery.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

static const char*  TAG              = "stateManager";
static portMUX_TYPE systemStateMutex = portMUX_INITIALIZER_UNLOCKED;

systemState_e systemState = SYSTEM_PAUSED; // 系统状态，默认暂停

systemState_e getCurrentState() {
  systemState_e currentState;
  taskENTER_CRITICAL(&systemStateMutex);
  currentState = systemState;
  taskEXIT_CRITICAL(&systemStateMutex);
  return currentState;
}

void stateToggle() {
  taskENTER_CRITICAL(&systemStateMutex);
  systemState  = (systemState == SYSTEM_RUNNING) ? SYSTEM_PAUSED : SYSTEM_RUNNING;
  bool running = (systemState == SYSTEM_RUNNING);
  taskEXIT_CRITICAL(&systemStateMutex);

  if (running) {
    servoWake(); // 唤醒任务
    ledSetMode(LED_ON, getBatteryColor(getBatteryLevel()), 0, 0);
  } else {
    ledSetMode(LED_BLINK, COLOR_RED, SHORT_FLASH_DURATION, SHORT_FLASH_INTERVAL);
  }
  // 暂停时什么都不做 → 任务下一轮发现 PAUSED，就阻塞在 Take 上
}
