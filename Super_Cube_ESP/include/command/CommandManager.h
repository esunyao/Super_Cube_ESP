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


class CommandNode {
public:
    using R = const std::map<std::string, std::variant<int, std::string, bool>>;
    using CommandFunction = std::function<void(Shell *, R &)>;

    explicit CommandNode(const std::string &name);

    std::unique_ptr<CommandNode> then(std::unique_ptr<CommandNode> next);
    std::unique_ptr<CommandNode> runs(CommandFunction func);

    const CommandNode *find_node(const std::vector<std::string> &path, std::map<std::string, std::variant<int, std::string, bool>> &context) const;
    void execute(Shell *shell, R &context) const;

    const std::string &get_name() const;

    static std::string context_to_string(const R &context);
//    static bool context_to_boolean(const R &context);
//    static int context_to_int(const R &context);

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
    std::unique_ptr<CommandNode> Literal(const std::string &name) {
        return std::make_unique<CommandNode>(name);
    }

    std::unique_ptr<CommandNode> StringParam(const std::string &name) {
        return std::make_unique<CommandNode>(name);
    }

    std::unique_ptr<CommandNode> BooleanParam(const std::string &name) {
        return std::make_unique<CommandNode>(name);
    }

    std::unique_ptr<CommandNode> IntegerParam(const std::string &name) {
        return std::make_unique<CommandNode>(name);
    }

private:
    std::map<std::string, std::unique_ptr<CommandNode>> commands;
};

#endif // COMMAND_MANAGER_H
