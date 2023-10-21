//
// Created by Esuny on 2023/10/21.
//

#include "service/Websocket_Service.h"

extern Logger logger;
extern WebSocketsServer server;
extern WebSocketsClient wsClient;
extern std::map<uint8_t, IPAddress> WebSocketsClientMapList;

extern void executeCallback(uint8_t num, const char *name, JsonDocument &msg);

void Websocket_Service::Start_Websocket(bool val) {

    // 参考https://blog.csdn.net/yq543858194/article/details/109186301

    if (val) {
        // 启动ws服务器
        server.begin();
        // 指定事件处理函数
        server.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
            {
                if (type == WStype_CONNECTED) {
                    // 若为客户端连接事件，显示提示信息
                    logger.info("[" + String(num) + "] 连接到服务端");
                    WebSocketsClientMapList[num] = server.remoteIP(num);
                } else if (type == WStype_DISCONNECTED) {
                    // 若为连接断开事件，显示提示信息
                    logger.info("[" + String(num) + "] 断开了连接");
                    WebSocketsClientMapList.erase(num);
                } else if (type == WStype_TEXT) {
                    // 接收来自客户端的信息（客户端FLASH按键状态），并控制LED的工作
                    if (String((char *) payload) != "Client") {
                        logger.debug("[" + String(num) + "] " + String((char *) payload));
                        DynamicJsonDocument msg(2048);
                        deserializeJson(msg, String((char *) payload));
                        executeCallback(num, msg["command"], msg);
                    } else if (String((char *) payload) == "Client" && Config["WiFi"]["Websocket"]["Init"]) {
                        logger.debug(String(num) + "初始化");
                        String configString = String("{\"command\": \"Set_Config\", \"new_Config\": {\"Logger\": {\"Debug\": true}, \"WiFi\": {\"SoftAP\": {\"ssid\": \"Super_Cube\", \"passwd\": \"FRS8571a8438a712517\", \"IsOpen\": false}, \"Connect\": {\"ssid\": \"") +
                                              String(Config["WiFi"]["Connect"]["ssid"]).c_str() + "\", \"passwd\": \"" + String(Config["WiFi"]["Connect"]["passwd"]).c_str() + "\", \"config\": false}, \"Websocket\": {\"Server\": false, \"Client\": true, \"Init\": false}, \"ip\": \"192.168.0.170\", \"gateway\": \"192.168.0.1\", \"subnet\": \"255.255.255.0\"}, \"LED\": {}}";
                        server.sendTXT(num, configString);
                        delay(500);
                        server.sendTXT(num, "{\"command\": \"Restart\"}");

                    }
                }
            }
        });
    } else {
        wsClient.begin("192.168.0.140", 81);
        wsClient.onEvent([](WStype_t type, uint8_t *payload, size_t length) {
            {
                if (type == WStype_CONNECTED) {
                    // 若为客户端连接事件，显示提示信息
                    logger.info("连接到服务端");
                    wsClient.sendTXT("Client");
                } else if (type == WStype_DISCONNECTED) {
                    // 若为连接断开事件，显示提示信息
                    logger.info("断开了连接");
                } else if (type == WStype_TEXT) {
                    logger.debug("[ 接收到服务端数据 ] " + String((char *) payload));
                    DynamicJsonDocument msg(2048);
                    deserializeJson(msg, String((char *) payload));
                    executeCallback(-1, msg["command"], msg);
                }
            }
        });
    }
}