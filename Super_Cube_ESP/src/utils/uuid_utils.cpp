//
// Created by Esuny on 2024/8/27.
//
#include <utils/uuid_utils.h>

String generateUUIDv4() {
    uint8_t uuid[16];

    // 使用芯片ID和CPU周期计数器初始化随机数种子
    uint32_t seed = ESP.getCycleCount() ^ ESP.getChipId();
    randomSeed(seed);

    // 生成随机字节数组
    for (int i = 0; i < 16; i++) {
        uuid[i] = (uint8_t) random(0, 256);
    }

    // 设置UUID v4版本和变体
    uuid[6] = 0x40 | (uuid[6] & 0x0F); // UUID版本4: 0100xxxx
    uuid[8] = 0x80 | (uuid[8] & 0x3F); // UUID变体: 10xxxxxx

    // 格式化UUID字符串
    char uuidStr[37];
    sprintf(uuidStr,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5],
            uuid[6], uuid[7],
            uuid[8], uuid[9],
            uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

    return String(uuidStr);
}