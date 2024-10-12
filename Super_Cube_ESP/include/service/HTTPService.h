//
// Created by Esuny on 2024/8/29.
//

#ifndef SUPER_CUBE_ESP_HTTPSERVICE_H
#define SUPER_CUBE_ESP_HTTPSERVICE_H


#include <super_cube.h>
#include <ESP8266WebServer.h>

class HttpServer {

public:
    HttpServer(super_cube *superCube, int port);

    void start();

    void handleClient();

protected:
    template<typename Func>
    void _running(Func func, bool running) {
        if (running)
            func();
    }

    void handleRoot();

    void handleNotFound();

    void handleData();

    void handleCmdCommandExecution();

    void handleCommandExecution();

    void commandRegister();

private:

    ESP8266WebServer httpServer;
    super_cube *superCube;
    Shell *shell;
};


#endif //SUPER_CUBE_ESP_HTTPSERVICE_H
