#include "button.h"
#include "buzzer.h"
#include "esp_log.h"
#include "led.h"
#include "stateManager.h"
#include <OneButton.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "button";

static const uint8_t servoUpPin   = 8;  // 舵机向上
static const uint8_t servoDownPin = 9;  // 舵机向下
static const uint8_t powerBTNPin  = 10; // 电源开关按钮
static const uint8_t MosPin       = 3;  // 电源MOS管控制引脚

volatile bool isPowerBTNShortPressed = false; // 电源按钮短按标志
volatile bool isPowerBTNLongPressed  = false; // 电源按钮长按标志

static portMUX_TYPE btnMutex = portMUX_INITIALIZER_UNLOCKED;

OneButton powerBTN;

void powerBTNShortPress() {
  // 切换逻辑：只要不是running，都切换到running；如果是，则切换到paused
  systemState_e currentState = getCurrentState();
  if (currentState == SYSTEM_RUNNING) {
    stateSet(SYSTEM_PAUSED);
  } else {
    // PAUSED 或 EMPTY 都切到 RUNNING
    stateSet(SYSTEM_RUNNING);
  }
  isPowerBTNShortPressed = !isPowerBTNShortPressed;
  buzzerSound(SOUND_KEY);
}

void powerBTNLongPress() {
  isPowerBTNLongPressed = true;
  buzzerSound(SOUND_KEY);
}

void buttonInit() {
  powerBTN.setup(powerBTNPin, INPUT);
  powerBTN.attachClick(powerBTNShortPress);
  powerBTN.attachLongPressStart(powerBTNLongPress);
  powerBTN.setPressMs(800);
}

void buttonTask(void* pvParameters) {
  while (1) {
    powerBTN.tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
