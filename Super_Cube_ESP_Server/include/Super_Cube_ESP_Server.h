//
// Created by Esuny on 2023/10/21.
//

#ifndef SUPER_CUBE_ESP_SERVER_SUPER_CUBE_ESP_SERVER_H
#define SUPER_CUBE_ESP_SERVER_SUPER_CUBE_ESP_SERVER_H

#include<main.h>

extern Logger logger;
extern WebSocketsServer server;
extern std::map<String, int> pinMap;
extern DynamicJsonDocument Config;
extern flash_write FlashWrite;

class Super_Cube_ESP_Server{
    void load_config();
    void load_wifi();

public:
    void _on_start();
    void Load_Config() {
        logger.debug(FlashWrite.readString(1));
        deserializeJson(Config, FlashWrite.readString(1));
        logger.set_Debug(Config["Logger"]["Debug"]);
        String CONFIG_TEST;
        serializeJson(Config, CONFIG_TEST);
        logger.success("Config:" + String(CONFIG_TEST));
        logger.success("成功加载配置文件");
    }
    void Save_Config() {
        String CONFIG_STRING;
        serializeJson(Config, CONFIG_STRING);
        FlashWrite.writeString(1, CONFIG_STRING);
        logger.success("成功保存配置文件");
    }
};


#endif //SUPER_CUBE_ESP_SERVER_SUPER_CUBE_ESP_SERVER_H
