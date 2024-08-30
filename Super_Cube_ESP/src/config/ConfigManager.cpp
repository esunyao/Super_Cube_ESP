//
// Created by Esuny on 2024/8/26.
//
#include "config/ConfigManager.h"
#include "main_.h"
#include <EEPROM.h>
#include "ArduinoJson.h"
#include "utils/uuid_utils.h"

// Constructor to initialize EEPROM
ConfigManager::ConfigManager() {
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
}

// Validate the config JSON structure
bool ConfigManager::validateConfig() {
    // List of required keys and their sub-keys
    const std::map<String, std::vector<String>> requiredKeys = {
            {"reset",      {}},
            {"DEBUG",      {}},
            {"ID",         {}},
            {"Internet",   {"ssid", "passwd"}},
            {"http",       {"port"}},
            {"Websocket",  {"ip",   "port"}},
            {"Mqtt",       {"ip",   "port", "username", "password", "topic"}},
            {"serverMode", {}}
    };

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

void ConfigManager::clearConfigDoc() {
    configDoc.clear();
}

JsonDocument &ConfigManager::getConfig() {
    return configDoc;
}