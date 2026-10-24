#include "laser.h"
#include <VL53L0X.h>
#include <Wire.h>
#include <esp_log.h>

VL53L0X laser;

const char*          TAG                  = "LASER";
static const uint8_t laser_i2c_scl_pin    = 4;     // SCL引脚
static const uint8_t laser_i2c_sda_pin    = 5;     // SDA引脚
volatile bool        laserTimeoutOccurred = false; // 激光测距超时标志

void laserInit() {
  Wire.begin(laser_i2c_sda_pin, laser_i2c_scl_pin);
  laser.setTimeout(500);
  if (!laser.init()) {
    ESP_LOGE("LASER", "Failed to detect and initialize sensor!");
    while (1) { }
  }
  // 增加测量时间，获得更好的测量精度
  laser.setMeasurementTimingBudget(200000);
}

int16_t getLaserDistance() {
  uint16_t distance = laser.readRangeSingleMillimeters();
  if (laser.timeoutOccurred()) { // 检查是否发生超时
    laserTimeoutOccurred = true;
    ESP_LOGW(TAG, "Laser timeout occurred!");
    return -1; // 返回-1表示测量失败
  }
  laserTimeoutOccurred = false;
  ESP_LOGI("LASER", "距离: %d mm", distance);
  return distance; // 返回测量的距离值（单位：毫米）
}
