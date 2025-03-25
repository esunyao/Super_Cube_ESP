////
//// Created by Esuny on 2024/8/29.
////
//
//#ifndef SUPER_CUBE_ESP_WEBSOCKETSERVICE_H
//#define SUPER_CUBE_ESP_WEBSOCKETSERVICE_H
//
//#include <super_cube.h>
//#include <WebSocketsClient.h>
//
//class WebSocketService {
//public:
//    WebSocketService(super_cube *superCube, String ip, int port);
//
//    ~WebSocketService();
//
//    void start();
//
//    std::unique_ptr<WebSocketsClient> webSocket;
//
//    void stop();
//
//protected:
//    void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
//
//    void sendResponse(String &response);
//
//private:
//    super_cube *superCube;
//    String ip;
//    int port;
//};
//
//#endif //SUPER_CUBE_ESP_WEBSOCKETSERVICE_H
