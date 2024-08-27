//
// Created by Esuny on 2024/8/26.
//
#include "super_cube.h"
#include <ESP8266WiFi.h> // Include the WiFi header
#include <utils/uuid_utils.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include <WString.h>
#include <main_.h>

super_cube::super_cube(HardwareSerial &serial) : serial(serial) {
    serial.println("SuperCube::SuperCube()");

}

super_cube::~super_cube() {
    // Destructor implementation
}

void super_cube::setup() {
//    EEPROM_Manger.initialize();
    _connectWiFi(_ssid, _password);
    command_registry.add_command(Command(
            flash_string_vector{std::string(String(F("digitalRead")).c_str())}, // Convert F() result to std::string
            flash_string_vector{std::string(String(F("digitalRead")).c_str())}, // Convert F() result to std::string
            [](Shell &shell, const std::vector<std::string> &arguments) {
                uint8_t pin = String(arguments[0].c_str()).toInt();
                auto value = digitalRead(pin);
                shell.println("asdfasdfasdf");
            }
    ));
}

void super_cube::loop(){

}

void super_cube::_connectWiFi(const char *ssid, const char *password) {
    WiFi.begin(ssid, password);
    serial.print("Connecting to WiFi");

    int attempts = 0;
    const int maxAttempts = 20; // Maximum number of attempts to connect to WiFi

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        serial.println("\nWiFi connected");
        serial.print("IP address: ");
        serial.println(WiFi.localIP());
    } else {
        serial.println("\nFailed to connect to WiFi. Starting AP mode...");

        // Start Access Point mode
        String uuid = generateUUIDv4();
        WiFi.softAP("SuperCube_" + uuid.substring(uuid.length() - 5), "password123");
        serial.print("AP IP address: ");
        serial.println(WiFi.softAPIP());
    }
}