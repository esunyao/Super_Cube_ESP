//
// Created by Esuny on 2024/8/26.
//
#include "config/ConfigManager.h"
#include "main_.h"
#include <EEPROM.h>
#include <algorithm>
#include <utility>
#include "ArduinoJson.h"
#include "utils/uuid_utils.h"

// Constructor to initialize EEPROM
ConfigManager::ConfigManager(super_cube *superCube) : superCube(superCube) {
    EEPROM.begin(EEPROM_SIZE);
    this->eepromSize = EEPROM_SIZE;
}

// Initialize EEPROM and load config
void ConfigManager::initialize() {
    // Read config from EEPROM
    if (!readConfig() || !validateConfig()) {
        // Handle invalid or missing config
        clear();
        clearConfigDoc();
        // Optionally set default config here if needed
        createDefaultConfig();  // Create the default config
        saveConfig();  // Save it to EEPROM
    }
    if (configDoc["reset"] == true) {
        clear();
        clearConfigDoc();
        createDefaultConfig();  // Create the default config
        saveConfig();  // Save it to EEPROM
    }
    requiredKeys.reset();
}

// Clear the EEPROM
void ConfigManager::clear() {
    for (unsigned int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

// Save the config from the JsonDocument to EEPROM
void ConfigManager::saveConfig() {
    String json;
    serializeJson(configDoc, json);
    int len = json.length();
    EEPROM.write(0, (uint8_t) (len >> 8));  // 写入高字节
    EEPROM.write(1, (uint8_t) (len & 0xFF));  // 写入低字节
    for (int i = 0; i < len; i++) {
        EEPROM.write(4 + i, json[i]);  // Write the JSON data itself
    }
    EEPROM.commit();
}

// Read the config from EEPROM into the JsonDocument
bool ConfigManager::readConfig() {
    int len = (EEPROM.read(0) << 8) | EEPROM.read(1);  // 组合高字节和低字节
    clearConfigDoc();
    if (len > 0 && len < eepromSize) {
        String json;
        for (int i = 0; i < len; i++) {
            json += (char) EEPROM.read(4 + i);  // Read the JSON string
        }

        DeserializationError error = deserializeJson(configDoc, json);
        return !error;  // Return true if deserialization was successful
    }
    return false;  // Return false if there was an issue reading or deserializing
}

void ConfigManager::createDefaultConfig() {
    String uuid = generateUUIDv4();
    configDoc["reset"] = false;
    configDoc["DEBUG"] = false;
    configDoc["HTTPDEBUG"] = false;
    configDoc["MQTTDEBUG"] = false;
    configDoc["ID"] = uuid.substring(uuid.length() - 5);
    configDoc["Internet"]["ssid"] = "inhand";
    configDoc["Internet"]["passwd"] = "33336666";
    configDoc["http"]["port"] = 80;
    configDoc["Websocket"]["ip"] = "";
    configDoc["Websocket"]["port"] = 80;
    configDoc["Mqtt"]["ip"] = "192.168.2.10";
    configDoc["Mqtt"]["port"] = 1883;
    configDoc["Mqtt"]["callback_topic"] = "superCube/callback";
    configDoc["Mqtt"]["attitude_topic"] = "superCube/attitude/";
    configDoc["Mqtt"]["username"] = "SuperCube";
    configDoc["Mqtt"]["password"] = "123456";
    configDoc["Mqtt"]["topic"] = "superCube/topic";
    configDoc["Mqtt"]["autoReconnected"] = true;
    configDoc["serverMode"] = "Mqtt";
    configDoc["light"].to<JsonArray>();
    configDoc["light_presets"].to<JsonObject>();
    configDoc["Attitude"]["enable"] = false;
    configDoc["Attitude"]["SCL"] = 6;
    configDoc["Attitude"]["SDA"] = 7;
}

// Validate the config JSON structure
bool ConfigManager::validateConfig() {
    return std::all_of(requiredKeys->begin(), requiredKeys->end(), [this](const auto &keyPair) {
        const auto &key = keyPair.first;
        const auto &subKeys = keyPair.second;
        return !configDoc[key].isNull() &&
               std::all_of(subKeys.begin(), subKeys.end(), [this, &key](const auto &subKey) {
                   return !configDoc[key][subKey].isNull();
               });
    });
}

template<typename T>
CommandNode *ConfigManager::_init_generic(std::string node, std::function<void(JsonVariant, T)> setter) {
    return superCube->command_registry
            ->Literal(node)
            ->runs([this, node](std::unique_ptr<Shell> shell, const R &context) {
                shell->println(configDoc[node].as<String>().c_str());
            })
            ->then(superCube->command_registry
                           ->Literal("set")
                           ->then(superCube->command_registry
                                          ->Param<T>("value")
                                          ->runs([this, node, setter](std::unique_ptr<Shell> shell, const R &context) {
                                              setter(configDoc[node], context.get<T>("value"));
                                              saveConfig();
                                          })
                           )
            );
}

CommandNode *ConfigManager::_init_stringer(std::string node) {
    return _init_generic<std::string>(std::move(node), [](JsonVariant doc, std::string value) {
        doc.set(value.c_str());
    });
}

CommandNode *ConfigManager::_init_boolean(std::string node) {
    return _init_generic<bool>(std::move(node), [](JsonVariant doc, bool value) {
        doc.set(value);
    });
}

CommandNode *ConfigManager::_init_inter(std::string node) {
    return _init_generic<int>(std::move(node), [](JsonVariant doc, int value) {
        doc.set(value);
    });
}

void ConfigManager::registerNodeCommands(const std::string &path, JsonVariant variant, CommandNode *parentNode,
                                         JsonVariant doc) {
    if (path == "light" || path == "light_presets")
        return;
    if (variant.is<bool>()) {
        parentNode->then(_init_boolean(path));
        return;
    } else if (variant.is<const char *>()) {
        parentNode->then(_init_stringer(path));
        return;
    } else if (variant.is<int>()) {
        parentNode->then(_init_inter(path));
        return;
    }
    for (JsonPair kv: variant.as<JsonObject>()) {
        if (kv.key() == "light" || kv.key() == "light_presets")
            continue;
        std::string newPath = path.empty() ? kv.key().c_str() : path + "." + kv.key().c_str();
        CommandNode *subNode = superCube->command_registry->Literal(kv.key().c_str());

        // 获取当前 doc 的子成员，确保递归进入到下一层嵌套对象
        JsonVariant nextDoc = doc[kv.key()];  // 使用 [] 访问子成员

        // 如果该成员存在，递归调用下一层
        if (!nextDoc.isNull()) {
            registerNodeCommands(newPath, kv.value(), subNode, nextDoc);
            parentNode->then(subNode);
        }
    }

}


void ConfigManager::command_initialize() {
    // Assuming you have included the necessary libraries and defined CommandNode, Shell, R, etc.

    CommandNode *literal = superCube->command_registry->Literal("config");
    literal->then(
            superCube->command_registry->Literal("get")->runs([this](std::unique_ptr<Shell> shell, const R &context) {
                shell->println(superCube->config_manager->getConfig().as<String>().c_str());
            })
    );

    literal->then(superCube->command_registry->Literal("setFromJson")->runs(
            [this](std::unique_ptr<Shell> shell, const R &context) {
                if (shell->getHttpMode() || shell->getMqttMode()) {
                    superCube->config_manager->clear();
                    superCube->config_manager->clearConfigDoc();
                    configDoc.set(shell->jsonDoc["config"]);
                    superCube->config_manager->saveConfig();
                    shell->println("Config Replace Successful");
                } else {
                    shell->println("Only can be used in HTTP Mode");
                }
            }));


// Loop through the required keys
//    for (const auto &key: requiredKeys) {
//        if (key.second.empty()) {
//            if (configDoc[key.first].is<int>()) {
//                literal->then(_init_inter(key.first));
//            } else if (configDoc[key.first].is<std::string>() || configDoc[key.first].is<String>()) {
//                literal->then(_init_stringer(key.first));
//            } else if (configDoc[key.first].is<bool>()) {
//                literal->then(_init_boolean(key.first));
//            }
//            continue;
//        }
//        CommandNode *subLiteral = superCube->command_registry->Literal(key.first.c_str());
//        for (const auto &subKey: key.second) {
//            superCube->serial->println(("  Processing subKey: " + subKey).c_str());
//
//            if (subKey == "ssid" || subKey == "passwd" || subKey == "ip" || subKey == "username" ||
//                subKey == "password" || subKey == "topic") {
//                subLiteral->then(superCube->command_registry->Literal(subKey.c_str())->then(_init_stringer(subKey)));
//            } else if (subKey == "port") {
//                subLiteral->then(superCube->command_registry->Literal(subKey.c_str())->then(_init_inter(subKey)));
//            }
//        }
//        literal->then(subLiteral);
//    }
    registerNodeCommands("", configDoc.as<JsonVariant>(), literal, configDoc.operator JsonVariant());
    superCube->command_registry->register_command(std::unique_ptr<CommandNode>(literal));
}

void ConfigManager::clearConfigDoc() {
    configDoc.clear();
}

JsonDocument &ConfigManager::getConfig() {
    return configDoc;
}