//
// Created by Esuny on 2024/10/12.
//

#ifndef SUPER_CUBE_ESP_TOOLS_H
#define SUPER_CUBE_ESP_TOOLS_H

#include <super_cube.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#ifndef SIMPLE_COMMAND_BUILDER_H
#define SIMPLE_COMMAND_BUILDER_H



namespace std {
    template<>
    struct hash<String> {
        std::size_t operator()(const String &s) const noexcept {
            // 使用 String 的内置 hashCode() 方法（如果有），或手动计算
            std::size_t hash = 0;
            for (int i = 0; i < s.length(); i++) {
                hash = hash * 31 + s[i]; // 简单哈希算法
            }
            return hash;
        }
    };
}

// AbstractNode base class
class AbstractNode {
public:
    virtual ~AbstractNode() = default;

    virtual void add_child(std::unique_ptr<AbstractNode> child) = 0;

    virtual void print_tree(int depth = 0) const = 0;
};

// Literal class
class Literal : public AbstractNode {
    String name;
public:
    Literal(const String &name);

    void add_child(std::unique_ptr<AbstractNode> child) override;

    void print_tree(int depth = 0) const override;
};

// ArgumentNode class
class ArgumentNode : public AbstractNode {
    String name;
public:
    ArgumentNode(const String &name);

    void add_child(std::unique_ptr<AbstractNode> child) override;

    void print_tree(int depth = 0) const override;
};

// Type definition for command callback
using CommandCallback = std::function<void()>;

// SimpleCommandBuilder class
class SimpleCommandBuilder {
    std::unordered_map<String, CommandCallback> commands;
    std::unordered_map<String, std::function<std::unique_ptr<AbstractNode>(const String &)>> literals;
    std::unordered_map<String, std::function<std::unique_ptr<AbstractNode>(const String &)>> arguments;

public:
    void command(const String &command_str, CommandCallback callback);

    void arg(const String &arg_name, std::function<std::unique_ptr<AbstractNode>(const String &)> node_factory);

    void literal(const String &literal_name,
                 std::function<std::unique_ptr<AbstractNode>(const String &)> node_factory = nullptr);

    std::vector<std::unique_ptr<AbstractNode>> build();

    void add_children_for(AbstractNode &parent_node);

    void print_tree();
};

#endif // SIMPLE_COMMAND_BUILDER_H


#endif //SUPER_CUBE_ESP_TOOLS_H
