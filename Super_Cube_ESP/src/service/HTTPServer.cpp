//
// Created by Esuny on 2024/8/29.
//
#include <service/HTTPService.h>
#include <regex>

HttpServer::HttpServer(super_cube *superCube, int port) : httpServer(port),
                                                          superCube(superCube) {
}

void HttpServer::start() {
    httpServer.on("/", HTTP_GET, [this]() { handleRoot(); });
    httpServer.on("/data", HTTP_POST, [this]() { handleData(); });
    httpServer.on("/cmd_command_execute", HTTP_POST, [this]() { handleCmdCommandExecution(); });
    httpServer.on("/command_execute", HTTP_POST, [this]() { handleCommandExecution(); });
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

void HttpServer::handleCmdCommandExecution() {
    std::unique_ptr<JsonDocument> jsonDoc = std::make_unique<JsonDocument>();
    String requestBody = httpServer.arg("plain");
    DeserializationError error = deserializeJson(*jsonDoc, requestBody);
    IPAddress clientIP = httpServer.client().remoteIP();
    superCube->debugln("[HttpServer] Get request from: ", clientIP.toString());
    if (!error) {
        superCube->debugln("[HttpServer][", clientIP.toString(), "] ", jsonDoc->as<String>());
        if (jsonDoc->operator[]("command").is<std::string>()) {
            superCube->hdebugln("[HttpServer] Command: ", jsonDoc->operator[]("command").as<String>());
            std::unique_ptr<Shell> shell = std::make_unique<Shell>(superCube, true, false);
            shell->setup();
            httpServer.send(200, "application/json", superCube->command_registry->execute_command(std::move(shell), jsonDoc->operator[]("command").as<std::string>())->res);
        }
    } else {
        superCube->hdebugln("[HttpServer] JSON deserialization failed: ");
        superCube->hdebugln(error.c_str());
        httpServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
}

void HttpServer::handleCommandExecution() {
    {
        JsonDocument jsonDoc = JsonDocument();
        String requestBody = httpServer.arg("plain");
        DeserializationError error = deserializeJson(jsonDoc, requestBody);
        IPAddress clientIP = httpServer.client().remoteIP();
        superCube->debugln("[HttpServer] Get request from: ", clientIP.toString());
        if (!error) {
            superCube->debugln("[HttpServer][", clientIP.toString(), "] ", jsonDoc.as<String>());
            if (jsonDoc.operator[]("command").is<std::string>()) {
                superCube->hdebugln("[HttpServer] Command: ", jsonDoc.operator[]("command").as<String>());
                std::unique_ptr<Shell> shell = std::make_unique<Shell>(superCube, true, false);
                shell->setup();
                shell->jsonDoc = jsonDoc;
                superCube->command_registry->execute_command(std::move(shell),
                                                             jsonDoc.operator[]("command").as<std::string>()).reset();
            }
        } else {
            superCube->hdebugln("[HttpServer] JSON deserialization failed: ");
            superCube->hdebugln(error.c_str());
            httpServer.send(400, "application/json", R"({"error":"Invalid JSON"})");
            return;
        }
    }
}