//
// Created by Esuny on 2024/11/16.
//

#ifndef SUPER_CUBE_ESP_ATTITUDESERVICE_H
#define SUPER_CUBE_ESP_ATTITUDESERVICE_H

#include <super_cube.h>
#include "MPU6050_6Axis_MotionApps20.h"

class AttitudeService {
public:
    AttitudeService(super_cube *cube);

    ~AttitudeService();

    void setup();

    JsonDocument GetData(bool out_put, const String &mode);

    void OffsetSet(int16_t XG, int16_t YG, int16_t ZG, int16_t XA, int16_t YA, int16_t ZA);

    static AttitudeService *instance; // 静态实例指针
    bool ConnectionTest();

    bool getReadyStatus();

    bool getDevStatus();

    uint8_t InitDmp();

    bool StartDmp();

private:
    super_cube *superCube;
    MPU6050 *mpu;
    /*---MPU6050控制/状态变量---*/
    bool DMPReady = false;  // 如果DMP初始化成功，则设置为true
    uint8_t devStatus;      // 每次设备操作后的返回状态（0 = 成功，!0 = 错误）
    uint16_t packetSize;    // 预期的DMP数据包大小（默认是42字节）
    uint8_t FIFOBuffer[64]; // FIFO存储缓冲区
    Quaternion q;           // [w, x, y, z]         四元数容器

};

#endif //SUPER_CUBE_ESP_ATTITUDESERVICE_H
