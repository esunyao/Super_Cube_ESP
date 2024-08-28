//
// Created by Esuny on 2024/8/26.
//

#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <map>
#include <ArduinoJson.h>

class EEPROMManager {
public:
    EEPROMManager();

    void initialize();

    void writeString(const String &name, const String &str);

    String readString(const String &name);

protected:
    void clear();

    int getAddress(const String &name);

    int allocateMemory(const String &name, int size);

    void saveMappingTable();

    bool readMappingTable();

    static int findNextAvailableAddress();

private:
    static std::map<String, int> mappingTable;
    int eepromSize;
    int nextAddress;
};

#endif // EEPROM_UTILS_H