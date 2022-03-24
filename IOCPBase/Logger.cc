#include "Logger.h"

namespace phodobit {
	std::map<std::string, Logger*> * Logger::loggers = new std::map<std::string, Logger*>();

	Logger::Logger(std::string name, int defaultLogLevel) {
		this->name = name;
		this->logLevel = defaultLogLevel;
	}

	Logger* Logger::getLogger(const std::string name) {
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

	template<typename T>
	Logger& Logger::operator<<(T input)
	{
		if (currentLevel <= logLevel) {
			std::cout << input;
		}

		return *this;
	}
}