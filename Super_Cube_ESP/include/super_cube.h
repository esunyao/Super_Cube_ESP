//
// Created by Esuny on 2024/8/26.
//

#ifndef SUPER_CUBE_ESP_SUEPR_CUBE_H
#define SUPER_CUBE_ESP_SUEPR_CUBE_H
#include <HardwareSerial.h>
#include "EEPROM.h"
#include "utils/EEPROM_Utils.h"
#include "command/CommandManager.h"

class super_cube {
public:
    super_cube(HardwareSerial &serial);
    ~super_cube();

    void setup();
    void loop();
protected:
    void _connectWiFi(const char *ssid, const char *password);
private:
    HardwareSerial &serial;
    EEPROMManager EEPROM_Manger;
    CommandRegistry command_registry;
};

#endif //SUPER_CUBE_ESP_SUEPR_CUBE_H
