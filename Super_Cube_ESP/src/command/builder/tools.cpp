//
// Created by Esuny on 2024/10/12.
//
#include "command/builder/tools.h"
// Implementation of Literal class
Literal::Literal(const String &name) : name(name) {}

void Literal::add_child(std::unique_ptr<AbstractNode> child) {
    // Implement child management (e.g., store children in a vector or map)
}

void Literal::print_tree(int depth) const {
    std::cout << std::string(depth, ' ') << "Literal: " << name.c_str() << "\n";
}

// Implementation of ArgumentNode class
ArgumentNode::ArgumentNode(const String &name) : name(name) {}

void ArgumentNode::add_child(std::unique_ptr<AbstractNode> child) {
    // Implement child management (e.g., store children in a vector or map)
}

void ArgumentNode::print_tree(int depth) const {
    std::cout << std::string(depth, ' ') << "Argument: " << name.c_str() << "\n";
}

// Implementation of SimpleCommandBuilder methods

void SimpleCommandBuilder::command(const String &command_str, CommandCallback callback) {
    commands[command_str] = callback;
}

void SimpleCommandBuilder::arg(const String &arg_name, std::function<std::unique_ptr<AbstractNode>(const String&)> node_factory) {
    arguments[arg_name] = node_factory;
}

void SimpleCommandBuilder::literal(const String &literal_name, std::function<std::unique_ptr<AbstractNode>(const String&)> node_factory) {
    if (node_factory == nullptr) {
        node_factory = [](const String &name) {
            return std::make_unique<Literal>(name);
        };
    }
    literals[literal_name] = node_factory;
}

std::vector<std::unique_ptr<AbstractNode>> SimpleCommandBuilder::build() {
    std::vector<std::unique_ptr<AbstractNode>> built_nodes;
    for (const auto &cmd : commands) {
        auto root = std::make_unique<Literal>(cmd.first);
        built_nodes.push_back(std::move(root));  // Add children logic here if needed
    }
    return built_nodes;
}

void SimpleCommandBuilder::add_children_for(AbstractNode &parent_node) {
    auto built_nodes = build();
    for (auto &node : built_nodes) {
        parent_node.add_child(std::move(node));
    }
}

void SimpleCommandBuilder::print_tree() {
    auto built_nodes = build();
    for (const auto &node : built_nodes) {
        node->print_tree();
    }
}