#include <ESP8266WiFi.h>
#include <memory>
#include <ESP8266WebServer.h>
#include "main_.h"
#include <Arduino.h>
#include "super_cube.h"
#include <uuid/common.h>
#include <uuid/console.h>

#include "Wire.h"
#include "utils/I2Cdev.h"
#include "utils/MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

#define LED_PIN 13
bool blinkState = false;
// WiFi设置
std::unique_ptr<super_cube> cube;
void setup() {
    cube = std::make_unique<super_cube>(&Serial);
    cube->setup();
}

void loop() {
    cube->loop();
}