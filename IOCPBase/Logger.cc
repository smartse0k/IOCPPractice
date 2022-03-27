#include "Logger.h"

namespace phodobit {
    std::map<std::string, Logger*> * Logger::loggers = nullptr;

    Logger::Logger(std::string name, int defaultLogLevel) {
        this->name = name;
        this->logLevel = defaultLogLevel;
    }

    Logger* Logger::getLogger(const std::string name) {
        if (loggers == nullptr) {
            loggers = new std::map<std::string, Logger*>();
        }

        std::map<std::string, Logger*>::iterator it = loggers->find(name);

        if (it == loggers->end()) {
            Logger* logger = new Logger(name, LogLevel::ERR);
            loggers->insert({ name, logger });

            return logger;
        }

        return it->second;
    }

    Logger* Logger::setLogLevel(int level) {
        logLevel = level;
        return this;
    }

    Logger& Logger::err() {
        currentLevel = LogLevel::ERR;
        return *this << "[ERROR] <" << name << "> ";
    }

    Logger& Logger::warn() {
        currentLevel = LogLevel::WARN;
        return *this << "[WARN] <" << name << "> ";
    }

    Logger& Logger::info() {
        currentLevel = LogLevel::INFO;
        return *this << "[INFO] <" << name << "> ";
    }

    Logger& Logger::debug() {
        currentLevel = LogLevel::DEBUG;
        return *this << "[DEBUG] <" << name << "> ";
    }

    Logger& Logger::operator<<(Logger& logger) {
        return logger;
    }

    Logger& Logger::printByteArray(char *byteArray, unsigned int start, unsigned int length) {
        *this << std::hex;

        unsigned int end = start + length - 1;
        for (unsigned int i = start; i <= end; i++) {
            *this << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(byteArray[i]);

            if (i < end) {
                *this << " ";
            }
        }

        *this << std::dec;

        return *this;
    }

    template<typename T>
    Logger& Logger::operator<<(T input)
    {
        if (currentLevel <= logLevel) {
            std::cout << input;
        }

        return *this;
    }
    template Logger& Logger::operator<<(int);
    template Logger& Logger::operator<<(unsigned int);
    template Logger& Logger::operator<<(short);
    template Logger& Logger::operator<<(unsigned short);
    template Logger& Logger::operator<<(float);
    template Logger& Logger::operator<<(double);
    template Logger& Logger::operator<<(long);
    template Logger& Logger::operator<<(unsigned long);
}