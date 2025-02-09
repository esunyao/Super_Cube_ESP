//
// Created by Esuny on 2024/11/16.
//
#include "service/AttitudeService.h"
#include "Wire.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "I2Cdev.h"
#include <Arduino.h>

volatile char AttitudeService::s_cDataUpdate = 0;
AttitudeService *AttitudeService::instance = nullptr;

AttitudeService::AttitudeService(super_cube *cube) : superCube(cube) {
    if (instance == nullptr) {
        instance = this;
    } else {
        delete this;
    }
}

AttitudeService::~AttitudeService() {
    if (instance == this) {
        instance = nullptr;
        delete instance;
    }
    mpu.reset();
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
            {7, D7},
            {8, D8},
    };
    if (superCube->config_manager->getConfig()["Attitude"]["MODE"].as<String>() == "MPU6050") {
        mpu.reset();
        mpu = std::make_unique<MPU6050>();
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
    } else if (superCube->config_manager->getConfig()["Attitude"]["MODE"].as<String>() == "JY901L") {
        sensorSerial.reset();
        sensorSerial = std::make_unique<SoftwareSerial>(
                pinMap[superCube->config_manager->getConfig()["Attitude"]["RX"].as<int>()],
                pinMap[superCube->config_manager->getConfig()["Attitude"]["TX"].as<int>()]);
        superCube->serial->println("[JY] 正在初始化 串口 设备...");
        sensorSerial->begin(9600);
        superCube->debugln("[JY] 正在 初始化配置...");
        WitInit(WIT_PROTOCOL_NORMAL, 0x50);
        // 串口发送函数注册
        WitSerialWriteRegister(SensorUartSend);
        // 传感器数据更新监听
        WitRegisterCallBack(SensorDataUpdata);
        // 毫秒级延时函数注册
        WitDelayMsRegister(Delayms);
        consoleMode = true;
    }
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
    if (devStatus == 0) {       // 修改：从指针比较改为解引用比较
        mpu->CalibrateAccel(100);  // 校准时间：生成偏移量并校准我们的MPU6050
        mpu->CalibrateGyro(100);
        superCube->serial->println("[MPU] 这些是活动偏移量: ");
        mpu->PrintActiveOffsets();
        superCube->serial->println("[MPU] 启用DMP...");   //打开DMP
        mpu->setDMPEnabled(true);

        DMPReady = true;      // 修改：对指针解引用赋值 true
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
    if (mpu->dmpGetCurrentFIFOPacket(FIFOBuffer.get())) {   // 修改：传入 FIFOBuffer.get()
        switch (modeMap[mode]) {
            case 1:
                mpu->dmpGetQuaternion(&q, FIFOBuffer.get());  // 修改：FIFOBuffer.get()
                jsdoc["quat"] = JsonVariant();
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
                mpu->dmpGetQuaternion(&q, FIFOBuffer.get());  // 修改：FIFOBuffer.get()
                mpu->dmpGetEuler(euler, &q);
                jsdoc["euler"] = JsonVariant();
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
                mpu->dmpGetQuaternion(&q, FIFOBuffer.get());  // 修改：FIFOBuffer.get()
                mpu->dmpGetGravity(&gravity, &q);
                mpu->dmpGetYawPitchRoll(ypr, &q, &gravity);
                jsdoc["ypr"] = JsonVariant();
                jsdoc["ypr"]["yaw"] = ypr[0] * 180 / M_PI;
                jsdoc["ypr"]["pitch"] = ypr[1] * 180 / M_PI;
                jsdoc["ypr"]["roll"] = ypr[2] * 180 / M_PI;
                if (out_put)
                    superCube->serial->println(
                            "[MPU] ypr\t" + String(ypr[0] * 180 / M_PI) + "\t" + String(ypr[1] * 180 / M_PI) + "\t" +
                            String(ypr[2] * 180 / M_PI));
                break;
            case 4:
                mpu->dmpGetQuaternion(&q, FIFOBuffer.get());  // 修改：FIFOBuffer.get()
                mpu->dmpGetAccel(&aa, FIFOBuffer.get());        // 修改：FIFOBuffer.get()
                mpu->dmpGetGravity(&gravity, &q);
                mpu->dmpGetLinearAccel(&aaReal, &aa, &gravity);
                jsdoc["areal"] = JsonVariant();
                jsdoc["areal"]["x"] = aaReal.x;
                jsdoc["areal"]["y"] = aaReal.y;
                jsdoc["areal"]["z"] = aaReal.z;
                if (out_put)
                    superCube->serial->println(
                            "[MPU] areal\t" + String(aaReal.x) + "\t" + String(aaReal.y) + "\t" + String(aaReal.z));
                break;
            case 5:
                mpu->getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
                jsdoc["motion"] = JsonVariant();
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

void AttitudeService::SensorUartSend(uint8_t *p_data, uint32_t uiSize) {
    AttitudeService::instance->sensorSerial->write(p_data, uiSize);
}

void AttitudeService::SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum) {
    int i;
    for (i = 0; i < uiRegNum; i++) {
        switch (uiReg) {
            // ...existing cases: AX/AY省略
            case AZ:
                s_cDataUpdate |= ACC_UPDATE;
                break;
                // ...existing cases: GX/GY省略
            case GZ:
                s_cDataUpdate |= GYRO_UPDATE;
                break;
                // ...existing cases: HX/HY省略
            case HZ:
                s_cDataUpdate |= MAG_UPDATE;
                break;
                // ...existing cases: Roll/Pitch省略
            case Yaw:
                s_cDataUpdate |= ANGLE_UPDATE;
                break;
            default:
                s_cDataUpdate |= READ_UPDATE;
                break;
        }
        uiReg++;
    }
}

void AttitudeService::Delayms(uint16_t ucMs) {
    delay(ucMs);
}

void AttitudeService::loop() {
    if (consoleMode) {
        while (sensorSerial->available()) {
            unsigned char ucTemp = sensorSerial->read();
            WitSerialDataIn(ucTemp);
        }
        if (s_cDataUpdate) {
            float fAcc[3], fGyro[3], fAngle[3];
            int i;
            for (i = 0; i < 3; i++) {
                // 乘以16.0f 将其标准化为与加速度相关的数值范围。
                fAcc[i] = sReg[AX + i] / 32768.0f * 16.0f;
                // 乘以2000.0f 将数据转换为度每秒（dps）。
                fGyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
                // 乘以180.0f 将角度从比率形式转换为实际的角度数
                fAngle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
            }
            if (s_cDataUpdate & ACC_UPDATE) {
                // 加速度
                Serial.printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
                s_cDataUpdate &= ~ACC_UPDATE;
            }
            if (s_cDataUpdate & GYRO_UPDATE) {
                // 角加速计
                Serial.printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
                s_cDataUpdate &= ~GYRO_UPDATE;
            }
            if (s_cDataUpdate & ANGLE_UPDATE) {
                // 滚动传感器
                Serial.printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
                s_cDataUpdate &= ~ANGLE_UPDATE;
            }
            if (s_cDataUpdate & MAG_UPDATE) {
                // 磁场
                Serial.printf("mag:%d %d %d\r\n", sReg[HX], sReg[HY], sReg[HZ]);
                s_cDataUpdate &= ~MAG_UPDATE;
            }
        }
    }
}

void AttitudeService::InitializeCommand() {
    superCube->command_registry->register_command(
            std::unique_ptr<CommandNode>(superCube->command_registry->Literal("Server_posture")->then(
                            superCube->command_registry->Literal("get")->runs(
                                    [this](std::unique_ptr<Shell> shelll, const R &context) {
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
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getDevStatus());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("getReadyStatus")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->getReadyStatus());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("ConnectionTest")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->ConnectionTest());
                                                         }))
                                                 ->then(superCube->command_registry->Literal("StartDmp")->runs(
                                                         [this](std::unique_ptr<Shell> shelll, const R &context) {
                                                             shelll->getSuperCube()->debugln("[MPU]",
                                                                                             shelll->getSuperCube()->attitudeService->StartDmp());
                                                         }))
            ));
}
