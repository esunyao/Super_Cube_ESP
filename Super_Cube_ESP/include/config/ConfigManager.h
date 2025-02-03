//
// Created by Esuny on 2024/8/26.
//

#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <map>
#include <variant>
#include "ArduinoJson.h"
#include <memory>
#include <super_cube.h>

class ConfigManager;

// 前向声明 ConfigData
struct ConfigData;

// 定义 ConfigValue
struct ConfigValue {
    std::variant<bool, int, std::string, std::unique_ptr<ConfigData>> value;

    // 默认构造函数
    ConfigValue() = default;

    // 拷贝构造函数（深拷贝）
    ConfigValue(const ConfigValue &other);

    // 赋值运算符（深拷贝）
    ConfigValue &operator=(const ConfigValue &other);

    // 类型转换函数
    template<typename T>
    const T &as() const {
        return std::get<T>(value);
    }

    // 赋值运算符重载
    ConfigValue &operator=(bool b);

    ConfigValue &operator=(int i);

    ConfigValue &operator=(const std::string &s);

    ConfigValue &operator=(const char *s);

    ConfigValue &operator=(const ConfigData &d);

    // 支持链式访问的 operator[]
    ConfigValue &operator[](const std::string &key);

    // --------------------------------
    // 类型转换操作符
    // --------------------------------

    // 显式转换为 bool（仅当存储类型为 bool 时直接转换）
    explicit operator bool() const {
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }
        return 1;
    }

    // 显式转换为 int（支持 bool 和 double 的隐式转换）
    explicit operator int() const {
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
        } else if (std::holds_alternative<bool>(value)) {
            return static_cast<int>(std::get<bool>(value));
        }
        return 1;
    }

    // 隐式转换为 std::string（仅当存储类型为 string 时直接转换）
    operator std::string() const {
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }
        return nullptr;
    }

    // 隐式转换为 const char*（基于 std::string 转换）
    operator const char *() const {
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value).c_str();
        }
        return nullptr;
    }
};

// 定义 ConfigData
struct ConfigData {
    using Storage = std::map<std::string, ConfigValue>;
    Storage data;

    // 默认构造函数
    ConfigData() = default;

    // 拷贝构造函数（深拷贝）
    ConfigData(const ConfigData &other);

    // 赋值运算符（深拷贝）
    ConfigData &operator=(const ConfigData &other);

    // 支持通过键访问 ConfigValue
    ConfigValue &operator[](const std::string &key);

    const ConfigValue &operator[](const std::string &key) const;
};


class super_cube;

class CommandNode;

class ConfigManager {
public:
    static ConfigManager &getInstance(super_cube *superCube) {
        static ConfigManager instance(superCube); // 保证只会创建一次，并在第一次使用时初始化
        return instance;
    }

    static void transferConfigDataFromJson(const JsonVariant &jsonVar, ConfigData &dst);

    static void transferJsonDataFromConfig(const ConfigData &src, const JsonVariant &destination);

    explicit ConfigManager(super_cube *superCube);

    void initialize();

    void clear();

    void saveConfig();

    bool readConfig();

    bool validateConfig();

    ConfigData &getConfig();

    void command_initialize();
    String toString();

protected:
    CommandNode *_init_stringer(std::string node);

    CommandNode *_init_boolean(std::string node);

    CommandNode *_init_inter(std::string node);

    template<typename T>
    CommandNode *_init_generic(std::string node, std::function<void(ConfigValue &, T)> setter);

private:
    int eepromSize;
    std::unique_ptr<JsonDocument> configDoc;
    ConfigData configData;
    super_cube *superCube;

    void createDefaultConfig();

    void clearConfigDoc();

    std::unique_ptr<std::map<std::string, std::vector<std::string>>> requiredKeys =
            std::make_unique<std::map<std::string, std::vector<std::string>>>(
                    std::map<std::string, std::vector<std::string>>{
                            {"reset",         {}},
                            {"HTTPDEBUG",     {}},
                            {"MQTTDEBUG",     {}},
                            {"DEBUG",         {}},
                            {"ID",            {}},
                            {"Internet",      {"ssid",   "passwd"}},
                            {"http",          {"port"}},
                            {"Websocket",     {"ip",     "port"}},
                            {"Mqtt",          {"ip",     "port", "username", "password", "topic", "callback_topic", "attitude_topic", "autoReconnected"}},
                            {"Attitude",      {"enable", "SCL",  "SDA"}},
                            {"serverMode",    {}},
                            {"light",         {}},
                            {"light_presets", {}},
                    });

    void registerNodeCommands(const std::string &path, const ConfigData &data, CommandNode *parentNode);
};

#endif // EEPROM_UTILS_H