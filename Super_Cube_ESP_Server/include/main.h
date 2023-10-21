//
// Created by Esuny on 2023/10/21.
//

#ifndef SUPER_CUBE_ESP_SERVER_MAIN_H
#define SUPER_CUBE_ESP_SERVER_MAIN_H

#include <ESP8266WiFi.h>
#include <iostream>
#include <EEPROM.h>
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <map>
#include <functional>
#include <Adafruit_NeoPixel.h>
#include "utils/Logger.h"
#include "utils/flash_write.h"
#include "service/Websocket_Service.h"

#define WIFIConnectTimeOut 100

const String Default_Config = "{\"Logger\": {\"Debug\": false}, \"WiFi\": {\"softAP\": {\"ssid\": \"Super_Cube\", \"passwd\": \"FRS8571a8438a712517\", \"ip\": \"192.168.0.140\", \"gateway\": \"192.168.0.140\", \"subnet\": \"255.255.255.0\"}, \"Connect\": {\"ssid\": \"\", \"passwd\": \"\", \"config\": false}, \"ip\": \"192.168.0.140\", \"gateway\": \"192.168.0.1\", \"subnet\": \"255.255.255.0\"}, \"LED\": {}}";
const int INIT_FLAG = 1061109;
extern int blockSize;

extern std::map<uint8_t, IPAddress> WebSocketsClient;

extern Logger logger;
extern WebSocketsServer server;
extern std::map<String, int> pinMap;
extern DynamicJsonDocument Config;
extern flash_write FlashWrite;

#endif //SUPER_CUBE_ESP_SERVER_MAIN_H
