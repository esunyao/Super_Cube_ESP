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

super_cube::super_cube(HardwareSerial *serial) : command_registry(new CommandRegistry()),
                                                 serial(serial),
                                                 config_manager(&ConfigManager::getInstance()),
                                                 serialHandler(new SerialHandler(this, this->serial)) {
    strip = nullptr;
    httpServer = nullptr;
    webSocketService = nullptr;
    mqttService = nullptr;
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
    httpServer = new HttpServer(this, static_cast<int>(config_manager->getConfig()["http"]["port"].as<int>()));
    webSocketService = new WebSocketService(this,
                                            static_cast<String>(config_manager->getConfig()["Websocket"]["ip"].as<String>()),
                                            static_cast<int>(config_manager->getConfig()["Websocket"]["port"].as<int>()));
    mqttService = new MqttService(this,
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["ip"].as<String>()),
                                  static_cast<int>(config_manager->getConfig()["Mqtt"]["port"].as<int>()),
                                  static_cast<String>(config_manager->getConfig()["ID"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["username"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["password"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["topic"].as<String>()));
    debugln("[DEBUG] Config: ", config_manager->getConfig().as<String>());
    _connectWiFi(config_manager->getConfig()["Internet"]["ssid"], config_manager->getConfig()["Internet"]["passwd"]);
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("config")
                            ->then(command_registry->Literal("get")
                                           ->runs([this](Shell *shell, const CommandNode::R &context) {
                                               shell->getSuperCube()->serial->println(
                                                       shell->getSuperCube()->config_manager->getConfig().as<String>());
                                           })
                            )->then(command_registry->StringParam("fff")
                                            ->runs([this](Shell *shell, const CommandNode::R &context) {
                                                shell->getSuperCube()->serial->println(context.get<int>("fff"));
                                            })
                            )
            )
    );


    debugln("[DEBUG] Starting HTTP Server...");
    httpServer->start();
    debugln("[DEBUG] HTTP Server Started, Listening...");
    if (config_manager->getConfig()["serverMode"] == "Websocket") {
        debugln("\n[DEBUG] Mode has been select as Websocket");
        debugln("[DEBUG] Starting Websocket Server...");
        webSocketService->start();
        debugln("[DEBUG] Websocket Server Started, Listening...");
    }
    if (config_manager->getConfig()["serverMode"] == "Mqtt") {
        debugln("\n[DEBUG] Mode has been select as Mqtt");
        debugln("[DEBUG] Starting Mqtt Client...");
        mqttService->start();
        debugln("[DEBUG] Mqtt Server Started, Listening...");
    }
}

void super_cube::loop() {
//    Serial.println(reinterpret_cast<uintptr_t>(this), HEX);
    serialHandler->handleSerial();
    httpServer->handleClient();
    if (config_manager->getConfig()["serverMode"] == "Websocket" && webSocketService->webSocket != nullptr) {
        webSocketService->webSocket->loop();
    }
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