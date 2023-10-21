//
// Created by Esuny on 2023/10/21.
//
#ifndef SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H
#define SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H

#include <main.h>

extern Logger logger;
extern int blockSize;

class flash_write {
public:
    void writeStringToEEPROM(int startAddr, String data){
        EEPROM.begin(8192);
        int length = data.length();
        for (int i = 0; i < length; i++) {
            EEPROM.write(startAddr + i, data[i]);
        }
        EEPROM.commit();
        EEPROM.end();
    }
    String readStringFromEEPROM(int startAddr, int length) {
        EEPROM.begin(8192);
        String data = "";
        for (int i = 0; i < length; i++) {
            char c = EEPROM.read(startAddr + i);
            data += c;
        }
        EEPROM.end();
        return data;
    }

/* 读取 */
    String readString(int addr) {
        EEPROM.begin(8192);
        int numBlocks = EEPROM.read(addr);
        EEPROM.end();
        String data = "";
        for (int i = 0; i < numBlocks; i++) {
            String block = readStringFromEEPROM(addr + i * blockSize + 1, blockSize);
            data += block;
        }
        return data;
    }

/* 写入 */
    void writeString(int addr, String data) {
//    int len = data.length();
        EEPROM.begin(8192);
        // 计算块的数量
        int numBlocks = data.length() / blockSize + 1;

        EEPROM.write(addr, numBlocks);
        EEPROM.end();
        // 逐个块地存储字符串
        for (int i = 0; i < numBlocks; i++) {
            String block = data.substring(i * blockSize, (i + 1) * blockSize);
            writeStringToEEPROM(addr + i * blockSize + 1, block);
        }
        logger.debug("已添加保存字符串 " + data);
//        EEPROM.commit();  // 写入EEPROM并保存更改
//        EEPROM.end();
    }
};


#endif //SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H
