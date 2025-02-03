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
    superCube->serial->println(ConnectionTest() ? "[MPU] MPU6050 连接成功" : "[MPU] MPU6050 连接失败");

    /* 初始化并配置DMP*/
    InitDmp();

    /* 在此处提供您的陀螺仪偏移量，按最小灵敏度缩放 */
    OffsetSet(0, 0, 0, 0, 0, 0);

    /* 确保它工作正常（如果是，则返回0） */
    StartDmp();
}

bool AttitudeService::ConnectionTest() {
    return mpu->testConnection();
}

bool AttitudeService::getReadyStatus() {
    return DMPReady;
}

bool AttitudeService::getDevStatus() {
    return devStatus;
}

uint8_t AttitudeService::InitDmp() {
    superCube->serial->println("[MPU] 初始化DMP...");
    devStatus = mpu->dmpInitialize();
    return devStatus;
}

bool AttitudeService::StartDmp() {
    if (devStatus == 0) {
        mpu->CalibrateAccel(100);  // 校准时间：生成偏移量并校准我们的MPU6050
        mpu->CalibrateGyro(100);
        superCube->serial->println("[MPU] 这些是活动偏移量: ");
        mpu->PrintActiveOffsets();
        superCube->serial->println("[MPU] 启用DMP...");   //打开DMP
        mpu->setDMPEnabled(true);

        DMPReady = true;
        packetSize = mpu->dmpGetFIFOPacketSize(); //获取预期的DMP数据包大小以供以后比较
        return true;
    } else {
        superCube->debugln(F("[MPU] DMP初始化失败（代码 "), devStatus == 1 ? F("初始化内存失败") : F("DMP配置更新失败"),
                           F(")")); //打印错误代码
        // 1 = 初始内存加载失败
        // 2 = DMP配置更新失败
        return false;
    }
}

void AttitudeService::OffsetSet(int16_t XG, int16_t YG, int16_t ZG, int16_t XA, int16_t YA, int16_t ZA) {
    /* 在此处提供您的陀螺仪偏移量，按最小灵敏度缩放 */
    mpu->setXGyroOffset(XG);
    mpu->setYGyroOffset(YG);
    mpu->setZGyroOffset(ZG);
    mpu->setXAccelOffset(XA);
    mpu->setYAccelOffset(YA);
    mpu->setZAccelOffset(ZA);
}

JsonDocument AttitudeService::GetData(bool out_put, const String &mode) {
    JsonDocument jsdoc = JsonDocument();
    std::map<String, int> modeMap = {
            {"OUTPUT_READABLE_QUATERNION",   1},       // 四元数分量
            {"OUTPUT_READABLE_EULER",        2},       // 欧拉角
            {"OUTPUT_READABLE_YAWPITCHROLL", 3},       // 偏航/俯仰/滚转角
            {"OUTPUT_READABLE_REALACCEL",    4},       // 输出去除重力后的加速度分量
            {"OUTPUT_INIT",                  5}
    };
    Quaternion q;           // [w, x, y, z]         四元数容器
    VectorInt16 aa;         // [x, y, z]            加速度传感器测量值
    VectorInt16 aaReal;     // [x, y, z]            去除重力的加速度传感器测量值
    VectorFloat gravity;    // [x, y, z]            重力向量
    float euler[3];         // [psi, theta, phi]    欧拉角容器
    float ypr[3];           // [yaw, pitch, roll]   偏航/俯仰/滚转容器和重力向量
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    if (mpu->dmpGetCurrentFIFOPacket(FIFOBuffer)) {
        switch (modeMap[mode]) {
            case 1:
                mpu->dmpGetQuaternion(&q, FIFOBuffer);
                jsdoc["quat"] = JsonDocument();
                jsdoc["quat"]["w"] = q.w;
                jsdoc["quat"]["x"] = q.x;
                jsdoc["quat"]["y"] = q.y;
                jsdoc["quat"]["z"] = q.z;
                if (out_put)
                    superCube->serial->println(
                            "[MPU] quat\t" + String(q.w) + "\t" + String(q.x) + "\t" + String(q.y) + "\t" +
                            String(q.z));
                break;
            case 2:
                mpu->dmpGetQuaternion(&q, FIFOBuffer);
                mpu->dmpGetEuler(euler, &q);
                jsdoc["euler"] = JsonDocument();
                jsdoc["euler"]["psi"] = euler[0] * 180 / M_PI;
                jsdoc["euler"]["theta"] = euler[1] * 180 / M_PI;
                jsdoc["euler"]["phi"] = euler[2] * 180 / M_PI;
                superCube->serial->println("[MPU] 0: " + String(euler[0] * 180 / M_PI));
                superCube->serial->println("[MPU] 1: " + String(euler[1] * 180 / M_PI));
                superCube->serial->println("[MPU] 2: " + String(euler[2] * 180 / M_PI));
                if (out_put)
                    superCube->serial->println(
                            "[MPU] euler\t" + String(euler[0] * 180 / M_PI) + "\t" + String(euler[1] * 180 / M_PI) +
                            "\t" +
                            String(euler[2] * 180 / M_PI));
                break;
            case 3:
                mpu->dmpGetQuaternion(&q, FIFOBuffer);
                mpu->dmpGetGravity(&gravity, &q);
                mpu->dmpGetYawPitchRoll(ypr, &q, &gravity);
                jsdoc["ypr"] = JsonDocument();
                jsdoc["ypr"]["yaw"] = ypr[0] * 180 / M_PI;
                jsdoc["ypr"]["pitch"] = ypr[1] * 180 / M_PI;
                jsdoc["ypr"]["roll"] = ypr[2] * 180 / M_PI;
                if (out_put)
                    superCube->serial->println(
                            "[MPU] ypr\t" + String(ypr[0] * 180 / M_PI) + "\t" + String(ypr[1] * 180 / M_PI) + "\t" +
                            String(ypr[2] * 180 / M_PI));
                break;
            case 4:
                mpu->dmpGetQuaternion(&q, FIFOBuffer);
                mpu->dmpGetAccel(&aa, FIFOBuffer);
                mpu->dmpGetGravity(&gravity, &q);
                mpu->dmpGetLinearAccel(&aaReal, &aa, &gravity);
                jsdoc["areal"] = JsonDocument();
                jsdoc["areal"]["x"] = aaReal.x;
                jsdoc["areal"]["y"] = aaReal.y;
                jsdoc["areal"]["z"] = aaReal.z;
                if (out_put)
                    superCube->serial->println(
                            "[MPU] areal\t" + String(aaReal.x) + "\t" + String(aaReal.y) + "\t" + String(aaReal.z));
                break;
            case 5:
                mpu->getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
                jsdoc["motion"] = JsonDocument();
                jsdoc["motion"]["ax"] = ax;
                jsdoc["motion"]["ay"] = ay;
                jsdoc["motion"]["az"] = az;
                jsdoc["motion"]["gx"] = gx;
                jsdoc["motion"]["gy"] = gy;
                jsdoc["motion"]["gz"] = gz;
        }
    }
    return jsdoc;
}

