//
// Created by Esuny on 2024/8/26.
//

#ifndef SUPER_CUBE_ESP_SUEPR_CUBE_H
#define SUPER_CUBE_ESP_SUEPR_CUBE_H

#include <HardwareSerial.h>
#include <memory>
#include "EEPROM.h"
#include "config/ConfigManager.h"
#include "command/CommandManager.h"
#include "handler/console_handler.h"
#include "handler/LightHandler.h"
#include <Adafruit_NeoPixel.h>
#include "service/HTTPService.h"
#include <ESP8266WebServer.h>
#include <service/WebsocketService.h>
#include <service/MqttService.h>
#include <service/AttitudeService.h>

class SerialHandler;

class CommandRegistry;

class Shell;

class HttpServer;

class MqttService;

class WebSocketService;

class LightHandler;

class AttitudeService;

class ConfigManager;

class super_cube {
public:
    super_cube(HardwareSerial *serial);

    ~super_cube();

    void setup();

    void loop();

    bool HTTPServiceDEBUG = false;
    bool MqttServiceDEBUG = false;

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

    template<typename... Args>
    void hdebugln(const Args &... args) {
        _running([this, args...]() {
            debugln(args...);
        }, HTTPServiceDEBUG);
    }

    template<typename... Args>
    void mdebugln(const Args &... args) {
        _running([this, args...]() {
            debugln(args...);
        }, MqttServiceDEBUG);
    }

    void debug() {}

    void debugln() {
        serial->println();
    }

    void DEBUG_MODE_SET(bool mode) {
        DEBUG = mode;
    }

    void HTTP_DEBUG_MODE_SET(bool mode) {
        HTTPServiceDEBUG = mode;
    }

    void Mqtt_DEBUG_MODE_SET(bool mode) {
        MqttServiceDEBUG = mode;
    }

    CommandRegistry *command_registry;
    HardwareSerial *serial;
    ConfigManager *config_manager;
    std::unique_ptr<HttpServer> httpServer;
    std::unique_ptr<WebSocketService> webSocketService;
    std::unique_ptr<MqttService> mqttService;
    std::unique_ptr<LightHandler> lightHandler;
    std::unique_ptr<AttitudeService> attitudeService;
protected:
    template<typename Func>
    void _running(Func func, bool running) {
        if (running)
            func();
    }

    void _connectWiFi(const char *ssid, const char *password);

    void _command_register() const;

private:
    bool DEBUG = false;
    SerialHandler *serialHandler;

};

#endif //SUPER_CUBE_ESP_SUEPR_CUBE_H
