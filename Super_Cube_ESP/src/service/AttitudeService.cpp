//
// Created by Esuny on 2024/11/16.
//
#include "service/AttitudeService.h"
#include "Wire.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "I2Cdev.h"
#include <Arduino.h>

AttitudeService *AttitudeService::instance = nullptr;

AttitudeService::AttitudeService(super_cube *cube) : superCube(cube) {
    mpu = new MPU6050();
    instance = this;
}

AttitudeService::~AttitudeService() {
    delete mpu;
}

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
    superCube->serial->println("[MPU] 正在初始化 I2C 设备...");
    mpu->initialize();

    // 验证连接
    superCube->serial->println("[MPU] 测试设备连接...");
    superCube->serial->println(mpu->testConnection() ? "[MPU] MPU6050 连接成功" : "[MPU] MPU6050 连接失败");

    /* 初始化并配置DMP*/
    superCube->serial->println("[MPU] 初始化DMP...");
    devStatus = mpu->dmpInitialize();

    /* 在此处提供您的陀螺仪偏移量，按最小灵敏度缩放 */
    OffsetSet(0, 0, 0, 0, 0, 0);

    /* 确保它工作正常（如果是，则返回0） */
    if (devStatus == 0) {
        mpu->CalibrateAccel(6);  // 校准时间：生成偏移量并校准我们的MPU6050
        mpu->CalibrateGyro(6);
        superCube->serial->println("[MPU] 这些是活动偏移量: ");
        mpu->PrintActiveOffsets();
        superCube->serial->println("[MPU] 启用DMP...");   //打开DMP
        mpu->setDMPEnabled(true);

        DMPReady = true;
        packetSize = mpu->dmpGetFIFOPacketSize(); //获取预期的DMP数据包大小以供以后比较
    } else {
        superCube->debugln(F("[MPU] DMP初始化失败（代码 "), devStatus == 1 ? F("初始化内存失败") : F("DMP配置更新失败"), F(")")); //打印错误代码
        // 1 = 初始内存加载失败
        // 2 = DMP配置更新失败
    }
}

void AttitudeService::update() {
    float euler[3];         // [psi, theta, phi]    欧拉角容器
    if (!DMPReady) return;
    if (mpu->dmpGetCurrentFIFOPacket(FIFOBuffer)) {
        mpu->dmpGetQuaternion(&q, FIFOBuffer);
        mpu->dmpGetEuler(euler, &q);
        superCube->debugln("[MPU] euler\t", euler[0] * 180/M_PI, "\t", euler[1] * 180/M_PI, "\t", euler[2] * 180/M_PI);
    }
}

void AttitudeService::getMotion6(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz) {
}

void AttitudeService::OffsetSet(int16_t XG, int16_t YG, int16_t ZG, int16_t XA, int16_t YA, int16_t ZA) {
    /* 在此处提供您的陀螺仪偏移量，按最小灵敏度缩放 */
    mpu->setXGyroOffset(XG);
    mpu->setYGyroOffset(YG);
    mpu->setZGyroOffset(ZG);
    mpu->setXAccelOffset(XA);
    mpu->setYAccelOffset(YA);
    instance = this;
    mpu->setZAccelOffset(ZA);
}

