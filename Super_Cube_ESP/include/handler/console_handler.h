//
// Created by Esuny on 2024/8/27.
//

#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include <Arduino.h>

class SerialHandler {
public:
    SerialHandler(HardwareSerial &serial, unsigned long baudRate);
    void begin();
    void handleSerial();
    void sendResponse(const String &response);

private:
    HardwareSerial &serial;
    unsigned long baudRate;
};

#endif // SERIAL_HANDLER_H