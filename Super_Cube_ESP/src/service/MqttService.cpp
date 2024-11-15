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
                         String topic,
                         String callback) :
        superCube(superCube),
        server(server),
        port(port),
        clientId(clientId),
        username(username),
        password(password),
        topic(topic),
        callback(callback) { shell = new Shell(superCube, true); }

void MqttService::start() {
    mqttClient = std::make_unique<PubSubClient>(espClient);
    mqttClient->setServer(server.c_str(), port);
    mqttClient->setCallback([this](char *topic, byte *payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });
}

void MqttService::Connect_() {
    if (mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())) {
        mqttClient->subscribe(topic.c_str());
        superCube->mdebugln("[MqttServer] MQTT client connected and subscribed to topic: ", topic);
    } else {
        superCube->mdebugln("[MqttServer] MQTT client failed to connect, state: ", mqttClient->state());
    }
}

void MqttService::loop() {
    if (!mqttClient->connected() && superCube->config_manager->getConfig()["Mqtt"]["autoReconnected"]) {
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

void MqttService::publishMessage(const String &message) {
    if (mqttClient->connected()) {
        superCube->mdebugln("[MqttServer] MQTT client connected, publishing message.");
        superCube->mdebugln("[MqttServer] Publishing message: ", shell->res);
        superCube->mdebugln("[MqttServer] PublicTopic: ", callback);
        mqttClient->setBufferSize(1038);
        const size_t maxPayloadSize = 1024;
        size_t messageLength = message.length();
        size_t offset = 0;
        int part = 0;
        int totalParts = (messageLength + maxPayloadSize - 1) / maxPayloadSize;

        while (offset < messageLength) {
            String chunk = message.substring(offset, offset + maxPayloadSize);
            String payload = String(part) + "/" + String(totalParts) + ":" + chunk;
            bool result = mqttClient->publish(callback.c_str(), payload.c_str());
            if (result) {
                superCube->mdebugln("[MqttServer] Chunk published successfully: ", payload);
            } else {
                superCube->mdebugln("[MqttServer] Failed to publish chunk: ", payload);
                superCube->mdebugln("[MqttServer] MQTT publish failed, state: ", mqttClient->state());
                break;
            }
            offset += maxPayloadSize;
            part++;
        }

    } else {
        superCube->mdebugln("[MqttServer] MQTT client not connected, cannot publish message.");
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
            superCube->mdebugln("[MqttServer] Command: ", jsonDoc.operator[]("command").as<String>());
            shell->setup();
            shell->jsonDoc = jsonDoc;
            superCube->command_registry->execute_command(shell,
                                                         shell->jsonDoc.operator[]("command").as<std::string>());
            publishMessage(shell->res);
        }
    } else {
        superCube->mdebugln("[MqttServer] JSON deserialization failed: ");
        superCube->mdebugln(error.c_str());
        return;
    }
}