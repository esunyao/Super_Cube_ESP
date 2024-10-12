//
// Created by Esuny on 2024/10/12.
//

#ifndef SUPER_CUBE_ESP_TOOLS_H
#define SUPER_CUBE_ESP_TOOLS_H

#include <super_cube.h>
#ifndef SIMPLE_COMMAND_BUILDER_H
#define SIMPLE_COMMAND_BUILDER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

// AbstractNode base class
class AbstractNode {
public:
    virtual ~AbstractNode() = default;
    virtual void add_child(std::unique_ptr<AbstractNode> child) = 0;
    virtual void print_tree(int depth = 0) const = 0;
};

// Literal class
class Literal : public AbstractNode {
    std::string name;
public:
    Literal(const std::string &name);
    void add_child(std::unique_ptr<AbstractNode> child) override;
    void print_tree(int depth = 0) const override;
};

// ArgumentNode class
class ArgumentNode : public AbstractNode {
    std::string name;
public:
    ArgumentNode(const std::string &name);
    void add_child(std::unique_ptr<AbstractNode> child) override;
    void print_tree(int depth = 0) const override;
};

// Type definition for command callback
using CommandCallback = std::function<void()>;

// SimpleCommandBuilder class
class SimpleCommandBuilder {
    std::unordered_map<std::string, CommandCallback> commands;
    std::unordered_map<std::string, std::function<std::unique_ptr<AbstractNode>(const std::string&)>> literals;
    std::unordered_map<std::string, std::function<std::unique_ptr<AbstractNode>(const std::string&)>> arguments;

public:
    void command(const std::string &command_str, CommandCallback callback);
    void arg(const std::string &arg_name, std::function<std::unique_ptr<AbstractNode>(const std::string&)> node_factory);
    void literal(const std::string &literal_name, std::function<std::unique_ptr<AbstractNode>(const std::string&)> node_factory = nullptr);
    std::vector<std::unique_ptr<AbstractNode>> build();
    void add_children_for(AbstractNode &parent_node);
    void print_tree();
};

 #endif // SIMPLE_COMMAND_BUILDER_H


#endif //SUPER_CUBE_ESP_TOOLS_H
