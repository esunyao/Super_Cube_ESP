//
// Created by Esuny on 2024/8/29.
//
#include <service/WebsocketService.h>

WebSocketService::WebSocketService(super_cube *superCube, String ip, int port) : ip(ip), port(port),
                                                                                 superCube(superCube) {
    webSocket = nullptr;
}

WebSocketService::~WebSocketService() {
}

void WebSocketService::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            superCube->debugln("Disconnected from WebSocket server");
            break;
        case WStype_CONNECTED:
            superCube->debugln("Connected to WebSocket server");
            webSocket->sendTXT("Hello from ESP8266");
            break;
        case WStype_TEXT:
            break;
        case WStype_BIN:
            superCube->debugln("Received binary message");
            break;
        case WStype_PING:
            superCube->debugln("Received ping");
            break;
        case WStype_PONG:
            superCube->debugln("Received pong");
            break;
        case WStype_ERROR:
            superCube->debugln("Error");
            break;
    }
}

void WebSocketService::start() {
    webSocket = std::make_unique<WebSocketsClient>();
    webSocket->begin(ip, port, "/");
    webSocket->onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        this->handleWebSocketEvent(type, payload, length);
    });
}

void WebSocketService::stop() {
    if (webSocket && webSocket->isConnected()) {
        webSocket->disconnect();
        webSocket.reset();  // Clear the unique_ptr
    }
}

void WebSocketService::sendResponse(String &response) {
    if (webSocket && webSocket->isConnected()) {
        webSocket->sendTXT(response);
    }
}