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

private:
    super_cube *superCube;
    std::unique_ptr<PubSubClient> mqttClient;
    String server;
    int port;
    String clientId;
    String username;
    String password;
    String topic;
    WiFiClient espClient;

    void reconnect();

    void callback(char *topic, byte *payload, unsigned int length);

    void loop();

    void publishMessage(String message);
};

#endif //SUPER_CUBE_ESP_MQTTSERVICE_H
