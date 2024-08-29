//
// Created by Esuny on 2024/8/26.
//

#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <map>
#include <ArduinoJson.h>

class EEPROMManager {
public:
    static EEPROMManager &getInstance() {
        static EEPROMManager instance; // 保证只会创建一次，并在第一次使用时初始化
        return instance;
    }

    EEPROMManager();

    void initialize();

    void clear();

    void saveConfig();

    bool readConfig();

    bool validateConfig();

private:
    int eepromSize;
    JsonDocument configDoc;
    void createDefaultConfig();

    void clearConfigDoc();
};

#endif // EEPROM_UTILS_H