//
// Created by Esuny on 2024/8/26.
//
#include "utils/EEPROM_Utils.h"
#include <main_.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

std::map<String, int> EEPROMManager::mappingTable;

EEPROMManager::EEPROMManager() {
    EEPROM.begin(EEPROM_SIZE);
    this->eepromSize = EEPROM_SIZE;
    this->nextAddress = 0;
}

void EEPROMManager::initialize() {
    if (!readMappingTable()) {
        clear();
    }
}

void EEPROMManager::writeString(const String &name, const String &str) {
    int address = getAddress(name);
    if (address == -1) {
        address = allocateMemory(name, str.length() + 1);
    }

    for (unsigned int i = 0; i < str.length(); i++) {
        EEPROM.write(address + i, str[i]);
    }
    EEPROM.write(address + str.length(), '\0');
    EEPROM.commit();
}

String EEPROMManager::readString(const String &name) {
    int address = getAddress(name);
    if (address == -1) return "";

    String str;
    char ch;
    while ((ch = EEPROM.read(address++)) != '\0') {
        str += ch;
    }
    return str;
}

void EEPROMManager::clear() {
    for (unsigned int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    mappingTable.clear();
    nextAddress = 0;
}

int EEPROMManager::getAddress(const String &name) {
    if (mappingTable.find(name) != mappingTable.end()) {
        return mappingTable[name];
    }
    return -1;
}

int EEPROMManager::allocateMemory(const String &name, int size) {
    if (nextAddress + size >= eepromSize) {
        return -1;
    }

    int address = nextAddress;
    mappingTable[name] = address;
    nextAddress += size;
    saveMappingTable();
    return address;
}

void EEPROMManager::saveMappingTable() {
    JsonDocument doc;
    for (const auto &kv : mappingTable) {
        doc[kv.first] = kv.second;
    }

    String json;
    serializeJson(doc, json);

    int len = json.length();
    EEPROM.write(0, len);
    for (int i = 0; i < len; i++) {
        EEPROM.write(1 + i, json[i]);
    }
    EEPROM.commit();
}

bool EEPROMManager::readMappingTable() {
    int len = EEPROM.read(0);
    if (len > 0 && len < eepromSize) {
        String json;
        for (int i = 0; i < len; i++) {
            json += (char) EEPROM.read(1 + i);
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (!error) {
            for (JsonPair kv : doc.as<JsonObject>()) {
                mappingTable[kv.key().c_str()] = kv.value().as<int>();
            }
            nextAddress = findNextAvailableAddress();
            return true;
        }
    }
    return false;
}

int EEPROMManager::findNextAvailableAddress() {
    int maxAddress = 0;
    for (const auto &kv : mappingTable) {
        int address = kv.second;
        int size = kv.second;
        if (address + size > maxAddress) {
            maxAddress = address + size;
        }
    }
    return maxAddress;
}