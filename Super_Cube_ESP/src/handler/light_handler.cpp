//
// Created by Esuny on 2024/10/13.
//

#include "handler/LightHandler.h"


LightHandler::LightHandler(super_cube *superCube_) : superCube(superCube_) {
    shell = new Shell(superCube, true);
}

void LightHandler::lightInitiation() {
//    for (JsonVariant v : superCube->config_manager->getConfig()["light"].as<JsonArray>()) {
//        shell->jsonDoc->clear();
////        shell->jsonDoc->set(v.to<JsonDocument>())
//        // shell->jsonDoc->operator[]("command") = "Server_NeoPixel";
//        // superCube->command_registry->execute_command(shell, "Server_NeoPixel");
//    }
}
