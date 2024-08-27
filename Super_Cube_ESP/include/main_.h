//
// Created by Esuny on 2024/8/26.
//

#ifndef SUPER_CUBE_ESP_MAIN__H
#define SUPER_CUBE_ESP_MAIN__H
extern const char *_ssid;
extern const char *_password;
const int baud = 115200;


/* EEPROM */
#define EEPROM_SIZE 8192         // EEPROM 内存空间
const char EEPROM_INIT_FLAG = '\13';      // EEPROM 初始化标记
const char EEPROM_END_CHAR = '\12';       // EEPROM 终点标志


//void setup();
//
//void loop();

#endif //SUPER_CUBE_ESP_MAIN__H
