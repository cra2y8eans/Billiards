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

void stateToggle() {
  systemState_e cur = getCurrentState();
  stateSet(cur == SYSTEM_RUNNING ? SYSTEM_PAUSED : SYSTEM_RUNNING);
}

void stateSet(systemState_e newState) {
  taskENTER_CRITICAL(&systemStateMutex);
  systemState  = newState;
  bool running = (systemState == SYSTEM_RUNNING);
  taskEXIT_CRITICAL(&systemStateMutex);

  if (running) {
    ledSetMode(LED_ON, getBatteryColor(getBatteryLevel()), 0, 0);
  } else {
    ledSetMode(LED_ON, COLOR_ORANGE, 0, 0);
  }
}
