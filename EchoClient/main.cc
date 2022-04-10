#include <WinSock2.h>
#include <cstring>
#include <iostream>
#include <string>
#include <WS2tcpip.h>

#include <Packet.h>

SOCKET s;

void init() {
    WSAData wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        throw "WSAStartup failed.";
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        throw "socket failed.";
    }

    std::cout << "Initialized\n";
}

void connect(char *ip, unsigned short port) {
    SOCKADDR_IN sockaddr;
    std::memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &sockaddr.sin_addr);
    sockaddr.sin_port = htons(port);

    int ret = connect(s, (SOCKADDR*)&sockaddr, sizeof(sockaddr));
    if (ret == SOCKET_ERROR) {
        throw "connect failed.";
    }

    std::cout << "Connected\n";
}

void loop() {
    while (true) {
        std::string input;

        std::cout << "Client>";

        std::getline(std::cin, input);

        phodobit::Packet outPacket;
        outPacket.write(input);

        // << Send

        unsigned short length = sizeof(unsigned short) + outPacket.getLength();
        char bLength[2] = { 0, };
        std::memcpy(bLength, &length, sizeof(unsigned short));

        send(s, bLength, sizeof(unsigned short), 0);
        send(s, outPacket.getData(), outPacket.getLength(), 0);

        // Send >>

        // << Receive

        char bRecvLength[2];
        recv(s, bRecvLength, sizeof(unsigned short), 0);

        unsigned short receiveLength;
        std::memcpy(&receiveLength, bRecvLength, sizeof(unsigned short));

        char* receiveData = new char[receiveLength];
        recv(s, receiveData, receiveLength, 0);

        // Receive >>

        phodobit::Packet* inPacket = phodobit::Packet::createFromByteArray(receiveData, 0, receiveLength);
        std::string output;
        inPacket->read(output);

        delete[] receiveData;

        std::cout << "Server>" << output << "\n";
    }
}

int main(void) {
    char ip[20] = "127.0.0.1";

    init();
    connect(ip, 9999);
    loop();

    return 0;
}