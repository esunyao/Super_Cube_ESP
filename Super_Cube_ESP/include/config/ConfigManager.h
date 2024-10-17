//
// Created by Esuny on 2024/8/26.
//

#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <map>
#include "ArduinoJson.h"
#include <super_cube.h>

class super_cube;

class CommandNode;

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
    CommandNode *_init_stringer(std::string node);

    CommandNode *_init_boolean(std::string node);

    CommandNode *_init_inter(std::string node);

    CommandNode *_config_registry(std::string node);

private:
    int eepromSize;
    JsonDocument configDoc;
    super_cube *superCube;

    void createDefaultConfig();

    void clearConfigDoc();

    // List of required keys and their sub-keys
    const std::map<std::string, std::vector<std::string>> requiredKeys = {
            {"reset",          {}},
            {"HTTPDEBUG",      {}},
            {"MQTTDEBUG",      {}},
            {"DEBUG",          {}},
            {"ID",             {}},
            {"Internet",       {"ssid", "passwd"}},
            {"http",           {"port"}},
            {"Websocket",      {"ip",   "port"}},
            {"Mqtt",           {"ip",   "port", "username", "password", "topic", "callback_topic"}},
            {"serverMode",     {}},
            {"light",          {}},
            {"light_presets",  {}},
    };

    void registerNodeCommands(const std::string &path, JsonVariant variant, CommandNode *parentNode, JsonVariant doc);
};

#endif // EEPROM_UTILS_H