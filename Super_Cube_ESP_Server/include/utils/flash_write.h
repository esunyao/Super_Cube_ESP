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
        int length = data.length();
        for (int i = 0; i < length; i++) {
            EEPROM.write(startAddr + i, data[i]);
        }
    }
    String readStringFromEEPROM(int startAddr, int length) {
        String data = "";
        for (int i = 0; i < length; i++) {
            char c = EEPROM.read(startAddr + i);
            data += c;
        }
        return data;
    }

/* 读取 */
    String readString(int addr) {
        int numBlocks = EEPROM.read(addr);
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

        // 计算块的数量
        int numBlocks = data.length() / blockSize + 1;

        EEPROM.write(addr, numBlocks);
        // 逐个块地存储字符串
        for (int i = 0; i < numBlocks; i++) {
            String block = data.substring(i * blockSize, (i + 1) * blockSize);
            writeStringToEEPROM(addr + i * blockSize + 1, block);
        }
        logger.debug("已添加保存字符串 " + data);
        EEPROM.commit();  // 写入EEPROM并保存更改
    }
};


#endif //SUPER_CUBE_ESP_SERVER_FLASH_WRITE_H
