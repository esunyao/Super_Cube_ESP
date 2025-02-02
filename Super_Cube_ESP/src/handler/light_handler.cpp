//
// Created by Esuny on 2024/10/13.
//

#include "handler/LightHandler.h"


LightHandler::LightHandler(super_cube *superCube_) : superCube(superCube_) {
}

void LightHandler::lightInitiation() {
    for (JsonVariant v: superCube->config_manager->getConfig()["light"].as<JsonArray>()) {
        std::unique_ptr<Shell> shell = std::make_unique<Shell>(superCube, false, false);
        shell->jsonDoc.clear();
        std::string buffer;
        serializeJson(v, buffer);
        deserializeJson(shell->jsonDoc, buffer);
        shell->jsonDoc["command"] = "Server_NeoPixel";
        superCube->command_registry->execute_command(std::move(shell), "Server_NeoPixel");
    }
}
