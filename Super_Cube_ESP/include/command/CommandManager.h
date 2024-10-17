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
    Shell(super_cube *superCube);

    Shell(super_cube *superCube, bool httpmode);

    void println(const char *message);

    void print(const char *message);

    void setup();

    super_cube *getSuperCube();

    String res;
    JsonDocument jsonDoc;
private:
    super_cube *superCube;
    bool httpMode = false;
};

class TYPE {
public:
    static const std::string STRING() { return "STRING"; }

    static const std::string BOOLEAN() { return "BOOLEAN"; }

    static const std::string INTEGER() { return "INTEGER"; }

    static const std::string NONE() { return "NONE"; }
};

class R : public std::map<std::string, std::variant<int, std::string, bool>> {
public:
    using Base = std::map<std::string, std::variant<int, std::string, bool>>;

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
    T get(const std::string &key) const {
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
            } else if (std::holds_alternative<std::string>(value)) {
                shell->println(
                        std::get<std::string>(value).c_str());
            } else if (std::holds_alternative<bool>(value)) {
                shell->println(std::to_string(std::get<bool>(value)).c_str());
            }
        }
    }
};

class CommandNode {
public:

    using CommandFunction = std::function<void(Shell *, const R &)>;

    explicit CommandNode(const std::string &name);

    explicit CommandNode(const std::string &name, const std::string &type);

    CommandNode *then(CommandNode *next);

    CommandNode *runs(CommandFunction func);

    const CommandNode *find_node(const std::vector<std::string> &path, R &context) const;

    void execute(Shell *shell, const R &context) const;

    const std::string &get_name() const;

    void printTree(int level = 0) const;

private:
    std::string name;
    std::string type;
    CommandFunction commandFunc;
    std::map<std::string, std::unique_ptr<CommandNode>> children;
};

class CommandRegistry {
public:
    void register_command(std::unique_ptr<CommandNode> root);

    void execute_command(Shell *shell, const std::string &input) const;

    void printCommandTree() const;

    // 修改后的工厂方法封装参数类型并返回 std::unique_ptr<CommandNode>
    CommandNode *Literal(const std::string &name) {
        return new CommandNode(name);
    }

    CommandNode *StringParam(const std::string &name) {
        return new CommandNode(name, TYPE::STRING());
    }

    CommandNode *BooleanParam(const std::string &name) {
        return new CommandNode(name, TYPE::BOOLEAN());
    }

    CommandNode *IntegerParam(const std::string &name) {
        return new CommandNode(name, TYPE::INTEGER());
    }

private:
    std::map<std::string, std::unique_ptr<CommandNode>> commands;
};

#endif // COMMAND_MANAGER_H
