#pragma once

#include <Arduino.h>

enum systemState_e {
  SYSTEM_RUNNING, // 系统运行
  SYSTEM_PAUSED,  // 系统暂停
};

extern systemState_e systemState;

void          stateToggle();
systemState_e getCurrentState();
void          stateSet(systemState_e newState);