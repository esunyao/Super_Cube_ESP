//
// Created by Esuny on 2024/8/26.
//
#include "super_cube.h"
#include <ESP8266WiFi.h> // Include the WiFi header
#include <utils/uuid_utils.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include <WString.h>
#include <main_.h>
#include <HardwareSerial.h>

super_cube::super_cube(HardwareSerial *serial) : command_registry(new CommandRegistry(*this)),
                                                 serial(serial),
                                                 config_manager(&ConfigManager::getInstance()),
                                                 serialHandler(new SerialHandler(this, this->serial)),
                                                 httpServer(new HttpServer(this)) {
    strip = nullptr;
}

super_cube::~super_cube() {
    // Destructor implementation
    delete command_registry, command_registry = nullptr;
    delete serialHandler, serialHandler = nullptr;
    delete config_manager, config_manager = nullptr;
    delete strip, strip = nullptr;
    delete httpServer, httpServer = nullptr;
}

void super_cube::setup() {
    config_manager->initialize();
    if (config_manager->getConfig()["DEBUG"])
        DEBUG_MODE_SET(true);
    serialHandler->start();
    debugln("[DEBUG] Loading Config Complete");
    debugln("[DEBUG] Config: ", config_manager->getConfig().as<String>());
    _connectWiFi(config_manager->getConfig()["Internet"]["ssid"], config_manager->getConfig()["Internet"]["passwd"]);
    command_registry->add_command(Command(
            flash_string_vector{"digitalRead"}, // Convert F() result to std::string
            flash_string_vector{"<pin>"}, // Convert F() result to std::string
            [](Shell *shell, const std::vector<std::string> &arguments) {
                shell->println(arguments[0].c_str());
                shell->println("啊拒绝哦i解耦i飞机破i");
            }
    ));
    command_registry->add_command(Command(
            flash_string_vector{"debug"}, // Convert F() result to std::string
            flash_string_vector{"<pin>"}, // Convert F() result to std::string
            [](Shell *shell, const std::vector<std::string> &arguments) {
                shell->getSuperCube()->config_manager->getConfig()["DEBUG"] = true;
                shell->getSuperCube()->config_manager->saveConfig();
            }
    ));
    debugln("[DEBUG] Command Registry Initialized");
    httpServer->start();
    debugln("[DEBUG] HTTP Server Started");
}

void super_cube::loop() {
//    Serial.println(reinterpret_cast<uintptr_t>(this), HEX);
    serialHandler->handleSerial();
}

void super_cube::_command_register() {

}

void super_cube::_connectWiFi(const char *ssid, const char *password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int attempts = 0;
    const int maxAttempts = 20; // Maximum number of attempts to connect to WiFi

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi. Starting AP mode...");

        // Start Access Point mode
        String uuid = generateUUIDv4();
        WiFi.softAP("SuperCube_" + uuid.substring(uuid.length() - 5), "password123");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
}