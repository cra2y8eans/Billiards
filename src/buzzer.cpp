#include "buzzer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

// ===================== LEDC API 兼容 (core 2.x / 3.x) =====================
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
#define BUZZER_LEDC_NEW_API 1
#else
#define BUZZER_LEDC_NEW_API 0
#endif

static const uint8_t LEDC_BITS = 10; // 占空比 10 bit (0~1023)
#if !BUZZER_LEDC_NEW_API
static const uint8_t LEDC_CHANNEL = 0; // 2.x 需要手动指定通道
#endif

// ===================== 内部状态 =====================
static const uint8_t buzzerPin = 7;

static BuzzerNote        noteBuffer[BUZZER_MAX_NOTES];
static volatile uint8_t  noteCount     = 0;
static volatile uint8_t  noteIndex     = 0;
static volatile bool     loopPlay      = false;
static volatile uint32_t noteStartTime = 0;
static volatile bool     buzzerBusy    = false;

static portMUX_TYPE buzzerMux = portMUX_INITIALIZER_UNLOCKED;

// ===================== 预定义提示音（duration 已调长） =====================
// 成功：C7 → E7 短促上扬（总 460ms）
static const BuzzerNote SND_OK[] = {
  { NOTE_C7, 150 }, { NOTE_REST, 60 }, { NOTE_E7, 250 }
};

// 错误：E6 → C6 下降长音（总 1000ms）
static const BuzzerNote SND_ERROR[] = {
  { NOTE_E6, 300 }, { NOTE_REST, 100 }, { NOTE_C6, 600 }
};

// 警告：两声相同（总 550ms）
static const BuzzerNote SND_WARNING[] = {
  { NOTE_A6, 200 }, { NOTE_REST, 150 }, { NOTE_A6, 200 }
};

// 开机：上行琶音（总 960ms）
static const BuzzerNote SND_STARTUP[] = {
  { NOTE_C6, 150 }, { NOTE_E6, 150 }, { NOTE_G6, 150 },
  { NOTE_C7, 150 }, { NOTE_REST, 60 }, { NOTE_E7, 300 }
};

// 关机：下行琶音（总 960ms）
static const BuzzerNote SND_SHUTDOWN[] = {
  { NOTE_E7, 150 }, { NOTE_C7, 150 }, { NOTE_G6, 150 },
  { NOTE_C6, 150 }, { NOTE_REST, 60 }, { NOTE_E6, 300 }
};

// 按键：极短"咔"（30ms，保持短）
static const BuzzerNote SND_KEY[] = {
  { NOTE_G7, 30 }
};

// ===================== 底层 PWM =====================
static inline uint32_t dutyMax() {
  return (1u << LEDC_BITS) - 1;
}

// 静音：按极性输出"关断"电平
static inline void buzzerSilence() {
#if BUZZER_ACTIVE_LOW
  uint32_t d = dutyMax(); // 低电平触发 → 引脚 HIGH 关断
#else
  uint32_t d = 0; // 高电平触发 → 引脚 LOW  关断
#endif
#if BUZZER_LEDC_NEW_API
  ledcWrite(buzzerPin, d);
#else
  ledcWrite(LEDC_CHANNEL, d);
#endif
}

// 播放指定频率（freq=0 表示静音）；ledcWriteTone 内部固定 50% 占空比
static inline void buzzerTone(uint16_t freq) {
  if (freq == 0) {
    buzzerSilence();
    return;
  }
#if BUZZER_LEDC_NEW_API
  ledcWriteTone(buzzerPin, freq);
#else
  ledcWriteTone(LEDC_CHANNEL, freq);
#endif
}

// ===================== 接口实现 =====================
void buzzerInit() {

#if BUZZER_LEDC_NEW_API
  ledcAttach(buzzerPin, 2000, LEDC_BITS);
#else
  ledcSetup(LEDC_CHANNEL, 2000, LEDC_BITS);
  ledcAttachPin(buzzerPin, LEDC_CHANNEL);
#endif

  buzzerSilence();

  buzzerBusy = false;
  noteCount  = 0;
  noteIndex  = 0;
  loopPlay   = false;
}

void buzzerPlay(const BuzzerNote* notes, uint8_t count, bool loop) {
  if (!notes || count == 0) return;
  if (count > BUZZER_MAX_NOTES) count = BUZZER_MAX_NOTES;

  uint16_t firstFreq = 0;

  portENTER_CRITICAL(&buzzerMux);
  memcpy(noteBuffer, notes, sizeof(BuzzerNote) * count);
  noteCount     = count;
  noteIndex     = 0;
  loopPlay      = loop;
  noteStartTime = millis();
  buzzerBusy    = true;
  firstFreq     = noteBuffer[0].freq;
  portEXIT_CRITICAL(&buzzerMux);

  buzzerTone(firstFreq);
}

void buzzerSound(BuzzerSound sound) {
  switch (sound) {
  case SOUND_OK:
    buzzerPlay(SND_OK, sizeof(SND_OK) / sizeof(BuzzerNote));
    break;
  case SOUND_ERROR:
    buzzerPlay(SND_ERROR, sizeof(SND_ERROR) / sizeof(BuzzerNote));
    break;
  case SOUND_WARNING:
    buzzerPlay(SND_WARNING, sizeof(SND_WARNING) / sizeof(BuzzerNote));
    break;
  case SOUND_STARTUP:
    buzzerPlay(SND_STARTUP, sizeof(SND_STARTUP) / sizeof(BuzzerNote));
    break;
  case SOUND_SHUTDOWN:
    buzzerPlay(SND_SHUTDOWN, sizeof(SND_SHUTDOWN) / sizeof(BuzzerNote));
    break;
  case SOUND_KEY:
    buzzerPlay(SND_KEY, sizeof(SND_KEY) / sizeof(BuzzerNote));
    break;
  default:
    break;
  }
}

void buzzerStop() {
  portENTER_CRITICAL(&buzzerMux);
  buzzerBusy = false;
  noteCount  = 0;
  noteIndex  = 0;
  loopPlay   = false;
  portEXIT_CRITICAL(&buzzerMux);
  buzzerSilence();
}

bool buzzerIsBusy() {
  return buzzerBusy;
}

void buzzerUpdate(void* pvParameter) {
  (void)pvParameter;

  while (1) {
    if (!buzzerBusy) {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    bool     changed  = false;
    uint16_t nextFreq = 0;
    bool     finished = false;

    portENTER_CRITICAL(&buzzerMux);

    if (!buzzerBusy) {
      portEXIT_CRITICAL(&buzzerMux);
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    uint32_t now = millis();
    if (now - noteStartTime >= noteBuffer[noteIndex].duration) {
      noteIndex++;
      if (noteIndex >= noteCount) {
        if (loopPlay) {
          noteIndex = 0;
        } else {
          buzzerBusy = false;
          noteCount  = 0;
          noteIndex  = 0;
          finished   = true;
        }
      }
      if (!finished) {
        nextFreq      = noteBuffer[noteIndex].freq;
        noteStartTime = now;
        changed       = true;
      }
    }
    portEXIT_CRITICAL(&buzzerMux);

    if (finished) {
      buzzerSilence();
      continue;
    }
    if (changed) {
      buzzerTone(nextFreq);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}