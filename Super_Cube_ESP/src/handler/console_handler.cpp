//
// Created by Esuny on 2024/8/27.
//
#include <handler/console_handler.h>
#include <main_.h>

SerialHandler::SerialHandler(super_cube *cube, HardwareSerial *serial)
        : superCube(cube), serial(serial) {
}

void SerialHandler::start() {
    serial->begin(baud);
}

void SerialHandler::handleSerial() {
    if (serial->available()) {
        String input = serial->readStringUntil('\n');
//        String tokens[10]; // Assuming a maximum of 10 tokens
//        int tokenCount = 0;
//        int startIndex = 0;
//        int endIndex = input.indexOf(' ');
//        sendResponse(input);
//        while (endIndex != -1 && tokenCount < 10) {
//            tokens[tokenCount++] = input.substring(startIndex, endIndex);
//            startIndex = endIndex + 1;
//            endIndex = input.indexOf(' ', startIndex);
//        }
//        if (tokenCount < 10) {
//            tokens[tokenCount++] = input.substring(startIndex);
//        }
//
//        if (tokenCount > 0) {
//            String command_name = tokens[0];
//            std::vector<std::string> args;
//            for (int i = 1; i < tokenCount; ++i) {
//                args.push_back(tokens[i].c_str());
//            }
        superCube->command_registry->execute_command(shell, std::string(input.c_str()));
    }
}

void SerialHandler::sendResponse(const String &response) {
    serial->println(response);
}
