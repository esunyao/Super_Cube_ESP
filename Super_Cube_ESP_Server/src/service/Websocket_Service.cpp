//
// Created by Esuny on 2023/10/21.
//

#include "service/Websocket_Service.h"
extern Logger logger;
extern WebSocketsServer server;
extern std::map<uint8_t, IPAddress> WebSocketsClient;

void Websocket_Service::Start_Websocket() {

    // 参考https://blog.csdn.net/yq543858194/article/details/109186301

    // 启动ws服务器
    server.begin();
    // 指定事件处理函数
    server.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        {
            if (type == WStype_CONNECTED) {
                // 若为客户端连接事件，显示提示信息
                logger.info("[" + String(num) + "] 连接到服务端");
                WebSocketsClient[num] = server.remoteIP(num);
            } else if (type == WStype_DISCONNECTED) {
                // 若为连接断开事件，显示提示信息
                logger.info("[" + String(num) + "] 断开了连接");
                WebSocketsClient.erase(num);
            } else if (type == WStype_TEXT) {
                // 接收来自客户端的信息（客户端FLASH按键状态），并控制LED的工作
                logger.debug("[" + String(num) + "] " + String((char*)payload));
                DynamicJsonDocument msg(2048);
                deserializeJson(msg, String((char*)payload));
//                executeCallback(num, msg["command"], msg);
            }
        }
    });
}