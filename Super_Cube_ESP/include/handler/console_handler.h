//
// Created by Esuny on 2024/8/27.
//

#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include <Arduino.h>
#include "super_cube.h"

class super_cube;

class Shell;

class SerialHandler {
public:

    SerialHandler(super_cube *cube, HardwareSerial *serial);

    void start();

    void handleSerial();

    void sendResponse(const String &response);

private:
    super_cube *superCube;
    Shell *shell;
    HardwareSerial *serial;
};

#endif // SERIAL_HANDLER_H