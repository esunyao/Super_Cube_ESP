//
// Created by Esuny on 2023/10/21.
//

#include "Super_Cube_ESP_Server.h"

extern Logger logger;
extern WebSocketsServer server;
extern std::map<uint8_t, IPAddress> WebSocketsClient;
extern flash_write FlashWrite;

void Super_Cube_ESP_Server::_on_start() {
    load_config();
    logger.info("开始执行开机任务");
    {
        DynamicJsonDocument st(2048);
        st["step"] = Config["LED"];
        st["Save"] = false;
//        executeCallback(0, "TurnLight", st);
    }
    load_wifi();
    Websocket_Service WebsocketService = Websocket_Service();
    WebsocketService.Start_Websocket();
}



void Super_Cube_ESP_Server::load_config() {
    logger.debug(FlashWrite.readString(1));
    deserializeJson(Config, FlashWrite.readString(1));
    logger.set_Debug(Config["Logger"]["Debug"]);
    String CONFIG_TEST;
    logger.debug(FlashWrite.readString(1));
    serializeJson(Config, CONFIG_TEST);
    logger.success("Config:" + String(CONFIG_TEST));
    logger.success("成功加载配置文件");
}

void Super_Cube_ESP_Server::load_wifi() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Super_Cube", "badou2008");
    IPAddress staticIP;  // 设置静态IP地址
    IPAddress gateway;   // 设置网关
    IPAddress subnet;    // 设置子网掩码

    WiFi.begin(String(Config["WiFi"]["Connect"]["ssid"]), String(Config["WiFi"]["Connect"]["passwd"]));
    for (int i = 0; i <= WIFIConnectTimeOut; i++) {
        if (WiFi.status() == WL_CONNECTED)
            break;
        if (i % 10 == 0)
            logger.warning("尝试连接中....");
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED) {
        logger.critical("WiFi连接失败或WiFi设置错误");
    } else {
        logger.success("网络已连接");
        if (staticIP.fromString(String(Config["WiFi"]["ip"])) && gateway.fromString(String(Config["WiFi"]["gateway"])) && subnet.fromString(String(Config["WiFi"]["subnet"]))) {
            WiFi.config(staticIP, gateway, subnet);
            // WiFi.softAPConfig(staticIP, gateway, subnet);
            logger.success("已设置网络的静态IP地址、网关和子网掩码");
        } else {
            logger.critical("IP地址格式错误");
        }
        IPAddress localIP = WiFi.localIP();
        logger.success("IP:" + localIP.toString());
    }
}