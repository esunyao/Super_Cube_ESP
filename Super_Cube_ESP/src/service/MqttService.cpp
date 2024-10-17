//
// Created by Esuny on 2024/8/29.
//
#include <service/MqttService.h>

MqttService::MqttService(super_cube *superCube,
                         String server,
                         int port,
                         String clientId,
                         String username,
                         String password,
                         String topic) :
        superCube(superCube),
        server(server),
        port(port),
        clientId(clientId),
        username(username),
        password(password),
        topic(topic) { shell = new Shell(superCube, true); }

void MqttService::start() {
    mqttClient = std::make_unique<PubSubClient>(espClient);
    mqttClient->setServer(server.c_str(), port);
    mqttClient->setCallback([this](char *topic, byte *payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });
}

void MqttService::loop() {
    if (!mqttClient->connected()) {
        if (ShouldReconnect) {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                if (mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())) {
                    mqttClient->subscribe(topic.c_str());
                    ShouldReconnect = false;
                } else {
                    Serial.print("failed, rc=");
                    Serial.print(mqttClient->state());
                    Serial.println(" try again in 5 seconds");
                }
            }
        } else
            ShouldReconnect = true;
    }
    mqttClient->loop();
}

void MqttService::publishMessage(String message) {
    if (mqttClient->connected()) {
        mqttClient->publish(topic.c_str(), message.c_str());
    }
}

void MqttService::handleMessage(char *topic, byte *payload, unsigned int length) {
    String requestBody;
    for (unsigned int i = 0; i < length; i++) {
        requestBody += (char) payload[i];
    }

    JsonDocument jsonDoc = JsonDocument();
    DeserializationError error = deserializeJson(jsonDoc, requestBody);
    superCube->debugln("[MqttServer] Get request from: ", topic);
    if (!error) {
        superCube->debugln("[MqttServer][", topic, "] ", jsonDoc.as<String>());
        if (jsonDoc.operator[]("command").is<std::string>()) {
            superCube->hdebugln("[MqttServer] Command: ", jsonDoc.operator[]("command").as<String>());
            shell->setup();
            shell->jsonDoc = jsonDoc;
            superCube->command_registry->execute_command(shell,
                                                         shell->jsonDoc.operator[]("command").as<std::string>());
        }
    } else {
        superCube->mdebugln("[MqttServer] JSON deserialization failed: ");
        superCube->mdebugln(error.c_str());
        return;
    }
}