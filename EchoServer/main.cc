#include <iostream>
#include <PacketProcessor.h>

#include "EchoServer.h"

EchoServer echoServer;

void startConsole()
{
    char input[4096] = { 0, };

    while (true) {
        std::cout << "prompt>";
        std::cin.getline(input, 4096);
    }
}

int main(void)
{
    phodobit::PacketProcessor::createPacketProcessor(4);

    echoServer.startServer();
    startConsole();

    return 0;
}
