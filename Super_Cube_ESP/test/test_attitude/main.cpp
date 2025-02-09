//
// ESP8266 版本完整代码
// 所需外部库：
//   Arduino.h  (ESP8266 Arduino 核心)
//   wit_c_sdk.h (请确保安装并正确配置 wit_c_sdk 库)
//

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <stdio.h>
#include "wit_c_sdk.h"
// ...existing code...（删除 "driver/uart.h" 和 FreeRTOS 相关头文件）

// 定义传感器串口：ESP8266 使用 SoftwareSerial（TX: GPIO4, RX: GPIO5）
// 定义传感器串口：ESP8266 使用 SoftwareSerial（TX: GPIO4, RX: GPIO5）
SoftwareSerial sensorSerial(D7, D8);  // 注意：构造函数参数为(rx, tx)

// 定义缓冲区大小
#define BUF_SIZE 1024

// 定义数据更新标志位
#define ACC_UPDATE      0x01  // 加速度更新
#define GYRO_UPDATE     0x02  // 陀螺仪更新
#define ANGLE_UPDATE    0x04  // 角度更新
#define MAG_UPDATE      0x08  // 磁场更新
#define READ_UPDATE     0x80  // 读取更新
static volatile char s_cDataUpdate = 0;
const uint32_t c_uiBaud[10] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};

// 假设 sReg 与各索引在 wit_c_sdk.h 中已定义，例如：AX, GX, Roll, HX 等
// ...existing function声明...
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void Delayms(uint16_t ucMs);
static void CmdProcess(char s_cCmd);
static void CopeCmdData(unsigned char ucData);
static void ShowHelp(void);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void AutoScanSensor(void);

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    // 发送数据至传感器串口
    sensorSerial.write(p_data, uiSize);
}

static void Delayms(uint16_t ucMs)
{
    delay(ucMs);
}

void CmdProcess(char s_cCmd)
{
    switch(s_cCmd)
    {
        case 'a':
            if(WitStartAccCali() != WIT_HAL_OK)
                Serial.printf("\r\nSet AccCali Error\r\n");
            break;
        case 'm':
            if(WitStartMagCali() != WIT_HAL_OK)
                Serial.printf("\r\nSet MagCali Error\r\n");
            break;
        case 'e':
            if(WitStopMagCali() != WIT_HAL_OK)
                Serial.printf("\r\nSet MagCali Error\r\n");
            break;
        case 'u':
            if(WitSetBandwidth(BANDWIDTH_5HZ) != WIT_HAL_OK)
                Serial.printf("\r\nSet Bandwidth Error\r\n");
            break;
        case 'U':
            if(WitSetBandwidth(BANDWIDTH_256HZ) != WIT_HAL_OK)
                Serial.printf("\r\nSet Bandwidth Error\r\n");
            break;
        case 'B':
            if(WitSetUartBaud(WIT_BAUD_115200) != WIT_HAL_OK)
                Serial.printf("\r\nSet Baud Error\r\n");
            else
                sensorSerial.begin(115200);
            break;
        case 'b':
            if(WitSetUartBaud(WIT_BAUD_9600) != WIT_HAL_OK)
                Serial.printf("\r\nSet Baud Error\r\n");
            else
                sensorSerial.begin(9600);
            break;
        case 'R':
            if(WitSetOutputRate(RRATE_10HZ) != WIT_HAL_OK)
                Serial.printf("\r\nSet Rate Error\r\n");
            break;
        case 'r':
            if(WitSetOutputRate(RRATE_1HZ) != WIT_HAL_OK)
                Serial.printf("\r\nSet Rate Error\r\n");
            break;
        case 'C':
            if(WitSetContent(RSW_ACC | RSW_GYRO | RSW_ANGLE | RSW_MAG) != WIT_HAL_OK)
                Serial.printf("\r\nSet RSW Error\r\n");
            break;
        case 'c':
            if(WitSetContent(RSW_ACC) != WIT_HAL_OK)
                Serial.printf("\r\nSet RSW Error\r\n");
            break;
        case 'h':
            ShowHelp();
            break;
        default:
            break;
    }
}

void CopeCmdData(unsigned char ucData)
{
    static unsigned char s_ucData[50];
    static unsigned char s_ucRxCnt = 0;

    s_ucData[s_ucRxCnt++] = ucData;
    if(s_ucRxCnt < 3) return; // 少于三个数据不处理
    if(s_ucRxCnt >= 50) s_ucRxCnt = 0;
    if(s_ucRxCnt >= 3)
    {
        if((s_ucData[1] == '\r' || s_ucData[1] == '\n') && (s_ucData[2] == '\r' || s_ucData[2] == '\n'))
        {
            CmdProcess(s_ucData[0]);
            memset(s_ucData, 0, 50);
            s_ucRxCnt = 0;
        }
        else
        {
            s_ucData[0] = s_ucData[1];
            s_ucData[1] = s_ucData[2];
            s_ucRxCnt = 2;
        }
    }
}

static void ShowHelp(void)
{
    // Serial.printf("\r\n************************ WIT_SDK_DEMO ************************\r\n");
    //    Serial.printf("UART SEND:a\\r\\n   Acceleration calibration.\r\n");
    //    Serial.printf("UART SEND:m\\r\\n   Magnetic field calibration, After calibration send: e\\r\\n to indicate the end\r\n");
    //    Serial.printf("UART SEND:U\\r\\n   Bandwidth increase.\r\n");
    //    Serial.printf("UART SEND:u\\r\\n   Bandwidth reduction.\r\n");
    //    Serial.printf("UART SEND:B\\r\\n   Baud rate increased to 115200.\r\n");
    //    Serial.printf("UART SEND:b\\r\\n   Baud rate reduction to 9600.\r\n");
    //    Serial.printf("UART SEND:R\\r\\n   The return rate increases to 10Hz.\r\n");
    //    Serial.printf("UART SEND:r\\r\\n   The return rate reduction to 1Hz.\r\n");
    //    Serial.printf("UART SEND:C\\r\\n   Basic return content: acceleration, angular velocity, angle, magnetic field.\r\n");
    //    Serial.printf("UART SEND:c\\r\\n   Return content: acceleration.\r\n");
    //    Serial.printf("UART SEND:h\\r\\n   help.\r\n");
    //    Serial.printf("******************************************************************************\r\n");
    Serial.printf("\r\n************************ WIT_SDK_DEMO ************************\r\n");
    Serial.printf("UART 发送:a\\r\\n   加速度校准。\r\n");
    Serial.printf("UART 发送:m\\r\\n   磁场校准，校准后发送: e\\r\\n 表示结束\r\n");
    Serial.printf("UART 发送:U\\r\\n   增加带宽。\r\n");
    Serial.printf("UART 发送:u\\r\\n   减少带宽。\r\n");
    Serial.printf("UART 发送:B\\r\\n   波特率增加到 115200。\r\n");
    Serial.printf("UART 发送:b\\r\\n   波特率减少到 9600。\r\n");
    Serial.printf("UART 发送:R\\r\\n   返回速率增加到 10Hz。\r\n");
    Serial.printf("UART 发送:r\\r\\n   返回速率减少到 1Hz。\r\n");
    Serial.printf("UART 发送:C\\r\\n   基本返回内容：加速度、角速度、角度、磁场。\r\n");
    Serial.printf("UART 发送:c\\r\\n   返回内容：加速度。\r\n");
    Serial.printf("UART 发送:h\\r\\n   帮助。\r\n");
    Serial.printf("******************************************************************************\r\n");
}

void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
    int i;
    for(i = 0; i < uiRegNum; i++)
    {
        switch(uiReg)
        {
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

void AutoScanSensor(void)
{
    int i, iRetry;
    for(i = 1; i < 10; i++)
    {
        sensorSerial.begin(c_uiBaud[i]);
        iRetry = 2;
        do
        {
            s_cDataUpdate = 0;
            WitReadReg(AX, 3);
            Delayms(100);
            if(s_cDataUpdate != 0)
            {
                Serial.printf("%lu baud find sensor\r\n\r\n", c_uiBaud[i]);
                ShowHelp();
                return;
            }
            iRetry--;
        } while(iRetry);
    }
    Serial.printf("can not find sensor\r\n");
    Serial.printf("please check your connection\r\n");
}

void setup(void)
{
    Serial.begin(115200);
    sensorSerial.begin(9600);
    // ...初始化其它外设...
    
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitSerialWriteRegister(SensorUartSend);
    WitRegisterCallBack(SensorDataUpdata);
    WitDelayMsRegister(Delayms);
    Serial.printf("\r\n********************** wit-motion normal example  ************************\r\n");
    // 传感器波特率固定，不需要扫描
    Serial.printf("Sensor fixed at 9600 baud, please check your connection if no data arrives.\r\n");
    // ...existing code...
}

void loop(void)
{
    // 读取传感器数据
    while(sensorSerial.available())
    {
        unsigned char ucTemp = sensorSerial.read();
        WitSerialDataIn(ucTemp);
    }
    // 读取命令输入
    while(Serial.available())
    {
        unsigned char c = Serial.read();
        CopeCmdData(c);
    }

    // 处理数据更新（和原 while(1) 循环中的处理逻辑相同）
    if(s_cDataUpdate)
    {
        float fAcc[3], fGyro[3], fAngle[3];
        int i;
        for(i = 0; i < 3; i++)
        {
            fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
            fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
            fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
        }
        if(s_cDataUpdate & ACC_UPDATE)
        {
            Serial.printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
            s_cDataUpdate &= ~ACC_UPDATE;
        }
        if(s_cDataUpdate & GYRO_UPDATE)
        {
            Serial.printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
            s_cDataUpdate &= ~GYRO_UPDATE;
        }
        if(s_cDataUpdate & ANGLE_UPDATE)
        {
            Serial.printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
            s_cDataUpdate &= ~ANGLE_UPDATE;
        }
        if(s_cDataUpdate & MAG_UPDATE)
        {
            Serial.printf("mag:%d %d %d\r\n", sReg[HX], sReg[HY], sReg[HZ]);
            s_cDataUpdate &= ~MAG_UPDATE;
        }
    }
    // 简单延时避免循环过快
    delay(10);
}

// ...existing code (其它函数不变)...
