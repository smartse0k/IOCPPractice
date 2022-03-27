#include <iostream>
#include <string>

#include "Logger.h"
#include "IOCP.h"

void startConsole() {
    phodobit::Logger* logger = phodobit::Logger::getLogger("console")->setLogLevel(phodobit::LogLevel::DEBUG);
    logger->debug() << "console()\n";

    char input[4096] = { 0 };
    while (true) {
        std::cout << "> ";
        std::cin.getline(input, 4096);
    }
}

int main(void)
{
    phodobit::Logger* logger = phodobit::Logger::getLogger("main")->setLogLevel(phodobit::LogLevel::DEBUG);
    logger->debug() << "main()\n";

    logger->info() << "IOCP Base starting\n";

    phodobit::IOCP *iocp = new phodobit::IOCP();
    iocp->initialize();
    iocp->bind(9999);
    iocp->listen();
    iocp->createAcceptThread(1);
    iocp->createWorkerThread(2);

    logger->info() << "IOCP Base started\n";

    startConsole();

    return 0;
}
