//
// Created by Esuny on 2023/10/21.
//

#ifndef SUPER_CUBE_ESP_SERVER_MAIN_H
#define SUPER_CUBE_ESP_SERVER_MAIN_H

#include <ESP8266WiFi.h>
#include <iostream>
#include <EEPROM.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <map>
#include <functional>
#include <Adafruit_NeoPixel.h>
#include "utils/Logger.h"
#include "utils/flash_write.h"
#include "service/Websocket_Service.h"

#define WIFIConnectTimeOut 50

const String Default_Config = "{\"Logger\": {\"Debug\": false}, \"WiFi\": {\"SoftAP\": {\"ssid\": \"Super_Cube\", \"passwd\": \"FRS8571a8438a712517\", \"IsOpen\": false}, \"Connect\": {\"ssid\": \"Super_Cube\", \"passwd\": \"FRS8571a8438a712517\", \"config\": false}, \"ip\": \"192.168.0.140\", \"gateway\": \"192.168.0.1\", \"subnet\": \"255.255.255.0\"}, \"LED\": {}}";
const uint8_t INIT_FLAG = uint8_t (114514);
extern int blockSize;

extern std::map<uint8_t, IPAddress> WebSocketsClientMapList;

extern Logger logger;
extern WebSocketsServer server;
extern std::map<String, int> pinMap;
extern DynamicJsonDocument Config;
extern flash_write FlashWrite;

// 将callback添加到CallbackFunction中
void addCallbackToMap();
void executeCallback(uint8_t num, const char* name, JsonDocument& msg);
#endif //SUPER_CUBE_ESP_SERVER_MAIN_H
