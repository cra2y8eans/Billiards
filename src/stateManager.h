#pragma once

#include <Arduino.h>

enum systemState_e {
  SYSTEM_RUNNING, // 系统运行
  SYSTEM_PAUSED,  // 系统暂停
  SYSTEM_EMPTY,   // 系统空仓
};

extern systemState_e systemState;

systemState_e getCurrentState();
void          stateSet(systemState_e newState);