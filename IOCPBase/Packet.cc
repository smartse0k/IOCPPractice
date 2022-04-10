#include <cstring>

#include "Packet.h"
#include "Client.h"

namespace phodobit {
    Logger* Packet::logger = Logger::getLogger("Packet")->setLogLevel(LogLevel::DEBUG);

    Packet::Packet() {
        data = nullptr;
        length = 0;
        _length = 0;
        readOffset = 0;
    }

    Packet* Packet::createFromByteArray(char* byteArray, unsigned int start, unsigned short length) {
        logger->debug() << "createFromByteArray()\n";

        Packet* packet = new Packet();

        packet->data = new char[length];
        std::memcpy(packet->data, &byteArray[start], length);

        packet->length = length;
        packet->readOffset = 0;

        return packet;
    }

    // Read 관련은 PacketReader.cc에서 구현한다.
    // Write 관련은 PacketWriter.cc에서 구현한다.

    char* Packet::getData() {
        return data;
    }

    unsigned short Packet::getLength() {
        return length;
    }

    void Packet::printInfoToCLI() {
        logger->info() << "Packet Info...\n"
                       << "  length: " << length << "\n"
                       << "  data: ";
        logger->printByteArray(data, 0, length);
        *logger << "\n";
    }

    Packet::~Packet() {
        if (data != nullptr) {
            delete[] data;
        }
    }
}