////
//// Created by Esuny on 2024/12/14.
////
///*
//  MPU6050 DMP6
//
//  数字运动处理器（DMP）执行复杂的运动处理任务。
//  - 融合加速度计、陀螺仪和外部磁力计的数据（如果有），补偿各个传感器的噪声和误差。
//  - 检测特定类型的运动，无需微控制器持续监控原始传感器数据。
//  - 减少微处理器的工作负载。
//  - 输出处理后的数据，如四元数、欧拉角和重力向量。
//
//  代码包括自动校准和偏移生成任务。提供不同的输出格式。
//
//  此代码与茶壶项目兼容，使用茶壶输出格式。
//
//  电路：除了连接3.3v、GND、SDA和SCL外，此草图依赖于MPU6050的INT引脚连接到Arduino的外部中断#0引脚。
//
//  如果使用DMP 6.12固件版本，由于FIFO结构变化，茶壶处理示例可能会损坏。
//
//  在此处查找完整的MPU6050库文档：
//  https://github.com/ElectronicCats/mpu6050/wiki
//
//*/
//
//#include "I2Cdev.h"
//#include "MPU6050_6Axis_MotionApps20.h"
////#include "MPU6050_6Axis_MotionApps612.h" // 取消注释此库以使用DMP 6.12，并注释掉上面的库。
//
///* MPU6050默认I2C地址是0x68*/
//MPU6050 mpu;
////MPU6050 mpu(0x69); //用于AD0高
////MPU6050 mpu(0x68, &Wire1); //用于AD0低，但使用第二个Wire（TWI/I2C）对象。
//
///* 输出格式定义-------------------------------------------------------------------------------------------
//- 使用"OUTPUT_READABLE_QUATERNION"以[w, x, y, z]格式输出四元数分量。四元数不会出现万向节锁问题，但在远程主机或软件环境（如Processing）中解析或处理效率较低。
//
//- 使用"OUTPUT_READABLE_EULER"以度为单位输出欧拉角，从FIFO中的四元数计算得出。欧拉角存在万向节锁问题。
//
//- 使用"OUTPUT_READABLE_YAWPITCHROLL"以度为单位输出偏航/俯仰/滚转角，从FIFO中的四元数计算得出。这需要计算重力向量。
//偏航/俯仰/滚转角存在万向节锁问题。
//
//- 使用"OUTPUT_READABLE_REALACCEL"输出去除重力后的加速度分量。加速度参考框架未补偿方向。+X始终为传感器的+X。
//
//- 使用"OUTPUT_READABLE_WORLDACCEL"输出去除重力并调整为世界参考框架的加速度分量。如果没有磁力计，偏航是相对的。
//
//- 使用"OUTPUT_TEAPOT"输出与InvenSense茶壶演示匹配的输出。
//-------------------------------------------------------------------------------------------------------------------------------*/
//#define OUTPUT_READABLE_YAWPITCHROLL
////#define OUTPUT_READABLE_QUATERNION
////#define OUTPUT_READABLE_EULER
////#define OUTPUT_READABLE_REALACCEL
////#define OUTPUT_READABLE_WORLDACCEL
////#define OUTPUT_TEAPOT
//
//int const INTERRUPT_PIN = 2;  // 定义中断#0引脚
//bool blinkState;
//
///*---MPU6050控制/状态变量---*/
//bool DMPReady = false;  // 如果DMP初始化成功，则设置为true
//uint8_t MPUIntStatus;   // 保存MPU的实际中断状态字节
//uint8_t devStatus;      // 每次设备操作后的返回状态（0 = 成功，!0 = 错误）
//uint16_t packetSize;    // 预期的DMP数据包大小（默认是42字节）
//uint8_t FIFOBuffer[64]; // FIFO存储缓冲区
//
///*---方向/运动变量---*/
//Quaternion q;           // [w, x, y, z]         四元数容器
//VectorInt16 aa;         // [x, y, z]            加速度传感器测量值
//VectorInt16 gy;         // [x, y, z]            陀螺仪传感器测量值
//VectorInt16 aaReal;     // [x, y, z]            去除重力的加速度传感器测量值
//VectorInt16 aaWorld;    // [x, y, z]            世界框架的加速度传感器测量值
//VectorFloat gravity;    // [x, y, z]            重力向量
//float euler[3];         // [psi, theta, phi]    欧拉角容器
//float ypr[3];           // [yaw, pitch, roll]   偏航/俯仰/滚转容器和重力向量
//
///*-InvenSense茶壶演示的包结构-*/
//uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };
//
///*------中断检测例程------*/
//volatile bool MPUInterrupt = false;     // 指示MPU6050中断引脚是否已高电平
//void DMPDataReady() {
//    MPUInterrupt = true;
//}
//
//void setup() {
//#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
//    Wire.begin();
//    Wire.setClock(400000); // 400kHz I2C时钟。如果编译有困难，请注释此行
//#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
//    Fastwire::setup(400, true);
//#endif
//
//    Serial.begin(115200); //115200是茶壶演示输出所需的
//    while (!Serial);
//
//    /*初始化设备*/
//    Serial.println(F("初始化I2C设备..."));
//    mpu.initialize();
//    pinMode(INTERRUPT_PIN, INPUT);
//
//    /*验证连接*/
//    Serial.println(F("测试MPU6050连接..."));
//    if(mpu.testConnection() == false){
//        Serial.println("MPU6050连接失败");
//        while(true);
//    }
//    else {
//        Serial.println("MPU6050连接成功");
//    }
//
//    /*等待串口输入*/
//    Serial.println(F("\n发送任意字符以开始: "));
//    while (Serial.available() && Serial.read()); // 清空缓冲区
//    while (!Serial.available());                 // 等待数据
//    while (Serial.available() && Serial.read()); // 再次清空缓冲区
//
//    /* 初始化并配置DMP*/
//    Serial.println(F("初始化DMP..."));
//    devStatus = mpu.dmpInitialize();
//
//    /* 在此处提供您的陀螺仪偏移量，按最小灵敏度缩放 */
//    mpu.setXGyroOffset(0);
//    mpu.setYGyroOffset(0);
//    mpu.setZGyroOffset(0);
//    mpu.setXAccelOffset(0);
//    mpu.setYAccelOffset(0);
//    mpu.setZAccelOffset(0);
//
//    /* 确保它工作正常（如果是，则返回0） */
//    if (devStatus == 0) {
//        mpu.CalibrateAccel(6);  // 校准时间：生成偏移量并校准我们的MPU6050
//        mpu.CalibrateGyro(6);
//        Serial.println("这些是活动偏移量: ");
//        mpu.PrintActiveOffsets();
//        Serial.println(F("启用DMP..."));   //打开DMP
//        mpu.setDMPEnabled(true);
//
//        /*启用Arduino中断检测*/
//        Serial.print(F("启用中断检测（Arduino外部中断 "));
//        Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
//        Serial.println(F(")..."));
//        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), DMPDataReady, RISING);
//        MPUIntStatus = mpu.getIntStatus();
//
//        /* 设置DMP就绪标志，以便主循环函数知道可以使用它 */
//        Serial.println(F("DMP就绪！等待第一次中断..."));
//        DMPReady = true;
//        packetSize = mpu.dmpGetFIFOPacketSize(); //获取预期的DMP数据包大小以供以后比较
//    }
//    else {
//        Serial.print(F("DMP初始化失败（代码 ")); //打印错误代码
//        Serial.print(devStatus);
//        Serial.println(F(")"));
//        // 1 = 初始内存加载失败
//        // 2 = DMP配置更新失败
//    }
//    pinMode(LED_BUILTIN, OUTPUT);
//}
//
//void loop() {
//    if (!DMPReady) return; // 如果DMP编程失败，则停止程序。
//
//    /* 从FIFO读取数据包 */
//    if (mpu.dmpGetCurrentFIFOPacket(FIFOBuffer)) { // 获取最新的数据包
//#ifdef OUTPUT_READABLE_YAWPITCHROLL
//        /* 以度为单位显示欧拉角 */
//        mpu.dmpGetQuaternion(&q, FIFOBuffer);
//        mpu.dmpGetGravity(&gravity, &q);
//        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
//        Serial.print("ypr\t");
//        Serial.print(ypr[0] * 180/M_PI);
//        Serial.print("\t");
//        Serial.print(ypr[1] * 180/M_PI);
//        Serial.print("\t");
//        Serial.println(ypr[2] * 180/M_PI);
//#endif
//
//#ifdef OUTPUT_READABLE_QUATERNION
//        /* 以易于矩阵形式显示四元数值：[w, x, y, z] */
//      mpu.dmpGetQuaternion(&q, FIFOBuffer);
//      Serial.print("quat\t");
//      Serial.print(q.w);
//      Serial.print("\t");
//      Serial.print(q.x);
//      Serial.print("\t");
//      Serial.print(q.y);
//      Serial.print("\t");
//      Serial.println(q.z);
//#endif
//
//#ifdef OUTPUT_READABLE_EULER
//        /* 以度为单位显示欧拉角 */
//      mpu.dmpGetQuaternion(&q, FIFOBuffer);
//      mpu.dmpGetEuler(euler, &q);
//      Serial.print("euler\t");
//      Serial.print(euler[0] * 180/M_PI);
//      Serial.print("\t");
//      Serial.print(euler[1] * 180/M_PI);
//      Serial.print("\t");
//      Serial.println(euler[2] * 180/M_PI);
//#endif
//
//#ifdef OUTPUT_READABLE_REALACCEL
//        /* 显示去除重力后的实际加速度 */
//      mpu.dmpGetQuaternion(&q, FIFOBuffer);
//      mpu.dmpGetAccel(&aa, FIFOBuffer);
//      mpu.dmpGetGravity(&gravity, &q);
//      mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
//      Serial.print("areal\t");
//      Serial.print(aaReal.x);
//      Serial.print("\t");
//      Serial.print(aaReal.y);
//      Serial.print("\t");
//      Serial.println(aaReal.z);
//#endif
//
//#ifdef OUTPUT_READABLE_WORLDACCEL
//        /* 显示初始世界框架加速度，去除重力并根据四元数的已知方向旋转 */
//      mpu.dmpGetQuaternion(&q, FIFOBuffer);
//      mpu.dmpGetAccel(&aa, FIFOBuffer);
//      mpu.dmpGetGravity(&gravity, &q);
//      mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
//      mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
//      Serial.print("aworld\t");
//      Serial.print(aaWorld.x);
//      Serial.print("\t");
//      Serial.print(aaWorld.y);
//      Serial.print("\t");
//      Serial.println(aaWorld.z);
//#endif
//
//#ifdef OUTPUT_TEAPOT
//        /* 以InvenSense茶壶演示格式显示四元数值 */
//      teapotPacket[2] = FIFOBuffer[0];
//      teapotPacket[3] = FIFOBuffer[1];
//      teapotPacket[4] = FIFOBuffer[4];
//      teapotPacket[5] = FIFOBuffer[5];
//      teapotPacket[6] = FIFOBuffer[8];
//      teapotPacket[7] = FIFOBuffer[9];
//      teapotPacket[8] = FIFOBuffer[12];
//      teapotPacket[9] = FIFOBuffer[13];
//      Serial.write(teapotPacket, 14);
//      teapotPacket[11]++; // PacketCount，有意在0xFF处循环
//#endif
//
//        /* 闪烁LED以指示活动 */
//        blinkState = !blinkState;
//        digitalWrite(LED_BUILTIN, blinkState);
//    }
//}
