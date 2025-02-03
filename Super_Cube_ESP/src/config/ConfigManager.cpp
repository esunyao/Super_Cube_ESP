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
#include <map>
#include <variant>
#include <string>

// 实现 ConfigValue 的成员函数（在头文件末尾或移到 .cpp 中）
inline ConfigValue::ConfigValue(const ConfigValue &other) {
    if (std::holds_alternative<bool>(other.value)) {
        value = std::get<bool>(other.value);
    } else if (std::holds_alternative<int>(other.value)) {
        value = std::get<int>(other.value);
    } else if (std::holds_alternative<std::string>(other.value)) {
        value = std::get<std::string>(other.value);
    } else if (std::holds_alternative<std::unique_ptr<ConfigData>>(other.value)) {
        const auto &ptr = std::get<std::unique_ptr<ConfigData>>(other.value);
        if (ptr)
            value = std::make_unique<ConfigData>(*ptr); // 深拷贝
//        } else {
//            value = nullptr;
//        }
    }
}

inline ConfigValue &ConfigValue::operator=(const ConfigValue &other) {
    if (this != &other) {
        // 与拷贝构造函数相同的逻辑
        if (std::holds_alternative<bool>(other.value)) {
            value = std::get<bool>(other.value);
        } else if (std::holds_alternative<int>(other.value)) {
            value = std::get<int>(other.value);
        } else if (std::holds_alternative<std::string>(other.value)) {
            value = std::get<std::string>(other.value);
        } else if (std::holds_alternative<std::unique_ptr<ConfigData>>(other.value)) {
            const auto &ptr = std::get<std::unique_ptr<ConfigData>>(other.value);
            if (ptr)
                value = std::make_unique<ConfigData>(*ptr); // 深拷贝
//            } else {
//                value = nullptr;
//            }
        }
    }
    return *this;
}

inline ConfigValue &ConfigValue::operator=(bool b) {
    value = b;
    return *this;
}

inline ConfigValue &ConfigValue::operator=(int i) {
    value = i;
    return *this;
}

inline ConfigValue &ConfigValue::operator=(const std::string &s) {
    value = s;
    return *this;
}

inline ConfigValue &ConfigValue::operator=(const char *s) {
    value = std::string(s);
    return *this;
}

inline ConfigValue &ConfigValue::operator=(const ConfigData &d) {
    value = std::make_unique<ConfigData>(d);
    return *this;
}

inline ConfigValue &ConfigValue::operator[](const std::string &key) {
    if (!std::holds_alternative<std::unique_ptr<ConfigData>>(value)) {
        value = std::make_unique<ConfigData>();
    }
    auto &data_ptr = std::get<std::unique_ptr<ConfigData>>(value);
    return (*data_ptr)[key];
}


// 实现 ConfigData 的成员函数
inline ConfigData::ConfigData(const ConfigData &other) : data(other.data) {}

inline ConfigData &ConfigData::operator=(const ConfigData &other) {
    if (this != &other) {
        data = other.data;
    }
    return *this;
}

inline ConfigValue &ConfigData::operator[](const std::string &key) {
    return data[key];
}

inline const ConfigValue &ConfigData::operator[](const std::string &key) const {
    return data.at(key);
}

// 辅助函数：从 JsonObject 递归载入 ConfigData
void ConfigManager::transferConfigDataFromJson(const JsonVariant &jsonVar, ConfigData &dst) {
    if (!jsonVar.is<JsonObject>()) return;
    for (auto kv: jsonVar.as<JsonObject>()) {
        std::string key = kv.key().c_str();
        auto value = kv.value();
        if (value.is<JsonObject>()) {
            ConfigData sub;
            transferConfigDataFromJson(value, sub);
            dst.data[key] = sub;
        } else if (value.is<bool>()) {
            dst.data[key] = value.as<bool>();
        } else if (value.is<int>()) {
            dst.data[key] = value.as<int>();
        } else if (value.is<const char *>() || value.is<String>() || value.is<std::string>()) {
            dst.data[key] = std::string(value.as<const char *>());
        }
    }
}

// 辅助函数：将 ConfigData 递归转换到 JsonVariant（假设 destination 为 JsonObject）
void ConfigManager::transferJsonDataFromConfig(const ConfigData &src, const JsonVariant &destination) {
    for (const auto &pair: src.data) {
        const std::string &key = pair.first;
        const ConfigValue &val = pair.second;
        if (std::holds_alternative<std::unique_ptr<ConfigData>>(val.value)) {
            auto &p = std::get<std::unique_ptr<ConfigData>>(val.value);
            if (p) {
                JsonVariant subObj = destination[key.c_str()].to<JsonObject>();
                transferJsonDataFromConfig(*p, subObj);
            }
        } else if (std::holds_alternative<bool>(val.value)) {
            destination[key.c_str()] = std::get<bool>(val.value);
        } else if (std::holds_alternative<int>(val.value)) {
            destination[key.c_str()] = std::get<int>(val.value);
        } else if (std::holds_alternative<std::string>(val.value)) {
            destination[key.c_str()] = std::get<std::string>(val.value).c_str();
        }
    }
}

// Constructor to initialize EEPROM
ConfigManager::ConfigManager(super_cube *superCube) : superCube(superCube) {
    EEPROM.begin(EEPROM_SIZE);
    this->eepromSize = EEPROM_SIZE;
}

// Initialize EEPROM and load config
void ConfigManager::initialize() {
    if (!configDoc)
        configDoc = std::make_unique<JsonDocument>();
    // Read config from EEPROM
    if (!readConfig() || !validateConfig()) {
        // Handle invalid or missing config
        clear();
        clearConfigDoc();
        // Optionally set default config here if needed
        createDefaultConfig();  // Create the default config
        saveConfig();  // Save it to EEPROM
    }
    if (configDoc->operator[]("reset") == true) {
        clear();
        clearConfigDoc();
        createDefaultConfig();  // Create the default config
        saveConfig();  // Save it to EEPROM
    }
    requiredKeys.reset();
    transferConfigDataFromJson(*configDoc, configData);
    configDoc.reset();
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
    if (!configDoc)
        configDoc = std::make_unique<JsonDocument>();
    transferJsonDataFromConfig(configData, *configDoc);
    String json;
    serializeJson(*configDoc, json);
    int len = json.length();
    EEPROM.write(0, (uint8_t) (len >> 8));  // 写入高字节
    EEPROM.write(1, (uint8_t) (len & 0xFF));  // 写入低字节
    for (int i = 0; i < len; i++) {
        EEPROM.write(4 + i, json[i]);  // Write the JSON data itself
    }
    EEPROM.commit();
    configDoc.reset();
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

        DeserializationError error = deserializeJson(*configDoc, json);
        return !error;  // Return true if deserialization was successful
    }
    return false;  // Return false if there was an issue reading or deserializing
}

void ConfigManager::createDefaultConfig() {
    String uuid = generateUUIDv4();
    configDoc->operator[]("reset") = false;
    configDoc->operator[]("DEBUG") = false;
    configDoc->operator[]("HTTPDEBUG") = false;
    configDoc->operator[]("MQTTDEBUG") = false;
    configDoc->operator[]("ID") = uuid.substring(uuid.length() - 5);
    configDoc->operator[]("Internet")["ssid"] = "inhand";
    configDoc->operator[]("Internet")["passwd"] = "33336666";
    configDoc->operator[]("http")["port"] = 80;
    configDoc->operator[]("Websocket")["ip"] = "";
    configDoc->operator[]("Websocket")["port"] = 80;
    configDoc->operator[]("Mqtt")["ip"] = "192.168.2.10";
    configDoc->operator[]("Mqtt")["port"] = 1883;
    configDoc->operator[]("Mqtt")["callback_topic"] = "superCube/callback";
    configDoc->operator[]("Mqtt")["attitude_topic"] = "superCube/attitude/";
    configDoc->operator[]("Mqtt")["username"] = "SuperCube";
    configDoc->operator[]("Mqtt")["password"] = "123456";
    configDoc->operator[]("Mqtt")["topic"] = "superCube/topic";
    configDoc->operator[]("Mqtt")["autoReconnected"] = true;
    configDoc->operator[]("serverMode") = "Mqtt";
    configDoc->operator[]("light").to<JsonArray>();
    configDoc->operator[]("light_presets").to<JsonObject>();
    configDoc->operator[]("Attitude")["enable"] = false;
    configDoc->operator[]("Attitude")["SCL"] = 6;
    configDoc->operator[]("Attitude")["SDA"] = 7;
}

// Validate the config JSON structure
bool ConfigManager::validateConfig() {
    return std::all_of(requiredKeys->begin(), requiredKeys->end(), [this](const auto &keyPair) {
        const auto &key = keyPair.first;
        const auto &subKeys = keyPair.second;
        return !configDoc->operator[](key).isNull() &&
               std::all_of(subKeys.begin(), subKeys.end(), [this, &key](const auto &subKey) {
                   return !configDoc->operator[](key)[subKey].isNull();
               });
    });
}

template<typename T>
CommandNode *ConfigManager::_init_generic(std::string node, std::function<void(ConfigValue &, T)> setter) {
    // Literal(node) 创建一个节点，用于展示当前值和设置新值
    return superCube->command_registry
            ->Literal(node)
            ->runs([this, node](std::unique_ptr<Shell> shell, const R &context) {
                shell->println(configData[node].as<std::string>().c_str());
            })
            ->then(superCube->command_registry
                           ->Literal("set")
                           ->then(superCube->command_registry
                                          ->Param<T>("value")
                                          ->runs([this, node, setter](std::unique_ptr<Shell> shell, const R &context) {
                                              setter(configData[node], context.get<T>("value"));
                                              saveConfig();
                                          })
                           )
            );
}

CommandNode *ConfigManager::_init_stringer(std::string node) {
    return _init_generic<std::string>(std::move(node), [](ConfigValue doc, std::string value) {
        doc = value;
    });
}

CommandNode *ConfigManager::_init_boolean(std::string node) {
    return _init_generic<bool>(std::move(node), [](ConfigValue doc, bool value) {
        doc = value;
    });
}

CommandNode *ConfigManager::_init_inter(std::string node) {
    return _init_generic<int>(std::move(node), [](ConfigValue doc, int value) {
        doc = value;
    });
}

void ConfigManager::registerNodeCommands(const std::string &path, const ConfigData &data, CommandNode *parentNode) {
    for (const auto &pair: data.data) {
        const std::string &key = pair.first;
        if (key == "light" || key == "light_presets")
            continue;
        std::string newPath = path.empty() ? key : path + "." + key;
        const ConfigValue &val = pair.second;
        if (std::holds_alternative<bool>(val.value)) {
            parentNode->then(_init_boolean(newPath));
        } else if (std::holds_alternative<int>(val.value)) {
            parentNode->then(_init_inter(newPath));
        } else if (std::holds_alternative<std::string>(val.value)) {
            parentNode->then(_init_stringer(newPath));
        } else if (std::holds_alternative<std::unique_ptr<ConfigData>>(val.value)) {
            const auto &p = std::get<std::unique_ptr<ConfigData>>(val.value);
            if (p) {
                CommandNode *childNode = superCube->command_registry->Literal(key.c_str());
                registerNodeCommands(newPath, *p, childNode);
                parentNode->then(childNode);
            }
        }
    }
}


void ConfigManager::command_initialize() {
    // Assuming you have included the necessary libraries and defined CommandNode, Shell, R, etc.

    CommandNode *literal = superCube->command_registry->Literal("config");
    literal->then(
            superCube->command_registry->Literal("get")->runs([this](std::unique_ptr<Shell> shell, const R &context) {
                shell->println(superCube->config_manager->toString().c_str());
            })
    );

    literal->then(superCube->command_registry->Literal("setFromJson")->runs(
            [this](std::unique_ptr<Shell> shell, const R &context) {
                if (shell->getHttpMode() || shell->getMqttMode()) {
                    superCube->config_manager->clear();
                    superCube->config_manager->clearConfigDoc();
                    if (!configDoc)
                        configDoc = std::make_unique<JsonDocument>();
                    configDoc->set(shell->jsonDoc["config"]);
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
    registerNodeCommands("", configData, literal);
    superCube->command_registry->register_command(std::unique_ptr<CommandNode>(literal));
}

void ConfigManager::clearConfigDoc() {
    configDoc.reset();
    configDoc = std::make_unique<JsonDocument>();
}

String ConfigManager::toString() {
    JsonDocument doc;
    transferJsonDataFromConfig(configData, doc.to<JsonObject>());
    return doc.as<String>();
}

ConfigData &ConfigManager::getConfig() {
    return configData;
}