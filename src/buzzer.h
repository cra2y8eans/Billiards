#pragma once

#include <Arduino.h>

// ===================== 用户配置 =====================
#define BUZZER_MAX_NOTES     16   // 单个旋律最多音符数

// 触发极性：低电平触发模块 → 1；高电平触发模块 → 0
#define BUZZER_ACTIVE_LOW    0  // SS8050驱动

// ===================== 常用音名 (Hz) =====================
#define NOTE_C6   1047
#define NOTE_E6   1319
#define NOTE_G6   1568
#define NOTE_A6   1760
#define NOTE_C7   2093
#define NOTE_E7   2637
#define NOTE_G7   3136
#define NOTE_REST 0        // 休止符(静音)

// ===================== 数据结构 =====================
struct BuzzerNote {
  uint16_t freq;       // 频率 Hz，0 表示静音
  uint16_t duration;   // 持续时长 ms
};

enum BuzzerSound : uint8_t {
  SOUND_OK = 0,      // 成功：短促上扬
  SOUND_ERROR,       // 错误：下降长音
  SOUND_WARNING,     // 警告：两声相同
  SOUND_STARTUP,     // 开机自检
  SOUND_SHUTDOWN,    // 关机提示
  SOUND_KEY,         // 按键咔哒
  SOUND_MAX
};

// ===================== 公共接口 =====================

/** 初始化（无源蜂鸣器 + LEDC PWM） */
void buzzerInit();

/** 播放一段非预设的旋律(非阻塞)。notes 会被拷贝到内部缓冲，函数返回后可释放 */
void buzzerPlay(const BuzzerNote* notes, uint8_t count, bool loop = false);

/** 播放预定义提示音(非阻塞) */
void buzzerSound(BuzzerSound sound);

/** 立即停止播放 */
void buzzerStop();

/** 是否正在播放，以免打断当前播放的音效 */
bool buzzerIsBusy();

/** 状态机更新：放进一个 FreeRTOS 任务里循环调用，内部是 while(1)，不会返回 */
void buzzerUpdate(void* pvParameter = nullptr);