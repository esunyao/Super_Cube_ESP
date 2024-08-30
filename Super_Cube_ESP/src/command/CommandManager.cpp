
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
    superCube->serial->println(message);
}

super_cube *Shell::getSuperCube() {
    return this->superCube;
}

Shell::Shell(super_cube *superCube) : superCube(superCube) {}

// CommandNode 类实现
CommandNode::CommandNode(const std::string &name) : name(name), commandFunc(nullptr) {}

CommandNode* CommandNode::then(CommandNode* next) {
    children[next->get_name()] = std::unique_ptr<CommandNode>(next);
    return this;
}

CommandNode* CommandNode::runs(CommandFunction func) {
    commandFunc = std::move(func);
    return this;
}

const CommandNode *CommandNode::find_node(const std::vector<std::string> &path,
                                          R &context) const {
    if (path.empty()) {
        return this;
    }

    auto it = children.find(path[0]);
    if (it != children.end()) {
        std::vector<std::string> subPath(path.begin() + 1, path.end());
        return it->second->find_node(subPath, context);
    }

    // Check for Integer type
    if (!path[0].empty() && std::all_of(path[0].begin(), path[0].end(), ::isdigit)) {
        context[name] = std::stoi(path[0]);
        return find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
    }

    // Check for Boolean type
    if (path[0] == "true" || path[0] == "false") {
        context[name] = (path[0] == "true");
        return find_node(std::vector<std::string>(path.begin() + 1, path.end()), context);
    }

    // Default to String type
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

// CommandRegistry 类实现
void CommandRegistry::register_command(std::unique_ptr<CommandNode> root) {
    commands[root->get_name()] = std::move(root);
}

void CommandRegistry::execute_command(Shell *shell, const std::string &input) const {
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
            CommandNode::R context;
            const CommandNode *node = it->second->find_node(subPath, context);

            if (node) {
                node->execute(shell, context);
            } else {
                shell->println("Error: Command not found or invalid parameters.");
            }
        } else {
            shell->println("Error: Command not found.");
        }
    }
}