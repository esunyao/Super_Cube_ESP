//
// Created by Esuny on 2024/8/27.
//

#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <variant>
#include <super_cube.h>

class super_cube;

class Shell {
public:
    enum Flags : int {
        HTTP = 0x01,   // 二进制 0001
        MQTT = 0x02,  // 二进制 0010
        CONSOLE = 0x04,  // 二进制 0100
    };

    explicit Shell(super_cube *superCube);

    explicit Shell(super_cube *superCube, Flags flag);

    void setFlag(Flags flagType);

    bool isNetworkFlag();

    bool isFlag(Flags flagType);

    void println(const char *message);

    void print(const char *message);

    void setup();

    super_cube *getSuperCube();

    String res;
    JsonDocument jsonDoc;
private:
    super_cube *superCube;
    Flags flag;
};

class TYPE {
public:
    static const String STRING() { return "STRING"; }

    static const String BOOLEAN() { return "BOOLEAN"; }

    static const String INTEGER() { return "INTEGER"; }

    static const String NONE() { return "NONE"; }
};

class R : public std::map<String, std::variant<int, String, bool>> {
public:
    using Base = std::map<String, std::variant<int, String, bool>>;

    using Base::operator[];
    using Base::at;
    using Base::find;
    using Base::erase;
    using Base::insert;
    using Base::begin;
    using Base::end;
    using Base::size;
    using Base::empty;
    using Base::clear;

    template<typename T>
    T get(const String &key) const {
        auto it = this->find(key);
        if (it != this->end()) {
            // 使用 std::get_if 来检查类型是否匹配
            if (auto value = std::get_if<T>(&(it->second))) {
                return *value;  // 如果类型匹配，返回值
            }
        }
        // 返回类型T的默认值
        return T{};
    }

    void print(Shell *shell) const {
        for (const auto &[key, value]: *this) {
            shell->print("Key:");
            shell->print(key.c_str());
            shell->print(", Value: ");
            if (std::holds_alternative<int>(value)) {
                shell->println(std::to_string(std::get<int>(value)).c_str());
            } else if (std::holds_alternative<String>(value)) {
                shell->println(
                        std::get<String>(value).c_str());
            } else if (std::holds_alternative<bool>(value)) {
                shell->println(std::to_string(std::get<bool>(value)).c_str());
            }
        }
    }
};

class CommandNode {
public:

    using CommandFunction = std::function<void(Shell *, const R &)>;

    explicit CommandNode();

    explicit CommandNode(const String &name);

    explicit CommandNode(const String &name, const String &type);

    CommandNode *then(CommandNode *next);

    CommandNode *runs(CommandFunction func);

    const CommandNode *find_node(const std::vector<String> &path, R &context) const;

    void execute(Shell *shell, const R &context) const;

    const String &get_name() const;

    void printTree(int level = 0) const;

private:
    String name;
    String type;
    CommandFunction commandFunc;
    std::map<String, std::unique_ptr<CommandNode>> children;
};

class CommandRegistry {
public:
    void register_command(std::unique_ptr<CommandNode> root);

    std::unique_ptr<Shell> execute_command(std::unique_ptr<Shell> shell, const String &input) const;

    void printCommandTree() const;

    // 修改后的工厂方法封装参数类型并返回 std::unique_ptr<CommandNode>
    CommandNode *Literal(const String &name) {
        return new CommandNode(name);
    }

    CommandNode *StringParam(const String &name) {
        return new CommandNode(name, TYPE::STRING());
    }

    CommandNode *BooleanParam(const String &name) {
        return new CommandNode(name, TYPE::BOOLEAN());
    }

    CommandNode *IntegerParam(const String &name) {
        return new CommandNode(name, TYPE::INTEGER());
    }

    template<typename T>
    CommandNode *Param(const char *name) {
        if constexpr (std::is_same<T, String>::value) {
            return new CommandNode(name, TYPE::STRING());
        } else if constexpr (std::is_same<T, bool>::value) {
            return new CommandNode(name, TYPE::BOOLEAN());
        } else if constexpr (std::is_integral<T>::value) {
            return new CommandNode(name, TYPE::INTEGER());
        } else {
            static_assert(sizeof(T) == 0, "不支持的参数类型");
        }
    }


private:
    std::map<String, std::unique_ptr<CommandNode>> commands;
};

#endif // COMMAND_MANAGER_H
