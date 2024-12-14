//
// Created by Esuny on 2024/8/29.
//
#include <service/MqttService.h>
#include <queue>
#include <ArduinoJson.h>

std::queue<char*> messageQueue;
volatile bool isProcessing = false;

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
    shell = new Shell(superCube, false, true);
}

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
    processMessageQueue();
}

void MqttService::publishMessage(const String &message) {
    publishMessage(message, callback);
}

void MqttService::publishMessage(const String &message, String topic_pub) {
    if (mqttClient->connected()) {
        superCube->mdebugln("[MqttServer] MQTT client connected, publishing message.");
        superCube->mdebugln("[MqttServer] Publishing message: ", shell->res);
        superCube->mdebugln("[MqttServer] PublicTopic: ", topic_pub);
        mqttClient->setBufferSize(1038);
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
    if (isProcessing) {
        superCube->mdebugln("[MqttServer] Previous message is still being processed. Discarding new message.");
        return;
    }
    isProcessing = true;

    // 将消息复制到本地缓冲区
    char *message = (char *)malloc(length + 1);
    if (message == nullptr) {
        superCube->mdebugln("[MqttServer] Unable to allocate memory for message.");
        isProcessing = false;
        return;
    }
    memcpy(message, payload, length);
    message[length] = '\0';

    // 将消息添加到队列中
    messageQueue.push(message);
}

void MqttService::processMessageQueue() {
    while (!messageQueue.empty()) {
        char *message = messageQueue.front();
        messageQueue.pop();

        // 处理消息
        processMessage(message);

        // 释放内存
        free(message);
    }
    isProcessing = false;
}

void MqttService::processMessage(const char *message) {
    static unsigned long lastProcessTime = 0;
    unsigned long currentTime = millis();
    const unsigned long minInterval = 100; // 最小处理间隔，单位：毫秒

    if (currentTime - lastProcessTime < minInterval) {
        superCube->mdebugln("[MqttServer] Message processing rate limited.");
        return;
    }
    lastProcessTime = currentTime;

    StaticJsonDocument<1024> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, message);
    if (!error) {
        superCube->debugln("[MqttServer][", topic, "] ", jsonDoc.as<String>());
        // 后续代码保持不变
        if (jsonDoc["devices"].is<const char *>()) {
            const char *deviceId = jsonDoc["devices"];
            const char *configId = superCube->config_manager->getConfig()["ID"];
            if (strcmp(deviceId, configId) != 0) {
                superCube->mdebugln("[MqttServer] Device ID not match, ignore.");
                return;
            }
        }
        if (jsonDoc["command"].is<const char *>()) {
            const char *command = jsonDoc["command"];
            superCube->mdebugln("[MqttServer] Command: ", command);
            shell->setup();
            shell->jsonDoc = jsonDoc;
            { superCube->command_registry->execute_command(shell, command); }
            publishMessage(shell->res);
            shell->setup();
            // jsonDoc.clear(); // StaticJsonDocument 不需要手动清理
        }
    } else {
        superCube->mdebugln("[MqttServer] JSON deserialization failed: ", error.c_str());
    }
}

void MqttService::commandRegister() {
    superCube->command_registry->register_command(
            std::unique_ptr<CommandNode>(superCube->command_registry->Literal("Server_posture")->then(
                            superCube->command_registry->Literal("get")->runs([this](Shell *shelll, const R &context) {
                                bool out_put = false;
                                if (shelll->jsonDoc["out_put"].is<bool>())
                                    out_put = shelll->jsonDoc["out_put"];
                                JsonDocument data = shelll->getSuperCube()->attitudeService->GetData(out_put,
                                                                                                     shelll->jsonDoc["mode"].as<String>());
                                String dataStr;
                                serializeJson(data, dataStr);
                                if (shelll->getSuperCube())
                                    shelll->getSuperCube()->mqttService->publishMessage(dataStr,
                                                                                        shelll->getSuperCube()->config_manager->getConfig()["Mqtt"]["attitude_topic"].as<String>() +
                                                                                        shelll->getSuperCube()->config_manager->getConfig()["ID"].as<String>());
                                else
                                    shelll->println("only Support Mqtt");
                            }))
                                                 ->then(superCube->command_registry->Literal("getDevStatus")->runs(
                                                         [this](Shell *shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getDevStatus());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("getReadyStatus")->runs(
                                                         [this](Shell *shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getReadyStatus());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("ConnectionTest")->runs(
                                                         [this](Shell *shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->ConnectionTest());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("StartDmp")->runs(
                                                         [this](Shell *shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->StartDmp());
                                                         }))
            ));
}
