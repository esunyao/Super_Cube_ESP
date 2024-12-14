//
// Created by Esuny on 2024/8/29.
//
#include <service/HTTPService.h>
#include <regex>

HttpServer::HttpServer(super_cube *superCube, int port) : httpServer(port),
                                                          superCube(superCube) {
    shell = new Shell(superCube, true, false);
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
            shell->setup();
            superCube->command_registry->execute_command(shell, jsonDoc->operator[]("command").as<std::string>());
            httpServer.send(200, "application/json", shell->res);
        }
    } else {
        superCube->hdebugln("[HttpServer] JSON deserialization failed: ");
        superCube->hdebugln(error.c_str());
        httpServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
}

void HttpServer::handleCommandExecution() {
    JsonDocument jsonDoc = JsonDocument();
    String requestBody = httpServer.arg("plain");
    DeserializationError error = deserializeJson(jsonDoc, requestBody);
    IPAddress clientIP = httpServer.client().remoteIP();
    superCube->debugln("[HttpServer] Get request from: ", clientIP.toString());
    if (!error) {
        superCube->debugln("[HttpServer][", clientIP.toString(), "] ", jsonDoc.as<String>());
        if (jsonDoc.operator[]("command").is<std::string>()) {
            superCube->hdebugln("[HttpServer] Command: ", jsonDoc.operator[]("command").as<String>());
            shell->setup();
            shell->jsonDoc = jsonDoc;
            superCube->command_registry->execute_command(shell,
                                                         shell->jsonDoc.operator[]("command").as<std::string>());
        }
    } else {
        superCube->hdebugln("[HttpServer] JSON deserialization failed: ");
        superCube->hdebugln(error.c_str());
        httpServer.send(400, "application/json", R"({"error":"Invalid JSON"})");
        return;
    }
}

void HttpServer::commandRegister() {
    superCube->command_registry->register_command(
            std::unique_ptr<CommandNode>(
                    superCube->command_registry->Literal("Server_NeoPixel")->runs([](Shell *shelll, const R &context) {
                        // {"command": "Server_NeoPixel", "pin": 1, "r": 255, "g": 255, "b": 255, "bright": 255, "num": ["0-3"]}
                        // {"command": "Server_NeoPixel", "presets": ""}
                        {
                            std::map<int, const uint8_t> pinMap = {
                                    {1, D1},
                                    {2, D2},
                                    {3, D3},
                                    {4, D4},
                                    {5, D5},
                                    {6, D6}
                            };
                            if (shelll->jsonDoc.operator[]("save").is<bool>())
                                if (shelll->jsonDoc.operator[]("save").as<bool>()) {
                                    shelll->jsonDoc.remove("save");
                                    shelll->getSuperCube()->config_manager->getConfig()["light"].as<JsonArray>().add(
                                            shelll->jsonDoc);
                                    shelll->getSuperCube()->config_manager->saveConfig();
                                }
                            if (shelll->jsonDoc.operator[]("presets").is<std::string>()) {
                                JsonObject presets = shelll->getSuperCube()->config_manager->getConfig()["light_presets"][shelll->jsonDoc.operator[](
                                        "presets").as<std::string>()];
                                shelll->jsonDoc.operator[]("r") = presets["r"];
                                shelll->jsonDoc.operator[]("g") = presets["g"];
                                shelll->jsonDoc.operator[]("b") = presets["b"];
                                shelll->jsonDoc.operator[]("bright") = presets["bright"];
                            }
                            Adafruit_NeoPixel *stripasd = new Adafruit_NeoPixel(25,
                                                                                pinMap[shelll->jsonDoc.operator[](
                                                                                        "pin").as<int>()],
                                                                                NEO_GRB +
                                                                                NEO_KHZ800);
                            stripasd->begin();
                            for (JsonVariant v: shelll->jsonDoc.operator[]("num").as<JsonArray>()) {
                                if (v.as<int>())
                                    stripasd->setPixelColor(v.as<int>(),
                                                            stripasd->Color(
                                                                    shelll->jsonDoc.operator[]("r").as<int>() | 0,
                                                                    shelll->jsonDoc.operator[]("g").as<int>() | 0,
                                                                    shelll->jsonDoc.operator[]("b").as<int>() |
                                                                    0));
                                else {
                                    std::regex pattern(R"((\d+)-(\d+))");
                                    std::smatch matches;
                                    std::string str = v.as<String>().c_str();
                                    if (std::regex_search(str, matches, pattern))
                                        for (int i = std::stoi(matches[1].str()); i <= std::stoi(matches[2].str()); i++)
                                            stripasd->setPixelColor(i, stripasd->Color(
                                                    shelll->jsonDoc.operator[]("r").as<int>() | 0,
                                                    shelll->jsonDoc.operator[]("g").as<int>() | 0,
                                                    shelll->jsonDoc.operator[]("b").as<int>() | 0));
                                }
                            }
                            stripasd->setBrightness(shelll->jsonDoc.operator[]("bright"));
                            stripasd->show();
                            shelll->println("Lighting up");
                            delete stripasd;
                        }
                    })));
}
