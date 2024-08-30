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

    void println(const char *message);

    super_cube *getSuperCube();

private:
    super_cube *superCube;
};

class RC : public std::map<std::string, std::variant<int, std::string, bool>> {
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

    template <typename T>
    T get(const std::string& key) const {
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


};

class CommandNode {
public:

    using R = RC;
    using CommandFunction = std::function<void(Shell *, const R &)>;

    explicit CommandNode(const std::string &name);

    CommandNode *then(CommandNode *next);

    CommandNode *runs(CommandFunction func);

    const CommandNode *find_node(const std::vector<std::string> &path, R &context) const;

    void execute(Shell *shell, const R &context) const;

    const std::string &get_name() const;

private:
    std::string name;
    CommandFunction commandFunc;
    std::map<std::string, std::unique_ptr<CommandNode>> children;
};

class CommandRegistry {
public:
    void register_command(std::unique_ptr<CommandNode> root);

    void execute_command(Shell *shell, const std::string &input) const;

    // 修改后的工厂方法封装参数类型并返回 std::unique_ptr<CommandNode>
    CommandNode *Literal(const std::string &name) {
        return new CommandNode(name);
    }

    CommandNode *StringParam(const std::string &name) {
        return new CommandNode(name);
    }

    CommandNode *BooleanParam(const std::string &name) {
        return new CommandNode(name);
    }

    CommandNode *IntegerParam(const std::string &name) {
        return new CommandNode(name);
    }

private:
    std::map<std::string, std::unique_ptr<CommandNode>> commands;
};

#endif // COMMAND_MANAGER_H
