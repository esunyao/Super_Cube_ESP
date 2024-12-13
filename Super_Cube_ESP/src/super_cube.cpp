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
                                                 config_manager(&ConfigManager::getInstance(this)),
                                                 serialHandler(new SerialHandler(this, this->serial)),
                                                 lightHandler(new LightHandler(this)) {
    strip = nullptr;
    httpServer = nullptr;
    webSocketService = nullptr;
    mqttService = nullptr;
    attitudeService = nullptr;
}

super_cube::~super_cube() {
    // Destructor implementation
    delete command_registry, command_registry = nullptr;
    delete serialHandler, serialHandler = nullptr;
    delete config_manager, config_manager = nullptr;
    delete strip, strip = nullptr;
    delete httpServer, httpServer = nullptr;
    delete mqttService, mqttService = nullptr;
    delete attitudeService, attitudeService = nullptr;
}

void super_cube::setup() {
    config_manager->initialize();
    serialHandler->start();
    if (config_manager->getConfig()["DEBUG"])
        DEBUG_MODE_SET(true);
    if (config_manager->getConfig()["HTTPDEBUG"])
        HTTP_DEBUG_MODE_SET(true);
    if (config_manager->getConfig()["MQTTDEBUG"])
        Mqtt_DEBUG_MODE_SET(true);

    debugln("[DEBUG] Loading Config Complete");
    httpServer = new HttpServer(this, static_cast<int>(config_manager->getConfig()["http"]["port"].as<int>()));
    config_manager->command_initialize();
    _command_register();
    lightHandler->lightInitiation();
    webSocketService = new WebSocketService(this,
                                            static_cast<String>(config_manager->getConfig()["Websocket"]["ip"].as<String>()),
                                            static_cast<int>(config_manager->getConfig()["Websocket"]["port"].as<int>()));
    mqttService = new MqttService(this,
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["ip"].as<String>()),
                                  static_cast<int>(config_manager->getConfig()["Mqtt"]["port"].as<int>()),
                                  static_cast<String>(config_manager->getConfig()["ID"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["username"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["password"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["topic"].as<String>()),
                                  static_cast<String>(config_manager->getConfig()["Mqtt"]["callback_topic"].as<String>()));
    debugln("[DEBUG] Config: ", config_manager->getConfig().as<String>());
    _connectWiFi(config_manager->getConfig()["Internet"]["ssid"], config_manager->getConfig()["Internet"]["passwd"]);
    debugln("[DEBUG] Starting HTTP Server...");
    httpServer->start();
    command_registry->register_command(std::unique_ptr<CommandNode>(command_registry->Literal("asdf")->then(
            command_registry->Literal("f")->then(
                    command_registry->IntegerParam("value")->runs([](Shell *shell, const R &context) {
                        shell->println(std::to_string(context.get<int>("value")).c_str());
                    })))));
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
        mqttService->Connect_();
        debugln("[DEBUG] Mqtt Server Started, Listening...");
    }
    if (config_manager->getConfig()["Attitude"]["enable"]) {
        attitudeService = new AttitudeService(this);
        attitudeService->setup();
    }
}

void super_cube::loop() {
//    Serial.println(reinterpret_cast<uintptr_t>(this), HEX);
    serialHandler->handleSerial();
    httpServer->handleClient();
    if (config_manager->getConfig()["serverMode"] == "Websocket" && webSocketService->webSocket != nullptr) {
        webSocketService->webSocket->loop();
    }
    if (config_manager->getConfig()["serverMode"] == "Mqtt" && mqttService->mqttClient != nullptr) {
        mqttService->loop();
    }
    if (config_manager->getConfig()["Attitude"]["enable"]) {
        attitudeService->update();
    }
}

void super_cube::_command_register() {
    httpServer->commandRegister();
    command_registry->register_command(
            std::unique_ptr<CommandNode>(command_registry->Literal("restart")->runs([](Shell *shell, const R &context) {
                EspClass::restart();
            })));
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("commandtree")->runs([](Shell *shell, const R &context) {
                        shell->getSuperCube()->command_registry->printCommandTree();
                    })));
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