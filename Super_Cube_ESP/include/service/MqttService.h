//
// Created by Esuny on 2024/8/29.
//

#ifndef SUPER_CUBE_ESP_MQTTSERVICE_H
#define SUPER_CUBE_ESP_MQTTSERVICE_H

#include <PubSubClient.h>
#include <super_cube.h>

class MqttService {
public:
    MqttService(super_cube *superCube,
                String server,
                int port,
                String clientId,
                String username,
                String password,
                String topic,
                String callback);

    void start();

    std::unique_ptr<PubSubClient> mqttClient;

    void loop();

    void Connect_();

    void commandRegister();

    void publishMessage(const String &message, String topic_pub);

private:
    super_cube *superCube;

    String server;
    int port;
    String clientId;
    String username;
    String password;
    String topic;
    String callback;
    WiFiClient espClient;
    unsigned long previousMillis = 0;
    const long interval = 5000;
    bool ShouldReconnect = false;

    void handleMessage(char *topic, byte *payload, unsigned int length);

    void publishMessage(const String &message);

};

#endif //SUPER_CUBE_ESP_MQTTSERVICE_H
