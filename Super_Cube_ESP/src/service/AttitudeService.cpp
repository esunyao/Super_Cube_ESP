//
// Created by Esuny on 2024/11/16.
//
#include "service/AttitudeService.h"
#include "Wire.h"

AttitudeService::AttitudeService(super_cube *cube) : superCube(cube) {}

void AttitudeService::setup() {
    // 加入 I2C 总线（I2Cdev 库不会自动执行此操作）
    std::map<int, const uint8_t> pinMap = {
            {1, D1},
            {2, D2},
            {3, D3},
            {4, D4},
            {5, D5},
            {6, D6},
            {7, D7}
    };
    Wire.begin(pinMap[superCube->config_manager->getConfig()["Attitude"]["SDA"].as<int>()],
               pinMap[superCube->config_manager->getConfig()["Attitude"]["SCL"].as<int>()]);

    // 初始化设备
    superCube->serial->println("正在初始化 I2C 设备...");
//    accelgyro.initialize();

    // 验证连接
    superCube->serial->println("测试设备连接...");
//    superCube->serial->println(accelgyro.testConnection() ? "MPU6050 连接成功" : "MPU6050 连接失败");
}

void AttitudeService::update() {
    // 从传感器读取数据
    int16_t ax, ay, az, gx, gy, gz;
//    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // 打印数据到串口
    superCube->serial->print("a/g:\t");
    superCube->serial->print(ax);
    superCube->serial->print("\t");
    superCube->serial->print(ay);
    superCube->serial->print("\t");
    superCube->serial->print(az);
    superCube->serial->print("\t");
    superCube->serial->print(gx);
    superCube->serial->print("\t");
    superCube->serial->print(gy);
    superCube->serial->print("\t");
    superCube->serial->print(gz);
    superCube->serial->println();
}

void AttitudeService::getMotion6(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {
//    accelgyro.getMotion6(ax, ay, az, gx, gy, gz);
}

