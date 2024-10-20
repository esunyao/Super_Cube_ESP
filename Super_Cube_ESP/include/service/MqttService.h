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
                String topic);

    void start();
    std::unique_ptr<PubSubClient> mqttClient;

    void loop();

private:
    super_cube *superCube;
    Shell *shell;

    String server;
    int port;
    String clientId;
    String username;
    String password;
    String topic;
    WiFiClient espClient;
    unsigned long previousMillis = 0;
    const long interval = 5000;
    bool ShouldReconnect = false;

    void handleMessage(char *topic, byte *payload, unsigned int length);

    void publishMessage(String message);
};

#endif //SUPER_CUBE_ESP_MQTTSERVICE_H
