
//
// Created by Esuny on 2024/8/27.
//
// Include necessary headers
#include "command/CommandManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

// 实现 Shell 类
void Shell::println(const char *message) {
    // 打印消息到命令行
    if (isNetworkFlag())
        res += (String) message + '\n';
    superCube->serial->println(message);
}

void Shell::print(const char *message) {
    // 打印消息到命令行
    if (isNetworkFlag())
        res += message;
    superCube->serial->print(message);
}

super_cube *Shell::getSuperCube() {
    return this->superCube;
}

Shell::Shell(super_cube *superCube) : superCube(superCube), flag(Flags::CONSOLE) {}

Shell::Shell(super_cube *superCube, Flags flag) : superCube(superCube), flag(flag) {}

void Shell::setup() {
    res = "";
}

void Shell::setFlag(Flags flagType) {
    flag = flagType;
}

bool Shell::isNetworkFlag() {
    return flag != Flags::CONSOLE;
}

bool Shell::isFlag(Shell::Flags flagType) {
    return flag == flagType;
}

// CommandNode 类实现
CommandNode::CommandNode(const std::string &name) : name(name), commandFunc(nullptr), type(TYPE::NONE()) {}

CommandNode::CommandNode(const std::string &name, const std::string &type) : name(name), type(type),
                                                                             commandFunc(nullptr) {}

CommandNode::CommandNode() {

}

CommandNode *CommandNode::then(CommandNode *next) {
    children[next->get_name()] = std::unique_ptr<CommandNode>(next);
    return this;
}

CommandNode *CommandNode::runs(CommandFunction func) {
    commandFunc = std::move(func);
    return this;
}

const CommandNode *CommandNode::find_node(const std::vector<std::string> &path, R &context) const {
    if (path.empty()) {
        return this;
    }

    // Check for a literal match
    auto it = children.find(path[0]);
    if (it != children.end()) {
        std::vector<std::string> subPath(path.begin() + 1, path.end());
        return it->second->find_node(subPath, context);
    }

    // Check for specific parameter types based on registered commands
    for (const auto &child: children) {
        const std::string &childName = child.first;
        const std::string &childType = child.second->type;

        // Check for IntegerParam registration
        if (childType == TYPE::INTEGER() && std::all_of(path[0].begin(), path[0].end(), ::isdigit)) {
            context[childName] = std::stoi(path[0]);
            return child.second->find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
        }

        // Check for BooleanParam registration
        if (childType == TYPE::BOOLEAN() && (path[0] == "true" || path[0] == "false")) {
            context[childName] = (path[0] == "true");
            return child.second->find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
        }

        // Check for StringParam registration
        if (childType == TYPE::STRING()) {
            context[childName] = path[0];
            return child.second->find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
        }
    }

    // Default case if no matches found
    context[name] = path[0];
    return find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
}


void
CommandNode::execute(Shell *shell, const R &context) const {
    if (commandFunc) {
        commandFunc(shell, context);
    } else {
        shell->println("Error: No function to execute.");
    }
}

const std::string &CommandNode::get_name() const {
    return name;
}

void CommandNode::printTree(int level) const {
    // Print the current node with indentation based on the level
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
    if (level > 0) {
        std::cout << "|-";
    }
    std::cout << name << std::endl;

    // Recursively print the children nodes
    for (const auto &child: children) {
        child.second->printTree(level + 1);
    }
}

// CommandRegistry 类实现


void CommandRegistry::printCommandTree() const {
    for (const auto &command: commands) {
        command.second->printTree();
    }
}

void CommandRegistry::register_command(std::unique_ptr<CommandNode> root) {
    commands[root->get_name()] = std::move(root);
}

std::unique_ptr<Shell> CommandRegistry::execute_command(std::unique_ptr<Shell> shell, const std::string &input) const {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (!tokens.empty()) {
        auto it = commands.find(tokens[0]);
        if (it != commands.end()) {
            std::vector<std::string> subPath(tokens.begin() + 1, tokens.end());
            R context;
            const CommandNode *node = it->second->find_node(subPath, context);

            if (node) {
                node->execute(shell.get(), context);
            } else {
                shell->println("Error: Command not found or invalid parameters.");
            }
        } else {
            shell->println("Error: Command not found.");
        }
    }
    return shell;
}