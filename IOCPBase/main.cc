#include <iostream>
#include <string>

#include "Logger.h"
#include "PacketProcessor.h"
#include "IOCP.h"

void startPacketProcessor() {
    phodobit::PacketProcessor::createPacketProcessor(4);
}

void startIOCP() {
    phodobit::IOCP* iocp = new phodobit::IOCP();
    iocp->initialize();
    iocp->bind(9999);
    iocp->listen();
    iocp->createAcceptThread(1);
    iocp->createWorkerThread(2);
}

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

    logger->info() << "PacketProcessor starting\n";
    startPacketProcessor();
    logger->info() << "PacketProcessor started\n";

    logger->info() << "IOCP starting\n";
    startIOCP();
    logger->info() << "IOCP started\n";

    startConsole();

    return 0;
}
