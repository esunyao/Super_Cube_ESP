//
// Created by Esuny on 2024/8/26.
//
#include "super_cube.h"
#include <ESP8266WiFi.h> // Include the WiFi header
#include <utils/uuid_utils.h>
#include <Arduino.h>
#include <vector>
#include <regex>
#include <string>
#include <WString.h>
#include <main_.h>
#include <HardwareSerial.h>

extern size_t TOTAL_HEAP;

super_cube::super_cube(HardwareSerial *serial) : command_registry(new CommandRegistry()),
                                                 serial(serial),
                                                 config_manager(&ConfigManager::getInstance(this)),
                                                 lightHandler(std::make_unique<LightHandler>(this)),
                                                 serialHandler(new SerialHandler(this, this->serial)) {

}

super_cube::~super_cube() {
    // Destructor implementation
    delete command_registry, command_registry = nullptr;
    delete serialHandler, serialHandler = nullptr;
    delete config_manager, config_manager = nullptr;
    httpServer.reset();
    webSocketService.reset();
    mqttService.reset();
    lightHandler.reset();
    attitudeService.reset();
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
    config_manager->command_initialize();
    _command_register();
    lightHandler->lightInitiation();
    lightHandler.reset();
    debugln("[DEBUG] Config: ", config_manager->toString().c_str());
    _connectWiFi(config_manager->getConfig()["Internet"]["ssid"], config_manager->getConfig()["Internet"]["passwd"]);
    command_registry->register_command(std::unique_ptr<CommandNode>(command_registry->Literal("asdf")->then(
            command_registry->Literal("f")->then(
                    command_registry->IntegerParam("value")->runs([](std::unique_ptr<Shell> shell, const R &context) {
                        shell->println(std::to_string(context.get<int>("value")).c_str());
                    })))));
    if (config_manager->getConfig()["Attitude"]["enable"]) {
        attitudeService = std::make_unique<AttitudeService>(this);
        attitudeService->setup();
    }
    if (config_manager->getConfig()["serverMode"] == "Http") {
        debugln("\n[DEBUG] Mode has been select as Http");
        debugln("[DEBUG] Starting HTTP Server...");
        httpServer = std::make_unique<HttpServer>(this,
                                                  static_cast<int>(config_manager->getConfig()["http"]["port"].as<int>()));
        httpServer->start();
        debugln("[DEBUG] HTTP Server Started, Listening...");
    }
    if (config_manager->getConfig()["serverMode"] == "Websocket") {
        webSocketService = std::make_unique<WebSocketService>(this,
                                                              static_cast<String>(config_manager->getConfig()["Websocket"]["ip"].as<String>()),
                                                              static_cast<int>(config_manager->getConfig()["Websocket"]["port"].as<int>()));
        debugln("\n[DEBUG] Mode has been select as Websocket");
        debugln("[DEBUG] Starting Websocket Server...");
        webSocketService->start();
        debugln("[DEBUG] Websocket Server Started, Listening...");
    }
    if (config_manager->getConfig()["serverMode"] == "Mqtt") {
        mqttService = std::make_unique<MqttService>(this,
                                                    static_cast<String>(config_manager->getConfig()["Mqtt"]["ip"].as<String>()),
                                                    static_cast<int>(config_manager->getConfig()["Mqtt"]["port"].as<int>()),
                                                    static_cast<String>(config_manager->getConfig()["ID"].as<String>()),
                                                    static_cast<String>(config_manager->getConfig()["Mqtt"]["username"].as<String>()),
                                                    static_cast<String>(config_manager->getConfig()["Mqtt"]["password"].as<String>()),
                                                    static_cast<String>(config_manager->getConfig()["Mqtt"]["topic"].as<String>()),
                                                    static_cast<String>(config_manager->getConfig()["Mqtt"]["callback_topic"].as<String>()));
        debugln("\n[DEBUG] Mode has been select as Mqtt");
        debugln("[DEBUG] Starting Mqtt Client...");
        mqttService->start();
        mqttService->Connect_();
        debugln("[DEBUG] Mqtt Server Started, Listening...");
    }
}

void super_cube::loop() {
//    Serial.println(reinterpret_cast<uintptr_t>(this), HEX);
    serialHandler->handleSerial();
    if (config_manager->getConfig()["serverMode"] == "Http" && webSocketService->webSocket != nullptr) {
        httpServer->handleClient();
    }
    if (config_manager->getConfig()["serverMode"] == "Websocket" && webSocketService->webSocket != nullptr) {
        webSocketService->webSocket->loop();
    }
    if (config_manager->getConfig()["serverMode"] == "Mqtt" && mqttService->mqttClient != nullptr) {
        mqttService->loop();
    }
}

void super_cube::_command_register() const {
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("memory")->runs([](std::unique_ptr<Shell> shell, const R &context) {
                        size_t freeHeap = EspClass::getFreeHeap();
                        size_t usedHeap = TOTAL_HEAP - freeHeap;
                        {
                            float overallUsagePercent = (usedHeap * 100.0f) / TOTAL_HEAP;

                            int barWidth = 20;
                            int usedBarChars = static_cast<int>(barWidth * overallUsagePercent / 100.0f);
                            char bar[32], usedPart[32], freePart[32];
                            snprintf(usedPart, sizeof(usedPart), "%.*s", usedBarChars, "####################");
                            snprintf(freePart, sizeof(freePart), "%*s", barWidth - usedBarChars, "");
                            snprintf(bar, sizeof(bar), "[%s%s]", usedPart, freePart);
                            static char message[128];
                            snprintf(message, sizeof(message), "内存占用: %s %d%% (%u/%u 字节)",
                                     bar, (int) overallUsagePercent, usedHeap, TOTAL_HEAP);
                            shell->println(message);
                        }
                        std::vector<std::pair<String, size_t>> modules;
                        super_cube *sc = shell->getSuperCube();
                        if (sc->serialHandler != nullptr)
                            modules.emplace_back("serialHandler", sizeof(*(sc->serialHandler)));
                        if (sc->command_registry != nullptr)
                            modules.emplace_back("command_registry", sizeof(*(sc->command_registry)));
                        if (sc->config_manager != nullptr)
                            modules.emplace_back("config_manager", sizeof(*(sc->config_manager)));
                        if (sc->httpServer != nullptr)
                            modules.emplace_back("httpServer", sizeof(*(sc->httpServer)));
                        if (sc->webSocketService != nullptr)
                            modules.emplace_back("webSocketService", sizeof(*(sc->webSocketService)));
                        if (sc->mqttService != nullptr)
                            modules.emplace_back("mqttService", sizeof(*(sc->mqttService)));
                        if (sc->lightHandler != nullptr)
                            modules.emplace_back("lightHandler", sizeof(*(sc->lightHandler)));
                        if (sc->attitudeService != nullptr)
                            modules.emplace_back("attitudeService", sizeof(*(sc->attitudeService)));
                        std::sort(modules.begin(), modules.end(), [](const auto &a, const auto &b) {
                            return a.second > b.second;
                        });

                        shell->println("模块内存占用情况：");

                        for (const auto &mod: modules) {
                            float modPercent = (mod.second * 100.0f) / usedHeap;
                            static char modMessage[64];
                            snprintf(modMessage, sizeof(modMessage), "%s: %u 字节 (%.2f%%)", mod.first.c_str(),
                                     mod.second, modPercent);
                            shell->println(modMessage);
                        }
                    })));
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("restart")->runs([](std::unique_ptr<Shell> shell, const R &context) {
                        EspClass::restart();
                    })));
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("commandtree")->runs([](std::unique_ptr<Shell> shell, const R &context) {
                        shell->getSuperCube()->command_registry->printCommandTree();
                    })));
    command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    command_registry->Literal("Server_NeoPixel")->runs(
                            [](std::unique_ptr<Shell> shelll, const R &context) {
                                // {"command": "Server_NeoPixel", "pin": 1, "r": 255, "g": 255, "b": 255, "bright": 255, "num": ["0-3"]}
                                // {"command": "Server_NeoPixel", "presets": ""}
                                std::map<int, const uint8_t> pinMap = {
                                        {1, D1},
                                        {2, D2},
                                        {3, D3},
                                        {4, D4},
                                        {5, D5},
                                        {6, D6}
                                };
                                if (shelll->jsonDoc.operator[]("save").is<bool>())
                                    if (shelll->jsonDoc.operator[]("save").as<bool>()) {
                                        shelll->jsonDoc.remove("save");
                                        auto copiedDoc = std::make_unique<JsonDocument>();
                                        copiedDoc->set(shelll->jsonDoc);
                                        shelll->getSuperCube()->config_manager->getConfig()["light"].as<JsonArray>().add(
                                                *copiedDoc);
                                        shelll->getSuperCube()->config_manager->saveConfig();
                                    }
                                if (shelll->jsonDoc.operator[]("presets").is<std::string>()) {
                                    ConfigValue presets = shelll->getSuperCube()->config_manager->getConfig()["light_presets"][shelll->jsonDoc.operator[](
                                            "presets").as<std::string>()];
                                    shelll->jsonDoc.operator[]("r") = presets["r"].as<int>();
                                    shelll->jsonDoc.operator[]("g") = presets["g"].as<int>();
                                    shelll->jsonDoc.operator[]("b") = presets["b"].as<int>();
                                    shelll->jsonDoc.operator[]("bright") = presets["bright"].as<int>();
                                }
                                std::unique_ptr<Adafruit_NeoPixel> stripasd = std::make_unique<Adafruit_NeoPixel>(25,
                                                                                                                  pinMap[shelll->jsonDoc.operator[](
                                                                                                                          "pin").as<int>()],
                                                                                                                  NEO_GRB +
                                                                                                                  NEO_KHZ800);
                                stripasd->begin();
                                for (JsonVariant v: shelll->jsonDoc.operator[]("num").as<JsonArray>()) {
                                    if (v.as<int>())
                                        stripasd->setPixelColor(v.as<int>(),
                                                                stripasd->Color(
                                                                        shelll->jsonDoc.operator[]("r").as<int>() | 0,
                                                                        shelll->jsonDoc.operator[]("g").as<int>() | 0,
                                                                        shelll->jsonDoc.operator[]("b").as<int>() |
                                                                        0));
                                    else {
                                        std::regex pattern(R"((\d+)-(\d+))");
                                        std::smatch matches;
                                        std::string str = v.as<String>().c_str();
                                        if (std::regex_search(str, matches, pattern))
                                            for (int i = std::stoi(matches[1].str());
                                                 i <= std::stoi(matches[2].str()); i++)
                                                stripasd->setPixelColor(i, stripasd->Color(
                                                        shelll->jsonDoc.operator[]("r").as<int>() | 0,
                                                        shelll->jsonDoc.operator[]("g").as<int>() | 0,
                                                        shelll->jsonDoc.operator[]("b").as<int>() | 0));
                                    }
                                }
                                stripasd->setBrightness(shelll->jsonDoc.operator[]("bright"));
                                stripasd->show();
                                shelll->println("Lighting up");
                                stripasd.reset();
                            })));
    command_registry->register_command(
            std::unique_ptr<CommandNode>(command_registry->Literal("Server_posture")->then(
                            command_registry->Literal("get")->runs(
                                    [this](std::unique_ptr<Shell> shelll, const R &context) {
                                        bool out_put = false;
                                        if (shelll->jsonDoc["out_put"].is<bool>())
                                            out_put = shelll->jsonDoc["out_put"];
                                        JsonDocument data = shelll->getSuperCube()->attitudeService->GetData(out_put,
                                                                                                             shelll->jsonDoc["mode"].as<String>());
                                        String dataStr;
                                        serializeJson(data, dataStr);
                                        if (shelll->getSuperCube())
                                            shelll->getSuperCube()->mqttService->publishMessage(dataStr,
                                                                                                shelll->getSuperCube()->config_manager->getConfig()["Mqtt"]["attitude_topic"].as<String>() +
                                                                                                shelll->getSuperCube()->config_manager->getConfig()["ID"].as<String>());
                                        else
                                            shelll->println("only Support Mqtt");
                                    }))
                                                 ->then(command_registry->Literal("getDevStatus")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getDevStatus());
                                                         }))
                                                 ->then(command_registry->Literal("getReadyStatus")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getReadyStatus());
                                                         }))
                                                 ->then(command_registry->Literal("ConnectionTest")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->ConnectionTest());
                                                         }))
                                                 ->then(command_registry->Literal("StartDmp")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->StartDmp());
                                                         }))
            ));
}

void super_cube::_connectWiFi(const char *ssid, const char *password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int attempts = 0;
    const int maxAttempts = 200; // Maximum number of attempts to connect to WiFi

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(50);
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