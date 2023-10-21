#include <main.h>
#include "Super_Cube_ESP_Server.h"


Logger logger;
flash_write FlashWrite;
int blockSize = 150;

// pinMap 灯光设定
std::map<String, int> pinMap = {
        {"pin1", D1},
        {"pin2", D2},
        {"pin3", D3},
        {"pin4", D4},
        {"pin5", D5},
        {"pin6", D6}
};

DynamicJsonDocument Config(2048);

WebSocketsClient wsClient;

// 创建WebSocket服务器对象
WebSocketsServer server(81);


using CallbackFunction = std::function<void(uint8_t, JsonDocument &)>;

std::map<uint8_t, IPAddress> WebSocketsClientMapList;
std::map<String, CallbackFunction> callbackMap;

void addCallback(const String &name, CallbackFunction callback) {
    callbackMap[name] = callback;
}

// 执行回调函数
void executeCallback(uint8_t num, const char *name, JsonDocument &msg) {
    auto it = callbackMap.find(String(name));
    if (it != callbackMap.end()) {
        it->second(num, msg);  // 调用回调函数
    } else {
        logger.debug("在执行callback函数时发生错误：未找到函数" + String(name));
    }
}

class CallBackFunctionClass;

// 创建Super_Cube_ESP主类
Super_Cube_ESP_Server Super_Cube_Server = Super_Cube_ESP_Server();

void setup() {
    Serial.begin(115200);
    logger.info("");
    logger.info(" .----------------.  .----------------.  .----------------.  .-----------------. .----------------.");
    logger.info("| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |");
    logger.info("| |  _________   | || |    _______   | || | _____  _____ | || | ____  _____  | || |  ____  ____  | |");
    logger.info(
            "| | |_   ___  |  | || |   /  ___  |  | || ||_   _||_   _|| || ||_   \\|_   _| | || | |_  _||_  _| | |");
    logger.info(
            "| |   | |_  \\_|  | || |  |  (__ \\_|  | || |  | |    | |  | || |  |   \\ | |   | || |   \\ \\  / /   | |");
    logger.info(
            "| |   |  _|  _   | || |   '.___`-.   | || |  | '    ' |  | || |  | |\\ \\| |   | || |    \\ \\/ /    | |");
    logger.info(
            "| |  _| |___/ |  | || |  |`\\____) |  | || |   \\ `--' /   | || | _| |_\\   |_  | || |    _|  |_    | |");
    logger.info(
            "| | |_________|  | || |  |_______.'  | || |    `.__.'    | || ||_____|\\____| | || |   |______|   | |");
    logger.info("| |              | || |              | || |              | || |              | || |              | |");
    logger.info("| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |");
    logger.info(" '----------------'  '----------------'  '----------------'  '----------------'  '----------------'");
    logger.info("Super_Cube_ESP_Server将在0.5s后启动");
    delay(500);
    EEPROM.begin(8192);
    if (EEPROM.read(0) != INIT_FLAG) {
        logger.warning("检测到系统处于初始化状态");
        logger.warning("开始初始化");
        EEPROM.write(0, INIT_FLAG);
        EEPROM.commit();
        EEPROM.end();
        FlashWrite.writeString(1, Default_Config);
        logger.success("已成功初始化系统");
        logger.info(FlashWrite.readString(1));
    }

    addCallbackToMap();
    Super_Cube_Server._on_start();

//    EEPROM.commit();
//    EEPROM.end();
}

String Serial_String = "";

void loop() {
    if(Config["WiFi"]["Websocket"]["Server"])
        server.loop();
    if(Config["WiFi"]["Websocket"]["Client"])
        wsClient.loop();
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();  // 读取一个字符
        Serial_String += receivedChar;
        if (receivedChar == '\n') {
            logger.success("成功接收到: " + Serial_String);
            DynamicJsonDocument msg(2048);
            deserializeJson(msg, String(Serial_String));
            String ReDeJsonDocument;
            serializeJson(msg, ReDeJsonDocument);
            logger.success("解析为: " + ReDeJsonDocument);
            executeCallback(-1, msg["command"], msg);
            Serial_String = "";
        }
    }
}

/*

Callback

*/
class CallBackFunctionClass {
public:
    // 获取Config
    static void Get_Config(uint8_t num, JsonDocument &msg);

    static void Set_Config(uint8_t num, JsonDocument &msg);

    static void Restart(uint8_t num, JsonDocument &msg);

    static void Reset(uint8_t num, JsonDocument &msg);

    static void TurnLight(uint8_t num, JsonDocument &msg);

    static void Get_All_Client(uint8_t num, JsonDocument &msg);

    static void Broadcast(uint8_t num, JsonDocument &msg);

    static void Send(uint8_t num, JsonDocument &msg);

    static void WiFiOpen(uint8_t num, JsonDocument &msg);
};

CallBackFunctionClass CallBackFunctionClassN;

void addCallbackToMap() {
    addCallback("Get_Config", CallBackFunctionClassN.Get_Config);
    addCallback("Set_Config", CallBackFunctionClassN.Set_Config);
    addCallback("Restart", CallBackFunctionClassN.Restart);
    addCallback("Reset", CallBackFunctionClassN.Reset);
    addCallback("TurnLight", CallBackFunctionClassN.TurnLight);
    addCallback("Get_All_Client", CallBackFunctionClassN.Get_All_Client);
    addCallback("Broadcast", CallBackFunctionClassN.Broadcast);
    addCallback("Send", CallBackFunctionClassN.Send);
    addCallback("WiFiOpen", CallBackFunctionClassN.WiFiOpen);
}

void CallBackFunctionClass::WiFiOpen(uint8_t num, JsonDocument &msg) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(String(Config["WiFi"]["SoftAP"]["ssid"]), String(Config["WiFi"]["SoftAP"]["passwd"]));
}

void CallBackFunctionClass::Broadcast(uint8_t num, JsonDocument &msg) {
    for (const auto &entry: WebSocketsClientMapList) {
        uint8_t key = entry.first;
        if (num == -1)
            server.sendTXT(key, msg["info"].as<const char *>());
        else
            wsClient.sendTXT(msg["info"].as<const char *>());
    }
}

void CallBackFunctionClass::Send(uint8_t num, JsonDocument &msg) {
    if (num == -1)
        server.sendTXT((uint8_t) msg["num"], msg["info"].as<const char *>());
    else
        wsClient.sendTXT(msg["info"].as<const char *>());
}

void CallBackFunctionClass::Get_All_Client(uint8_t num, JsonDocument &msg) {
    {
        String result = "";
        DynamicJsonDocument res(1024);

        for (const auto &entry: WebSocketsClientMapList) {
            uint8_t key = entry.first;
            const IPAddress &ip = entry.second;
            res[int(key)] = ip;
            // 将每个客户端的信息添加到结果字符串中
            result += "Client ID " + String(key) + ": " + ip.toString() + "\n";
        }
        String ress;
        serializeJson(res, ress);
        logger.debug(result);
        if (num == -1)
        server.sendTXT(num, ress);
        else
            wsClient.sendTXT(ress);
    }
}

void CallBackFunctionClass::Get_Config(uint8_t num, JsonDocument &msg) {
    String result;
    serializeJson(Config, result);
    if (num == (uint8_t) -1) {
        logger.info(result);
        return;
    }
    if (num == -1)
    server.sendTXT(num, result);
    else
        wsClient.sendTXT(result);
}

void CallBackFunctionClass::Set_Config(uint8_t num, JsonDocument &msg) {
    Config = msg["new_Config"];
    Super_Cube_Server.Save_Config();
    logger.success("成功设置Config并保存");
}

void CallBackFunctionClass::Restart(uint8_t num, JsonDocument &msg) {
    logger.warning("Super_Cube_ESP_Server将会重启");
    ESP.restart();
}

void CallBackFunctionClass::Reset(uint8_t num, JsonDocument &msg) {
    EEPROM.begin(8192);
    for (int i = 0; i <= 8192; i++)
        EEPROM.write(i, 0);
    EEPROM.commit();
    EEPROM.end();
    logger.success("配置成功，将在下次启动时重置");
}

void CallBackFunctionClass::TurnLight(uint8_t num, JsonDocument &msg) {
    JsonObject step = msg["step"];

    if (msg["Save"] == true) {
        Config["LED"] = msg["step"];
        Super_Cube_Server.Save_Config();
        logger.success("成功设置Config并保存");
    }

    for (JsonPair element: step) {
        {
            JsonArray array = element.value().as<JsonArray>();
            String res1;
            serializeJson(array, res1);
            logger.debug(res1);
            logger.debug(String(pinMap[String(element.key().c_str())]));
            Adafruit_NeoPixel stripasd = Adafruit_NeoPixel(array[0]["NUMPIXELS"], pinMap[String(element.key().c_str())],
                                                           NEO_GRB + NEO_KHZ800);
            stripasd.begin();
            for (JsonObject obj: array) {
                String res;
                serializeJson(obj, res);
                logger.debug(res);
                String type = obj["type"];
                if (type.equals("clearall")) {
                    stripasd.clear();
                } else if (type.equals("set")) {
                    for (int i = int(obj["from"]); i <= int(obj["to"]); i++) {
                        stripasd.setPixelColor(i, stripasd.Color((uint8_t) obj["colour"][0], (uint8_t) obj["colour"][1],
                                                                 (uint8_t) obj["colour"][2]));
                    }
                    logger.debug("灯珠设定完毕");
                } else if (type.equals("show")) {
                    logger.debug("WS2812 show");
                    stripasd.show();
                }
            }
            // for (JsonPair element : step)
            //   Adafruit_NeoPixel stripasd = Adafruit_NeoPixel(NUMPIXELS, pinMap[String(element.key().c_str())], NEO_GRB + NEO_KHZ800);
            // if (element.value()["type"].equals("clearall")) {
            // }

            // logger.info("Key: " + String(element.key().c_str()) + ", Value: " + String(element.value().as<String>()));
        }
    }
}