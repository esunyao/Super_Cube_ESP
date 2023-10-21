//
// Created by Esuny on 2023/10/21.
//
#ifndef SUPER_CUBE_ESP_SERVER_LOGGER_H
#define SUPER_CUBE_ESP_SERVER_LOGGER_H

#include <main.h>

class Logger {
public:

    void info() {}

    void success() {}

    void critical() {}

    void warning() {}

    void debug() {}

    template<typename T, typename... Ts>
    void info(T arg1, Ts... arg_left) {
        Serial.println("\033[00m[\033[01mINFO\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + String(arg1) + "\033[00m");
        info(arg_left...);
    }

    template<typename T, typename... Ts>
    void success(T arg1, Ts... arg_left) {
        Serial.println(
                "\033[00m[\033[01m\033[32mSUCCESS\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + String(arg1) + "\033[00m");
        success(arg_left...);
    }

    template<typename T, typename... Ts>
    void critical(T arg1, Ts... arg_left) {
        Serial.println(
                "\033[00m[\033[01m\033[06m\033[41mCRITICAL\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + String(arg1) +
                "\033[00m");
        critical(arg_left...);
    }

    template<typename T, typename... Ts>
    void warning(T arg1, Ts... arg_left) {
        Serial.println(
                "\033[00m[\033[01m\033[33mWARNING\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + String(arg1) + "\033[00m");
        warning(arg_left...);
    }

    template<typename T, typename... Ts>
    void debug(T arg1, Ts... arg_left) {
        if (DEBUG) {
            Serial.println(
                    "\033[00m[\033[01m\033[36mDEBUG\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + String(arg1) + "\033[00m");
            debug(arg_left...);
        }
    }

    void set_Debug(bool debug) {
        DEBUG = debug;
    }

private:
    bool DEBUG = false;
};


#endif //SUPER_CUBE_ESP_SERVER_LOGGER_H
