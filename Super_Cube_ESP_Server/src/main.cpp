#include <main.h>


// pinMap 灯光设定
std::map<String, int> pinMap = {
        { "pin1", D1 },
        { "pin2", D2 },
        { "pin3", D3 },
        { "pin4", D4 },
        { "pin5", D5 },
        { "pin6", D6 }
};

DynamicJsonDocument Config(2048);

// 创建WebSocket服务器对象
WebSocketsServer server(81);


void setup(){

}
void loop(){

}