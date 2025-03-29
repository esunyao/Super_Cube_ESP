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
    mqttClient->setBufferSize(500);
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
                    mqttClient->unsubscribe(topic.c_str());
                    mqttClient->unsubscribe(
                            (topic + "/" + superCube->config_manager->getConfig()["ID"].as<String>()).c_str());
                    mqttClient->subscribe(topic.c_str());
                    mqttClient->subscribe(
                            (topic + "/" + superCube->config_manager->getConfig()["ID"].as<String>()).c_str());
                    ShouldReconnect = false;
                    superCube->setupModule();
                } else {
                    Serial.print("failed, rc=");
                    Serial.print(mqttClient->state());
                    Serial.println(" try again in 5 seconds");
                    superCube->releaseResource();
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
        superCube->mdebugln("[MqttServer] PublicTopic: ", topic_pub);
        const size_t maxPayloadSize = 300;
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
    JsonDocument jsonDoc = JsonDocument();
    superCube->debugln("[MqttServer] Get request from: ", topic);
    String requestBody;
    for (unsigned int i = 0; i < length; i++) {
        requestBody += (char) payload[i];
    }
    DeserializationError error = deserializeJson(jsonDoc, requestBody);
    if (!error) {
        superCube->debugln("[MqttServer][", topic, "] ", jsonDoc.as<String>());
        if (jsonDoc["devices"].is<String>())
            if (!(jsonDoc["devices"].as<String>() ==
                  superCube->config_manager->getConfig()["ID"].as<String>())) {
                superCube->mdebugln("[MqttServer] Device ID not match, ignore.");
                return;
            }
        if (jsonDoc.operator[]("command").is<String>()) {
            superCube->mdebugln("[MqttServer] Command: ", jsonDoc.operator[]("command").as<String>());
            std::unique_ptr<Shell> shell = std::make_unique<Shell>(superCube, Shell::Flags::MQTT);
            shell->setup();
            shell->jsonDoc = jsonDoc;
            if (strcmp(topic, superCube->config_manager->getConfig()["Mqtt"]["topic"].as<const char *>()) == 0)
                publishMessage(superCube->command_registry->execute_command(std::move(shell),
                                                                            jsonDoc.operator[](
                                                                                    "command").as<String>())->res);
            else
                publishMessage(superCube->command_registry->execute_command(std::move(shell),
                                                                            jsonDoc.operator[](
                                                                                    "command").as<String>())->res,
                               superCube->config_manager->getConfig()["Mqtt"]["callback_topic"].as<String>() + "/" +
                               superCube->config_manager->getConfig()["ID"].as<String>());
        }
    } else {
        superCube->mdebugln("[MqttServer] JSON deserialization failed: ");
        superCube->mdebugln(error.c_str());
        return;
    }
}

void MqttService::commandRegister() {

}
