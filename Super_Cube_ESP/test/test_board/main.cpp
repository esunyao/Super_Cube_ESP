#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT); // 设置内置 LED 引脚为输出
}

void loop() {
    digitalWrite(LED_BUILTIN, LOW); // 打开 LED
    Serial.println("LED ON");
    delay(1000); // 等待 1 秒

    digitalWrite(LED_BUILTIN, HIGH); // 关闭 LED
    Serial.println("LED OFF");
    delay(1000); // 等待 1 秒
}