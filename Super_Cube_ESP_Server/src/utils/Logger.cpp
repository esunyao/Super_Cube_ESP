//
// Created by Esuny on 2023/10/21.
//

#include "utils/Logger.h"

template<typename Args>
void Logger::info(Args val) {
    Serial.println("\033[00m[\033[01mINFO\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + val + "\033[00m");
}

template<typename Args>
void Logger::success(Args val) {
    Serial.println(
            "\033[00m[\033[01m\033[32mSUCCESS\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + val + "\033[00m");
}

template<typename Args>
void Logger::critical(Args val) {
    Serial.println(
            "\033[00m[\033[01m\033[06m\033[41mCRITICAL\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + val +
            "\033[00m");
}

template<typename Args>
void Logger::warning(Args val) {
    Serial.println(
            "\033[00m[\033[01m\033[33mWARNING\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + val + "\033[00m");
}

template<typename Args>
void Logger::debug(Args val) {
    if (DEBUG)
        Serial.println(
                "\033[00m[\033[01m\033[36mDEBUG\033[00m] \033[36m\033[04mSuper_Cube\033[00m | " + val + "\033[00m");
}

template<typename Args>
void Logger::set_Debug(bool debug) {
    DEBUG = debug;
}