//
// Created by Esuny on 2024/8/26.
//
#include "config/ConfigManager.h"
#include "main_.h"
#include <LittleFS.h>
#include <algorithm>
#include <utility>
#include "ArduinoJson.h"
#include "utils/uuid_utils.h"

// Constructor to initialize SPIFFS
ConfigManager::ConfigManager(super_cube *superCube) : superCube(superCube) {
    if (!LittleFS.begin()) {
        LittleFS.format();  // 初始化失败时格式化
        LittleFS.begin();
    }
}

// Initialize SPIFFS and load config
void ConfigManager::initialize() {
    // Read config from SPIFFS
    if (!readConfig() || !validateConfig()) {
        // Handle invalid or missing config
        clear();
        clearConfigDoc();
        // Optionally set default config here if needed
        createDefaultConfig();  // Create the default config
        saveConfig();  // Save it to SPIFFS
    }
    if (configDoc["reset"] == true) {
        clear();
        clearConfigDoc();
        createDefaultConfig();
        saveConfig();
    }
    requiredKeys.reset(); // 释放requiredKeys内存
}

// Clear the config file
void ConfigManager::clear() {
    LittleFS.remove("/config.msgpack");
}

// Save the config from the JsonDocument to SPIFFS
void ConfigManager::saveConfig() {
    File file = LittleFS.open("/config.msgpack", "w");
    if (!file) return;
    size_t len = serializeMsgPack(configDoc, file);
    file.close();
}

// Read the config from SPIFFS into the JsonDocument
bool ConfigManager::readConfig() {
    if (!LittleFS.exists("/config.msgpack")) return false;
    File file = LittleFS.open("/config.msgpack", "r");
    if (!file) return false;
    clearConfigDoc();
    DeserializationError error = deserializeMsgPack(configDoc, file);
    file.close();
    return !error;
}

// 以下方法无需修改（保持原有逻辑）
void ConfigManager::createDefaultConfig() {
    String uuid = generateUUIDv4();
    configDoc["reset"] = false;
    configDoc["DEBUG"] = false;
    configDoc["HTTPDEBUG"] = false;
    configDoc["MQTTDEBUG"] = false;
    configDoc["ATTITUDEDEBUG"] = false;
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
    configDoc["Attitude"]["RX"] = 7;
    configDoc["Attitude"]["TX"] = 8;
    configDoc["Attitude"]["MODE"] = "JY901L";
    configDoc["Attitude"]["Baud"] = 9600;
    configDoc["Attitude"]["AutoPublicAttitude"] = false;
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
CommandNode *
ConfigManager::_init_generic(std::string node, JsonVariant doc, std::function<void(JsonVariant, T)> setter) {
    return superCube->command_registry->Literal("set")
            ->then(superCube->command_registry
                           ->Param<T>("value")
                           ->runs([this, node, setter, doc](std::unique_ptr<Shell> shell,
                                                            const R &context) {
                               setter(doc, context.get<T>("value"));
                               shell->println((TypeName<T>::get() + " Completely set " +
                                               context.get<std::string>("value")).c_str());
                               saveConfig();
                           })
            );
}

void ConfigManager::_init_get(std::unique_ptr<Shell> shell, const R &context, JsonVariant doc) {
    shell->println(doc.as<String>().c_str());
}

CommandNode *ConfigManager::_init_stringer(std::string node, JsonVariant doc) {
    return _init_generic<std::string>(std::move(node), std::move(doc), [](JsonVariant doc, std::string value) {
        doc.set(value.c_str());
    });
}

CommandNode *ConfigManager::_init_boolean(std::string node, JsonVariant doc) {
    return _init_generic<bool>(std::move(node), std::move(doc), [](JsonVariant doc, bool value) {
        doc.set(value);
    });
}

CommandNode *ConfigManager::_init_inter(std::string node, JsonVariant doc) {
    return _init_generic<int>(std::move(node), std::move(doc), [](JsonVariant doc, int value) {
        doc.set(value);
    });
}

void ConfigManager::registerNodeCommands(const std::string &path, JsonVariant variant, CommandNode *parentNode,
                                         JsonVariant doc) {
    if (path == "light" || path == "light_presets")
        return;
    if (variant.is<bool>()) {
        parentNode->then(_init_boolean(path, doc));
        parentNode->runs(std::bind(&ConfigManager::_init_get, this, std::placeholders::_1, std::placeholders::_2, doc));
        return;
    } else if (variant.is<const char *>()) {
        parentNode->then(_init_stringer(path, doc));
        return;
    } else if (variant.is<int>()) {
        parentNode->then(_init_inter(path, doc));
        return;
    } else if (variant.is<JsonObject>())
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
                String output;
                serializeMsgPack(configDoc, output);
                shell->println(output.c_str());
            })
    );
    literal->then(
            superCube->command_registry->Literal("gets")->runs([this](std::unique_ptr<Shell> shell, const R &context) {
                shell->println(superCube->config_manager->getConfig().as<String>().c_str());
            })
    );

    literal->then(superCube->command_registry->Literal("setFromJson")->runs(
            [this](std::unique_ptr<Shell> shell, const R &context) {
                if (shell->isNetworkFlag()) {
                    superCube->config_manager->clear();
                    superCube->config_manager->clearConfigDoc();
                    configDoc.set(shell->jsonDoc["config"]);
                    superCube->config_manager->saveConfig();
                    shell->println("Config Replace Successful");
                } else {
                    shell->println("Only can be used in Network Mode");
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