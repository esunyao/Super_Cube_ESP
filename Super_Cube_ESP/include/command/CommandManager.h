//
// Created by Esuny on 2024/8/27.
//

#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <HardwareSerial.h>
#include "super_cube.h"

class super_cube;

// Declare the Shell class
class Shell {
public:
    Shell(super_cube *superCube, HardwareSerial *serial);

    void println(const char *message);

    super_cube *getSuperCube();

private:
    super_cube *superCube;
    HardwareSerial *serial;
};

// Define flash_string_vector as a vector of strings
using flash_string_vector = std::vector<std::string>;

class Command {
public:
    using CommandFunction = std::function<void(Shell *, const std::vector<std::string> &)>;
    using CompletionFunction = std::function<std::vector<std::string>(Shell *, const std::vector<std::string> &,
                                                                      const std::string &)>;

    // Default constructor
    Command() = default;

    Command(flash_string_vector name,
            flash_string_vector arguments,
            CommandFunction execute,
            CompletionFunction complete = [](Shell *, const std::vector<std::string> &,
                                             const std::string &) { return std::vector<std::string>{}; });

    void run(Shell *shell, const std::vector<std::string> &args) const;

    std::vector<std::string>
    get_completions(Shell *shell, const std::vector<std::string> &current_args, const std::string &next_arg) const;

    const flash_string_vector &get_name() const;

private:
    flash_string_vector name;
    flash_string_vector arguments;
    CommandFunction execute;
    CompletionFunction complete;
};

class CommandRegistry {
public:
    CommandRegistry(super_cube &superCube);

    void add_command(const Command &command);

    void execute_command(Shell *shell, const std::string &name, const std::vector<std::string> &arguments);

    std::vector<std::string>
    get_command_completions(Shell &shell, const std::string &name, const std::vector<std::string> &current_arguments,
                            const std::string &next_argument);

    void print_all_commands(Shell &shell);

private:
    std::map<std::string, Command> commands;
    super_cube &superCube;
};

#endif // COMMAND_MANAGER_H
