//
// Created by Esuny on 2023/10/19.
//
/*
 .----------------.  .----------------.  .----------------.  .-----------------. .----------------.
| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |
| |  _________   | || |    _______   | || | _____  _____ | || | ____  _____  | || |  ____  ____  | |
| | |_   ___  |  | || |   /  ___  |  | || ||_   _||_   _|| || ||_   \|_   _| | || | |_  _||_  _| | |
| |   | |_  \_|  | || |  |  (__ \_|  | || |  | |    | |  | || |  |   \ | |   | || |   \ \  / /   | |
| |   |  _|  _   | || |   '.___`-.   | || |  | '    ' |  | || |  | |\ \| |   | || |    \ \/ /    | |
| |  _| |___/ |  | || |  |`\____) |  | || |   \ `--' /   | || | _| |_\   |_  | || |    _|  |_    | |
| | |_________|  | || |  |_______.'  | || |    `.__.'    | || ||_____|\____| | || |   |______|   | |
| |              | || |              | || |              | || |              | || |              | |
| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |
 '----------------'  '----------------'  '----------------'  '----------------'  '----------------'
*/
// https://blog.csdn.net/weixin_41565556/article/details/127232783
#include <main.h>

LoggerCls logger = LoggerCls();

DynamicJsonDocument Config(1024);
std::map<String, int> pinMap = {
        { "pin1", D1 },
        { "pin2", D2 },
        { "pin3", D3 },
        { "pin4", D4 },
        { "pin5", D5 },
        { "pin6", D6 }
};

// 设定callback
using CallbackFunction = std::function<void(uint8_t, JsonDocument&)>;

// 创建WebSocket服务器对象
WebSocketsServer server(81);

void writeStringToEEPROM(int startAddr, String data) {
    int length = data.length();
    for (int i = 0; i < length; i++) {
        EEPROM.write(startAddr + i, data[i]);
    }
}

String readStringFromEEPROM(int startAddr, int length) {
    String data = "";
    for (int i = 0; i < length; i++) {
        char c = EEPROM.read(startAddr + i);
        data += c;
    }
    return data;
}

/* 读取 */
String readString(int addr) {
    int numBlocks = EEPROM.read(addr);
    String data = "";
    for (int i = 0; i < numBlocks; i++) {
        String block = readStringFromEEPROM(addr + i * blockSize + 1, blockSize);
        data += block;
    }
    return data;
}

/* 写入 */
void writeString(int addr, String data) {
    int len = data.length();

    // 计算块的数量
    int numBlocks = data.length() / blockSize + 1;

    EEPROM.write(addr, numBlocks);
    // 逐个块地存储字符串
    for (int i = 0; i < numBlocks; i++) {
        String block = data.substring(i * blockSize, (i + 1) * blockSize);
        writeStringToEEPROM(addr + i * blockSize + 1, block);
    }
    logger.debug("已添加保存字符串 " + data);
    EEPROM.commit();  // 写入EEPROM并保存更改
}

/* Util */
void splitString(String input, char delimiter, String* parts, int maxParts) {
    int partCount = 0;
    int startIndex = 0;
    int endIndex = 0;

    while (endIndex >= 0 && partCount < maxParts - 1) {
        endIndex = input.indexOf(delimiter, startIndex);

        if (endIndex >= 0) {
            parts[partCount] = input.substring(startIndex, endIndex);
        } else {
            parts[partCount] = input.substring(startIndex);
        }

        startIndex = endIndex + 1;
        partCount++;
    }

    parts[partCount] = "";  // 在最后一个元素设置为空字符串
}

/* System */
// 加载/保存配置文件
void Load_Config() {
    logger.debug(readString(1));
    deserializeJson(Config, readString(1));
    logger.set_Debug(Config["Logger"]["Debug"]);
    String CONFIG_TEST;
    serializeJson(Config, CONFIG_TEST);
    logger.success("Config:" + String(CONFIG_TEST));
    logger.success("成功加载配置文件");
}
void Save_Config() {
    String CONFIG_STRING;
    serializeJson(Config, CONFIG_STRING);
    writeString(1, CONFIG_STRING);
    logger.success("成功保存配置文件");
}

// 创建一个std::map来存储回调函数
std::map<String, CallbackFunction> callbackMap;
std::map<uint8_t, IPAddress> WebSocketsClient;

// 添加回调函数到map中
void addCallback(const String& name, CallbackFunction callback) {
    callbackMap[name] = callback;
}

// 执行回调函数
void executeCallback(uint8_t num, const char* name, JsonDocument& msg) {
    auto it = callbackMap.find(String(name));
    if (it != callbackMap.end()) {
        it->second(num, msg);  // 调用回调函数
    } else {
        logger.debug("在执行callback函数时发生错误：未找到函数" + String(name));
    }
}

class CallBackFunctionClass;

// 将callback添加到CallbackFunction中
void addCallbackToMap();



// 设定WiFi
void Load_WiFi() {
//    WiFi.softAP(String(Config["WiFi"]["softAP"]["ssid"]), String(Config["WiFi"]["softAP"]["passwd"]));
//    // 设置静态IP地址
//    logger.success("热点开启成功");
    IPAddress staticIP;  // 设置静态IP地址
    IPAddress gateway;   // 设置网关
    IPAddress subnet;    // 设置子网掩码
//    if (staticIP.fromString(String(Config["WiFi"]["softAP"]["ip"])) && gateway.fromString(String(Config["WiFi"]["softAP"]["gateway"])) && subnet.fromString(String(Config["WiFi"]["softAP"]["subnet"]))) {
//        // WiFi.config(staticIP, gateway, subnet);
//        WiFi.softAPConfig(staticIP, staticIP, subnet);
//        logger.success("已设置热点的静态IP地址、网关和子网掩码");
//    } else {
//        logger.critical("IP地址格式错误");
//    }
//    IPAddress apIP = WiFi.softAPIP();
//    logger.success("IP:" + apIP.toString());

    WiFi.begin(String(Config["WiFi"]["Connect"]["ssid"]), String(Config["WiFi"]["Connect"]["passwd"]));
    for (int i = 0; i <= WIFIConnectTimeOut; i++) {
        if (WiFi.status() == WL_CONNECTED)
            break;
        if (i % 10 == 0)
            logger.warning("尝试连接中....");
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED) {
        logger.critical("WiFi连接失败或WiFi设置错误");
    } else {
        logger.success("网络已连接");
        if (staticIP.fromString(String(Config["WiFi"]["ip"])) && gateway.fromString(String(Config["WiFi"]["gateway"])) && subnet.fromString(String(Config["WiFi"]["subnet"]))) {
            WiFi.config(staticIP, gateway, subnet);
            // WiFi.softAPConfig(staticIP, gateway, subnet);
            logger.success("已设置网络的静态IP地址、网关和子网掩码");
        } else {
            logger.critical("IP地址格式错误");
        }
        IPAddress localIP = WiFi.localIP();
        logger.success("IP:" + localIP.toString());
    }
}
// Websocket设定
void Start_Websocket() {
    // 参考https://blog.csdn.net/yq543858194/article/details/109186301

    // 启动ws服务器
    server.begin();
    // 指定事件处理函数
    server.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        {
            if (type == WStype_CONNECTED) {
                // 若为客户端连接事件，显示提示信息
                logger.info("[" + String(num) + "] 连接到服务端");
                WebSocketsClient[num] = server.remoteIP(num);
            } else if (type == WStype_DISCONNECTED) {
                // 若为连接断开事件，显示提示信息
                logger.info("[" + String(num) + "] 断开了连接");
                WebSocketsClient.erase(num);
            } else if (type == WStype_TEXT) {
                // 接收来自客户端的信息（客户端FLASH按键状态），并控制LED的工作
                logger.debug("[" + String(num) + "] " + String((char*)payload));
                DynamicJsonDocument msg(2048);
                deserializeJson(msg, String((char*)payload));
                executeCallback(num, msg["command"], msg);
            }
        }
    });
}


void setup() {
    EEPROM.begin(8192);
    Serial.println("设定串口频率");
    Serial.begin(115200);
    Serial.println("");
    delay(1000);
    logger.info(" .----------------.  .----------------.  .----------------.  .-----------------. .----------------.");
    logger.info("| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |");
    logger.info("| |  _________   | || |    _______   | || | _____  _____ | || | ____  _____  | || |  ____  ____  | |");
    logger.info("| | |_   ___  |  | || |   /  ___  |  | || ||_   _||_   _|| || ||_   \\|_   _| | || | |_  _||_  _| | |");
    logger.info("| |   | |_  \\_|  | || |  |  (__ \\_|  | || |  | |    | |  | || |  |   \\ | |   | || |   \\ \\  / /   | |");
    logger.info("| |   |  _|  _   | || |   '.___`-.   | || |  | '    ' |  | || |  | |\\ \\| |   | || |    \\ \\/ /    | |");
    logger.info("| |  _| |___/ |  | || |  |`\\____) |  | || |   \\ `--' /   | || | _| |_\\   |_  | || |    _|  |_    | |");
    logger.info("| | |_________|  | || |  |_______.'  | || |    `.__.'    | || ||_____|\\____| | || |   |______|   | |");
    logger.info("| |              | || |              | || |              | || |              | || |              | |");
    logger.info("| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |");
    logger.info(" '----------------'  '----------------'  '----------------'  '----------------'  '----------------'");
    logger.info("Super_Cube_ESP_Server将在0.5s后启动");

    delay(500);

    // 是否为第一次启动
    if (EEPROM.read(0) != INIT_FLAG) {
        logger.warning("检测到系统处于初始化状态");
        logger.warning("开始初始化");
        EEPROM.write(0, INIT_FLAG);
        EEPROM.commit();
        writeString(1, Default_Config);
        logger.success("已成功初始化系统");
        logger.info(readString(1));
    }

    // 读取配置
    Load_Config();

    // 设置ESP8266工作模式为AP模式
    // WiFi.mode(WIFI);
    WiFi.mode(WIFI_AP_STA);

    // 启动热点/连接网络
    Load_WiFi();

    addCallbackToMap();

    Start_Websocket();
    EEPROM.end();
    logger.success("初始化完毕");

    logger.info("开始执行开机任务");
    {
        DynamicJsonDocument st(2048);
        st["step"] = Config["LED"];
        st["Save"] = false;
        executeCallback(0, "TurnLight", st);
    }
}

void loop() {
    server.loop();
}


/*

Callback

*/
class CallBackFunctionClass {
public:
    // 获取Config
    static void Get_Config(uint8_t num, JsonDocument& msg);
    static void Set_Config(uint8_t num, JsonDocument& msg);
    static void Restart(uint8_t num, JsonDocument& msg);
    static void Reset(uint8_t num, JsonDocument& msg);
    static void TurnLight(uint8_t num, JsonDocument& msg);
    static void Get_All_Client(uint8_t num, JsonDocument& msg);
    static void Broadcast(uint8_t num, JsonDocument& msg);
    static void Send(uint8_t num, JsonDocument& msg);
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
}

void CallBackFunctionClass::Broadcast(uint8_t num, JsonDocument& msg) {
    for (const auto& entry : WebSocketsClient) {
        uint8_t key = entry.first;
        server.sendTXT(key, msg["info"].as<const char*>());
    }
}

void CallBackFunctionClass::Send(uint8_t num, JsonDocument& msg) {
    server.sendTXT((uint8_t)msg["num"], msg["info"].as<const char*>());
}

void CallBackFunctionClass::Get_All_Client(uint8_t num, JsonDocument& msg) {
    {
        String result = "";
        DynamicJsonDocument res(1024);

        for (const auto& entry : WebSocketsClient) {
            uint8_t key = entry.first;
            const IPAddress& ip = entry.second;
            res[int(key)] = ip;
            // 将每个客户端的信息添加到结果字符串中
            result += "Client ID " + String(key) + ": " + ip.toString() + "\n";
        }
        String ress;
        serializeJson(res, ress);
        logger.debug(result);
        server.sendTXT(num, ress);
    }
}

void CallBackFunctionClass::Get_Config(uint8_t num, JsonDocument& msg) {
    String result;
    serializeJson(Config, result);
    server.sendTXT(num, result);
}

void CallBackFunctionClass::Set_Config(uint8_t num, JsonDocument& msg) {
    EEPROM.begin(8192);
    Config = msg["new_Config"];
    Save_Config();
    EEPROM.commit();
    EEPROM.end();
    logger.success("成功设置Config并保存");
}

void CallBackFunctionClass::Restart(uint8_t num, JsonDocument& msg) {
    logger.warning("Super_Cube_ESP_Server将会重启");
    ESP.restart();
}

void CallBackFunctionClass::Reset(uint8_t num, JsonDocument& msg) {
    EEPROM.begin(8192);
    EEPROM.write(0, 0);
    EEPROM.commit();
    EEPROM.end();
    logger.success("配置成功，将在下次启动时重置");
}

void CallBackFunctionClass::TurnLight(uint8_t num, JsonDocument& msg) {
    JsonObject step = msg["step"];

    if (msg["Save"] == true) {
        EEPROM.begin(8192);
        Config["LED"] = msg["step"];
        Save_Config();
        EEPROM.commit();
        EEPROM.end();
        logger.success("成功设置Config并保存");
    }

    for (JsonPair element : step) {
        {
            JsonArray array = element.value().as<JsonArray>();
            String res1;
            serializeJson(array, res1);
            logger.debug(res1);
            logger.debug(String(pinMap[String(element.key().c_str())]));
            Adafruit_NeoPixel stripasd = Adafruit_NeoPixel(array[0]["NUMPIXELS"], pinMap[String(element.key().c_str())], NEO_GRB + NEO_KHZ800);
            stripasd.begin();
            for (JsonObject obj : array) {
                String res;
                serializeJson(obj, res);
                logger.debug(res);
                String type = obj["type"];
                if (type.equals("clearall")) {
                    stripasd.clear();
                } else if (type.equals("set")) {
                    for (int i = int(obj["from"]); i <= int(obj["to"]); i++) {
                        stripasd.setPixelColor(i, stripasd.Color((uint8_t)obj["colour"][0], (uint8_t)obj["colour"][1], (uint8_t)obj["colour"][2]));
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

