//
// Created by Esuny on 2024/11/16.
//

#ifndef SUPER_CUBE_ESP_ATTITUDESERVICE_H
#define SUPER_CUBE_ESP_ATTITUDESERVICE_H

#include <super_cube.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include <wit_c_sdk.h>
#include <SoftwareSerial.h>
// JY
// 定义缓冲区大小
#define BUF_SIZE 1024

// 定义数据更新标志位
#define ACC_UPDATE      0x01  // 加速度更新
#define GYRO_UPDATE     0x02  // 陀螺仪更新
#define ANGLE_UPDATE    0x04  // 角度更新
#define MAG_UPDATE      0x08  // 磁场更新
#define READ_UPDATE     0x80  // 读取更新

class AttitudeService {
public:
    AttitudeService(super_cube *cube); // 私有构造函数
    static AttitudeService *instance;

    ~AttitudeService();

    void setup();

    JsonDocument GetData(bool out_put, const String &mode);


    // MPU6050
    void OffsetSet(int16_t XG, int16_t YG, int16_t ZG, int16_t XA, int16_t YA, int16_t ZA);

    bool ConnectionTest();

    bool getReadyStatus();

    bool getDevStatus();

    uint8_t InitDmp();

    bool StartDmp();

    void loop();

private:
    super_cube *superCube;
    bool consoleMode = false;
    // MPU
    std::unique_ptr<MPU6050> mpu;
    /*---MPU6050控制/状态变量---*/
    bool DMPReady = false;  // 如果DMP初始化成功，则设置为true
    uint8_t devStatus;      // 每次设备操作后的返回状态（0 = 成功，!0 = 错误）
    uint16_t packetSize;    // 预期的DMP数据包大小（默认是42字节）
    std::unique_ptr<uint8_t[]> FIFOBuffer; // FIFO存储缓冲区
    // JY
    std::unique_ptr<SoftwareSerial> sensorSerial;

    static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);

    static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);

    static void Delayms(uint16_t ucMs);

    // volatile 告知编译器该变量可能被程序之外的机制（如中断、多线程、硬件等）意外修改。
    // 禁止编译器对此变量进行优化（如缓存到寄存器），确保每次访问都直接读写内存。
    static volatile uint8_t s_cDataUpdate;

    void CmdProcess(char s_cCmd, std::unique_ptr<Shell> shell);

    float Acc[3], Gyro[3], Angle[3], Mag[3];
    // 共用类

    void InitializeCommand();

    void JYUpdate();
};

#endif //SUPER_CUBE_ESP_ATTITUDESERVICE_H
