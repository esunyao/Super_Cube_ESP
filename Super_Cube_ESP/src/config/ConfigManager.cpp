//
// Created by Esuny on 2024/8/26.
//
#include "config/ConfigManager.h"
#include "main_.h"
#include <EEPROM.h>
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
    EEPROM.write(0, len);  // Write the length of the JSON data
    for (int i = 0; i < len; i++) {
        EEPROM.write(1 + i, json[i]);  // Write the JSON data itself
    }
    EEPROM.commit();
}

// Read the config from EEPROM into the JsonDocument
bool ConfigManager::readConfig() {
    int len = EEPROM.read(0);  // First byte is the length
    clearConfigDoc();
    if (len > 0 && len < eepromSize) {
        String json;
        for (int i = 0; i < len; i++) {
            json += (char) EEPROM.read(1 + i);  // Read the JSON string
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
    configDoc["ID"] = uuid.substring(uuid.length() - 5);
    configDoc["Internet"]["ssid"] = "inhand";
    configDoc["Internet"]["passwd"] = "33336666";
    configDoc["http"]["port"] = 80;
    configDoc["Websocket"]["ip"] = "";
    configDoc["Websocket"]["port"] = 80;
    configDoc["Mqtt"]["ip"] = "";
    configDoc["Mqtt"]["port"] = 80;
    configDoc["Mqtt"]["username"] = "";
    configDoc["Mqtt"]["password"] = "";
    configDoc["Mqtt"]["topic"] = "superCube/topic";
    configDoc["serverMode"] = "http";
    configDoc["light"] = JsonArray();
}

// Validate the config JSON structure
bool ConfigManager::validateConfig() {
    // Iterate through the required keys and check if they exist in the config
    for (const auto &key: requiredKeys) {
        if (!configDoc.containsKey(key.first)) {
            return false;
        }
        for (const auto &subKey: key.second) {
            if (!configDoc[key.first].containsKey(subKey)) {
                return false;
            }
        }
    }

    // Additional checks for specific value constraints can be added here

    return true;
}

CommandNode *ConfigManager::_init_stringer(std::string node) {
    return superCube->command_registry
            ->Literal(node.c_str())
            ->runs([this, node](Shell *shell, const R &context) {
                shell->println(configDoc[node].as<String>().c_str());
            })
            ->then(superCube->command_registry
                           ->Literal("set")
                           ->then(superCube->command_registry
                                          ->StringParam("value")
                                          ->runs([this, node](Shell *shell, const R &context) {
                                              configDoc[node] = context.get<std::string>("value").c_str();
                                              saveConfig();
                                          })));
}

CommandNode *ConfigManager::_init_boolean(std::string node) {
    return superCube->command_registry
            ->Literal(node.c_str())
            ->runs([this, node](Shell *shell, const R &context) {
                shell->println(configDoc[node].as<String>().c_str());
            })
            ->then(superCube->command_registry
                           ->Literal("set")
                           ->then(superCube->command_registry
                                          ->BooleanParam("value")
                                          ->runs([this, node](Shell *shell, const R &context) {
                                              configDoc[node] = context.get<boolean>("value");
                                              saveConfig();
                                          })));
}

CommandNode *ConfigManager::_init_inter(std::string node) {
    return superCube->command_registry
            ->Literal(node.c_str())
            ->runs([this, node](Shell *shell, const R &context) {
                shell->println(configDoc[node].as<String>().c_str());
            })
            ->then(superCube->command_registry
                           ->Literal("set")
                           ->then(superCube->command_registry
                                          ->IntegerParam("value")
                                          ->runs([this, node](Shell *shell, const R &context) {
                                              configDoc[node] = context.get<int>("value");
                                              saveConfig();
                                          })));
}

void ConfigManager::command_initialize() {
    // Assuming you have included the necessary libraries and defined CommandNode, Shell, R, etc.

    CommandNode *literal = superCube->command_registry->Literal("config");
    literal->then(superCube->command_registry->Literal("get")->runs([this](Shell *shell, const R &context) {
                      shell->println(superCube->config_manager->getConfig().as<String>().c_str());
                  })
    );


// Loop through the required keys
    for (const auto &key: requiredKeys) {
        if (key.second.empty()) {
            if (configDoc[key.first].is<int>()) {
                literal->then(_init_inter(key.first));
            } else if (configDoc[key.first].is<std::string>() || configDoc[key.first].is<String>()) {
                literal->then(_init_stringer(key.first));
            } else if (configDoc[key.first].is<bool>()) {
                literal->then(_init_boolean(key.first));
            }
            continue;
        }
        CommandNode *subLiteral = superCube->command_registry->Literal(key.first.c_str());
        for (const auto &subKey: key.second) {
            superCube->serial->println(("  Processing subKey: " + subKey).c_str());

            if (subKey == "ssid" || subKey == "passwd" || subKey == "ip" || subKey == "username" ||
                subKey == "password" || subKey == "topic") {
                subLiteral->then(superCube->command_registry->Literal(subKey.c_str())->then(_init_stringer(subKey)));
            } else if (subKey == "port") {
                subLiteral->then(superCube->command_registry->Literal(subKey.c_str())->then(_init_inter(subKey)));
            }
        }
        literal->then(subLiteral);
    }
    superCube->command_registry->register_command(std::unique_ptr<CommandNode>(literal));

}

void ConfigManager::clearConfigDoc() {
    configDoc.clear();
}

JsonDocument &ConfigManager::getConfig() {
    return configDoc;
}