//
// Created by Esuny on 2024/8/26.
//

#ifndef SUPER_CUBE_ESP_SUEPR_CUBE_H
#define SUPER_CUBE_ESP_SUEPR_CUBE_H

#include <HardwareSerial.h>
#include "EEPROM.h"
#include "config/ConfigManager.h"
#include "command/CommandManager.h"
#include "handler/console_handler.h"
#include <Adafruit_NeoPixel.h>
#include "service/HTTPService.h"
#include <ESP8266WebServer.h>
#include <service/WebsocketService.h>
#include <service/MqttService.h>

class SerialHandler;

class CommandRegistry;

class Shell;

class HttpServer;

class MqttService;

class WebSocketService;

class ConfigManager;

class super_cube {
public:
    super_cube(HardwareSerial *serial);

    ~super_cube();

    void setup();

    void loop();

    template<typename T, typename... Args>
    void debug(const T &first, const Args &... rest) {
        _running([this, &first]() { serial->print(first); }, DEBUG);
        _running([this, &rest...]() { debug(rest...); }, DEBUG);
    }

    template<typename T, typename... Args>
    void debugln(const T &first, const Args &... rest) {
        _running([this, &first]() { serial->print(first); }, DEBUG);
        _running([this, &rest...]() { debugln(rest...); }, DEBUG);
    }

    void debug() {}

    void debugln() {
        serial->println();
    }

    void DEBUG_MODE_SET(bool mode) {
        DEBUG = mode;
    }

    CommandRegistry *command_registry;
    Adafruit_NeoPixel *strip;
    HardwareSerial *serial;
    ConfigManager *config_manager;
    HttpServer *httpServer;
    WebSocketService *webSocketService;
    MqttService *mqttService;
protected:
    template<typename Func>
    void _running(Func func, bool running) {
        if (running)
            func();
    }

    void _connectWiFi(const char *ssid, const char *password);

    void _command_register();

private:
    bool DEBUG = false;
    SerialHandler *serialHandler;

};

#endif //SUPER_CUBE_ESP_SUEPR_CUBE_H
