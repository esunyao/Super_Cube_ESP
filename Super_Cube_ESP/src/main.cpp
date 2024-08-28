#include <ESP8266WiFi.h>
#include <memory>
#include <ESP8266WebServer.h>
#include "main_.h"
#include <Arduino.h>
#include "super_cube.h"
#include <uuid/common.h>
#include <uuid/console.h>
const char *_ssid = "您的WiFi名称";
const char *_password = "您的WiFi密码";
// WiFi设置
std::unique_ptr<super_cube> cube;
void setup() {
    cube = std::make_unique<super_cube>(&Serial);
    cube->setup();
}

void loop() {
    cube->loop();
}