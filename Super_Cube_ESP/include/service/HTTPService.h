//
// Created by Esuny on 2024/8/29.
//

#ifndef SUPER_CUBE_ESP_HTTPSERVICE_H
#define SUPER_CUBE_ESP_HTTPSERVICE_H


#include <super_cube.h>
#include <ESP8266WebServer.h>

class HttpServer {

public:
    HttpServer(super_cube *superCube);

    void start();

    void handleClient();

private:
    void handleRoot();

    void handleNotFound();

    ESP8266WebServer httpServer;
    super_cube *superCube;
};


#endif //SUPER_CUBE_ESP_HTTPSERVICE_H
