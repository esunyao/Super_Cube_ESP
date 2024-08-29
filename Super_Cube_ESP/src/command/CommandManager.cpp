
//
// Created by Esuny on 2024/8/27.
//
// Include necessary headers
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <command/CommandManager.h>
#include <HardwareSerial.h>

// 实现 Shell 类
void Shell::println(const char *message) {
    // 打印消息到命令行
    serial->println(message);
}

super_cube *Shell::getSuperCube() {
    return this->superCube;
}

Shell::Shell(super_cube *superCube, HardwareSerial *serial) : superCube(superCube), serial(serial) {}
// 实现 Command 类
Command::Command(flash_string_vector name,
                 flash_string_vector arguments,
                 CommandFunction execute,
                 CompletionFunction complete)
        : name(std::move(name)), arguments(std::move(arguments)), execute(std::move(execute)),
          complete(std::move(complete)) {}

// 运行命令
void Command::run(Shell *shell, const std::vector<std::string> &args) const {
    execute(shell, args);
}

// 获取命令补全建议
std::vector<std::string> Command::get_completions(Shell *shell, const std::vector<std::string> &current_args,
                                                  const std::string &next_arg) const {
    if (complete) {
        return complete(shell, current_args, next_arg);
    }
    return {};
}

// 获取命令名称
const flash_string_vector &Command::get_name() const {
    return name;
}

// 实现 CommandRegistry 类
CommandRegistry::CommandRegistry(super_cube &superCube) : superCube(superCube) {}

void CommandRegistry::add_command(const Command &command) {
    // 将命令添加到命令映射中
    commands[command.get_name()[0]] = command;
}

// 执行指定名称的命令
void
CommandRegistry::execute_command(Shell *shell, const std::string &name, const std::vector<std::string> &arguments) {
    auto it = commands.find(name);
    if (it != commands.end()) {
        it->second.run(shell, arguments);
    } else {
        shell->println("Command not found");
    }
}

// 获取命令补全建议
std::vector<std::string> CommandRegistry::get_command_completions(Shell &shell, const std::string &name,
                                                                  const std::vector<std::string> &current_arguments,
                                                                  const std::string &next_argument) {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.get_completions(&shell, current_arguments, next_argument);
    }
    return {};
}

// 打印所有命令
void CommandRegistry::print_all_commands(Shell &shell) {
    for (const auto &entry: commands) {
        shell.println(entry.first.c_str());
    }
}