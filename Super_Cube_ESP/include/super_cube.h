//
// Created by Esuny on 2024/8/26.
//

#ifndef SUPER_CUBE_ESP_SUEPR_CUBE_H
#define SUPER_CUBE_ESP_SUEPR_CUBE_H

#include <HardwareSerial.h>
#include "EEPROM.h"
#include "utils/EEPROM_Utils.h"
#include "command/CommandManager.h"
#include "handler/console_handler.h"
#include <Adafruit_NeoPixel.h>

class SerialHandler;

class CommandRegistry;

class Shell;

class super_cube {
public:
    super_cube(HardwareSerial *serial);

    ~super_cube();

    void setup();

    void loop();

    CommandRegistry *command_registry;
    Adafruit_NeoPixel *strip;
    HardwareSerial *serial;
protected:
    void _connectWiFi(const char *ssid, const char *password);
    void _command_register();
private:
    EEPROMManager EEPROM_Manger;
    SerialHandler *serialHandler;
};

#endif //SUPER_CUBE_ESP_SUEPR_CUBE_H
