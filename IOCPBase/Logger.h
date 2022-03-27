#pragma once

#include <string>
#include <map>
#include <iostream>
#include <iomanip>

namespace phodobit {
    enum LogLevel {
        ERR = 0,
        WARN,
        INFO,
        DEBUG
    };

    class Logger {
    public:
        static Logger* getLogger(const std::string name);

        Logger* setLogLevel(int level);
        Logger& err();
        Logger& warn();
        Logger& info();
        Logger& debug();

        Logger& operator<<(Logger& logger);

        Logger& printByteArray(char* byteArray, unsigned int start, unsigned int end);

        template<typename T>
        Logger& operator<<(T input);

    private:
        static std::map<std::string, Logger*> *loggers;
        std::string name;
        int logLevel;
        int currentLevel;

        Logger(std::string name, int defaultLogLevel);
    };
}