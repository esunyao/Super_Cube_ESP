//
// Created by Esuny on 2024/8/26.
//

#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <map>
#include "ArduinoJson.h"
#include <memory>
#include <super_cube.h>

class super_cube;

class Shell;

class R;

class CommandNode;

template<typename T>
struct TypeName {
    static std::string get() { return "unknown"; }
};

template<>
struct TypeName<int> {
    static std::string get() { return "Int"; }
};

template<>
struct TypeName<std::string> {
    static std::string get() { return "String"; }
};

template<>
struct TypeName<bool> {
    static std::string get() { return "Bool"; }
};

class ConfigManager {
public:

    static ConfigManager &getInstance(super_cube *superCube) {
        static ConfigManager instance(superCube); // 保证只会创建一次，并在第一次使用时初始化
        return instance;
    }

    ConfigManager(super_cube *superCube);

    void initialize();

    void clear();

    void saveConfig();

    bool readConfig();

    bool validateConfig();

    JsonDocument &getConfig();

    void command_initialize();

protected:
    CommandNode *_init_stringer(std::string node, JsonVariant doc);

    CommandNode *_init_boolean(std::string node, JsonVariant doc);

    CommandNode *_init_inter(std::string node, JsonVariant doc);

    void _init_get(std::unique_ptr<Shell> shell, const R &context, JsonVariant doc);

    template<typename T>
    CommandNode *_init_generic(std::string node, JsonVariant doc, std::function<void(JsonVariant, T)> setter);

private:
    int eepromSize;
    JsonDocument configDoc;
    super_cube *superCube;

    void createDefaultConfig();

    void clearConfigDoc();

    std::unique_ptr<std::map<std::string, std::vector<std::string>>> requiredKeys =
            std::make_unique<std::map<std::string, std::vector<std::string>>>(
                    std::map<std::string, std::vector<std::string>>{
                            {"reset",         {}},
                            {"HTTPDEBUG",     {}},
                            {"MQTTDEBUG",     {}},
                            {"DEBUG",         {}},
                            {"ID",            {}},
                            {"Internet",      {"ssid",   "passwd"}},
                            {"http",          {"port"}},
                            {"Websocket",     {"ip",     "port"}},
                            {"Mqtt",          {"ip",     "port", "username", "password", "topic", "callback_topic", "attitude_topic", "autoReconnected"}},
                            {"Attitude",      {"enable", "SCL",  "SDA",      "TX",       "RX",    "MODE"}},
                            {"serverMode",    {}},
                            {"light",         {}},
                            {"light_presets", {}},
                    });

    void registerNodeCommands(const std::string &path, JsonVariant variant, CommandNode *parentNode, JsonVariant doc);
};

#endif // EEPROM_UTILS_H