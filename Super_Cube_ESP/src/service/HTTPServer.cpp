//
// Created by Esuny on 2024/8/29.
//
#include <service/HTTPService.h>


HttpServer::HttpServer(super_cube *superCube) : superCube(superCube), httpServer(
        superCube->config_manager->getConfig()["http"]["port"].as<int>()) {
}

void HttpServer::start() {
    httpServer.on("/", HTTP_GET, [this]() { handleRoot(); });
//    server.onNotFound([this]() { handleNotFound(); });

//    server.begin();
}

void HttpServer::handleClient() {
//    server.handleClient();
}

void HttpServer::handleRoot() {
    httpServer.send(200, "text/plain", "Hello, this is ESP8266!");
    superCube->debugln("asdfasdf");
}

void HttpServer::handleNotFound() {
//    server.send(404, "text/plain", "404: Not Found");
}