//
// Created by Esuny on 2023/10/21.
//

#include <main.h>

#ifndef SUPER_CUBE_ESP_SERVER_LOGGER_H
#define SUPER_CUBE_ESP_SERVER_LOGGER_H

class Logger {
public:
    template<typename Args>
    void info(Args val);
    template<typename Args>
    void success(Args val);
    template<typename Args>
    void critical(Args val);
    template<typename Args>
    void warning(Args val);
    template<typename Args>
    void debug(Args val);

    void set_Debug(bool debug);

private:
    bool DEBUG = false;
};


#endif //SUPER_CUBE_ESP_SERVER_LOGGER_H
