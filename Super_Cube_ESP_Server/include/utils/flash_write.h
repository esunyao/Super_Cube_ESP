//
// Created by Esuny on 2023/10/21.
//
#ifndef SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H
#define SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H

#include <main.h>

template<typename Args> Logger<Args> logger;

void writeStringToEEPROM(int startAddr, String data);

String readStringFromEEPROM(int startAddr, int length) ;

/* 读取 */
String readString(int addr) ;

/* 写入 */
void writeString(int addr, String data) ;


#endif //SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H
