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
        topic(topic) {}

void MqttService::start() {
    mqttClient = std::make_unique<PubSubClient>(espClient);
    mqttClient->setServer(server.c_str(), port);
    mqttClient->setCallback([this](char *topic, byte *payload, unsigned int length) {
        this->callback(topic, payload, length);
    });
}

void MqttService::loop() {
    if (!mqttClient->connected()) {
        reconnect();
    }
    mqttClient->loop();
}

void MqttService::reconnect() {
    while (!mqttClient->connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())) {
            Serial.println("connected");
            mqttClient->subscribe(topic.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient->state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void MqttService::publishMessage(String message) {
    if (mqttClient->connected()) {
        mqttClient->publish(topic.c_str(), message.c_str());
    }
}

void MqttService::callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char) payload[i];
    }
    Serial.println(message);

    // You can handle the message here, for example:
    // superCube->handleMessage(topic, message);
}