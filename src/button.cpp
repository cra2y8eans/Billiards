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

volatile bool PowerOnFlag             = false; // 电源开关标志
volatile bool isServoUpShortPressed   = false; // 舵机向上短按标志
volatile bool isServoUpLongPressed    = false; // 舵机向上长按标志
volatile bool isServoDownShortPressed = false; // 舵机向下短按标志
volatile bool isServoDownLongPressed  = false; // 舵机向下长按标志
volatile bool isPowerBTNShortPressed  = false; // 电源按钮短按标志
volatile bool isPowerBTNLongPressed   = false; // 电源按钮长按标志

static portMUX_TYPE btnMutex = portMUX_INITIALIZER_UNLOCKED;

OneButton servoUpBTN, servoDownBTN, powerBTN;

void servoUpShortPress() {
  isServoUpShortPressed = !isServoUpShortPressed;
  buzzerSound(SOUND_KEY);
}
void servoUpLongPress() {
  isServoUpLongPressed = !isServoUpLongPressed;
  buzzerSound(SOUND_KEY);
}

void servoDownShortPress() {
  isServoDownShortPressed = !isServoDownShortPressed;
  buzzerSound(SOUND_KEY);
}
void servoDownLongPress() {
  isServoDownLongPressed = !isServoDownLongPressed;
  buzzerSound(SOUND_KEY);
}

void powerBTNShortPress() {
  stateToggle();
  isPowerBTNShortPressed = !isPowerBTNShortPressed;
  buzzerSound(SOUND_KEY);
}
void powerBTNLongPress() {
  isPowerBTNLongPressed = true;
  buzzerSound(SOUND_KEY);
}

void buttonInit() {
  servoUpBTN.setup(servoUpPin, INPUT);
  servoUpBTN.attachClick(servoUpShortPress);
  servoUpBTN.attachLongPressStart(servoUpLongPress);
  servoUpBTN.setPressMs(800);

  servoDownBTN.setup(servoDownPin, INPUT);
  servoDownBTN.attachClick(servoDownShortPress);
  servoDownBTN.attachLongPressStart(servoDownLongPress);
  servoDownBTN.setPressMs(800);

  powerBTN.setup(powerBTNPin, INPUT);
  powerBTN.attachClick(powerBTNShortPress);
  powerBTN.attachLongPressStart(powerBTNLongPress);
  powerBTN.setPressMs(800);
}

void buttonTask(void* pvParameters) {
  while (1) {
    servoUpBTN.tick();
    servoDownBTN.tick();
    powerBTN.tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
