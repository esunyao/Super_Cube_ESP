#include <ESP8266WiFi.h>
#include <memory>
#include <ESP8266WebServer.h>
#include "main_.h"
#include <Arduino.h>
#include "super_cube.h"
#include <uuid/common.h>
#include <uuid/console.h>

#include "Wire.h"

#define LED_PIN 13
size_t TOTAL_HEAP = EspClass::getFreeHeap();
// WiFi设置
std::unique_ptr<super_cube> cube;

void setup() {
    cube = std::make_unique<super_cube>(&Serial);
    cube->setup();
}

void loop() {
    cube->loop();
}