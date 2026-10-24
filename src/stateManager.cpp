#include "stateManager.h"
#include "esp_log.h"
#include "led.h"
#include <battery.h>
#include <freertos/FreeRTOS.h>
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

void stateSet(systemState_e newState) {
  taskENTER_CRITICAL(&systemStateMutex);
  systemState = newState;
  taskEXIT_CRITICAL(&systemStateMutex);
  switch (newState) {
  case SYSTEM_RUNNING:
    ledSetMode(LED_ON, getBatteryColor(getBatteryLevel()), 0, 0);
    break;
  case SYSTEM_PAUSED:
    ledSetMode(LED_ON, COLOR_RED, 0, 0);
    break;
  case SYSTEM_EMPTY:
    ledSetMode(LED_BLINK, COLOR_YELLOW, SHORT_FLASH_DURATION, SHORT_FLASH_INTERVAL);
    break;
  default:
    break;
  }
}
