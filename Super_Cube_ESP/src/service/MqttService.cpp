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
        callback(callback) {
}

void MqttService::start() {
    mqttClient = std::make_unique<PubSubClient>(espClient);
    mqttClient->setServer(server.c_str(), port);
    mqttClient->setCallback([this](char *topic, byte *payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });
    commandRegister();
}

void MqttService::Connect_() {
    if (mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())) {
        mqttClient->subscribe(topic.c_str());
        mqttClient->subscribe((topic + "/" + superCube->config_manager->getConfig()["ID"].as<String>()).c_str());
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
    publishMessage(message, callback);
}

void MqttService::publishMessage(const String &message, String topic_pub) {
    if (mqttClient->connected()) {
        superCube->mdebugln("[MqttServer] MQTT client connected, publishing message.");
        superCube->mdebugln("[MqttServer] Publishing message: ", message);
        superCube->mdebugln("[MqttServer] PublicTopic: ", topic_pub);
        mqttClient->setBufferSize(1034);
        const size_t maxPayloadSize = 1024;
        size_t messageLength = message.length();
        size_t offset = 0;
        int part = 0;
        int totalParts = (messageLength + maxPayloadSize - 1) / maxPayloadSize;

        while (offset < messageLength) {
            String chunk = message.substring(offset, offset + maxPayloadSize);
            String payload = String(part) + "/" + String(totalParts) + ":" + chunk;
            bool result = mqttClient->publish(topic_pub.c_str(), payload.c_str());
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
        if (jsonDoc["devices"].is<std::string>())
            if (!(jsonDoc["devices"].as<std::string>() ==
                  superCube->config_manager->getConfig()["ID"].as<std::string>())) {
                superCube->mdebugln("[MqttServer] Device ID not match, ignore.");
                return;
            }
        if (jsonDoc.operator[]("command").is<std::string>()) {
            superCube->mdebugln("[MqttServer] Command: ", jsonDoc.operator[]("command").as<String>());
            std::unique_ptr<Shell> shell = std::make_unique<Shell>(superCube, false, true);
            shell->setup();
            shell->jsonDoc = jsonDoc;
            publishMessage(superCube->command_registry->execute_command(std::move(shell),
                                                                        jsonDoc.operator[](
                                                                                "command").as<std::string>())->res);
        }
    } else {
        superCube->mdebugln("[MqttServer] JSON deserialization failed: ");
        superCube->mdebugln(error.c_str());
        return;
    }
}

void MqttService::commandRegister() {

}
