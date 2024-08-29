//
// Created by Esuny on 2024/8/29.
//
#include <service/HTTPService.h>


HttpServer::HttpServer(super_cube *superCube, int port) : superCube(superCube),
                                                          httpServer(port) {

}

void HttpServer::start() {
    httpServer.on("/", HTTP_GET, [this]() { handleRoot(); });
    httpServer.on("/data", HTTP_POST, [this]() { handleData(); });
    httpServer.onNotFound([this]() { handleNotFound(); });

    httpServer.begin();
}

void HttpServer::handleData() {
    // 创建一个JSON文档
    JsonDocument jsonDoc;  // 可以根据需要调整大小

    // 读取请求体中的数据
    String requestBody = httpServer.arg("plain");  // 获取原始请求体

    // 解析 JSON 数据
    DeserializationError error = deserializeJson(jsonDoc, requestBody);

    if (error) {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        httpServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // 处理解析后的 JSON 数据
    // 示例: 获取 JSON 中的字段
    if (jsonDoc.containsKey("console_trigger")) {
        String trigger = jsonDoc["console_trigger"].as<String>();

    }
    // 响应客户端
    httpServer.send(200, "application/json", "{\"status\":\"success\"}");
}
void HttpServer::handleClient() {
    httpServer.handleClient();
}

void HttpServer::handleRoot() {
    httpServer.send(200, "text/plain", "Hello, this is ESP8266!");
}

void HttpServer::handleNotFound() {
    httpServer.send(404, "text/plain", "404: Not Found");
}