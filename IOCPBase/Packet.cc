#include <cstring>

#include "Packet.h"

namespace phodobit {
    Logger* Packet::logger = Logger::getLogger("Packet")->setLogLevel(LogLevel::DEBUG);

    Packet* Packet::createFromByteArray(int ownerCompletionKey, char* byteArray, unsigned int start, unsigned short length) {
        logger->debug() << "createFromByteArray()\n";

        Packet* packet = new Packet();

        packet->ownerCompletionKey = ownerCompletionKey;

        packet->data = new char[length];
        std::memcpy(packet->data, &byteArray[start], length);

        packet->length = length;
        packet->readOffset = 0;

        return packet;
    }

    // Read 관련은 PacketReader.cc에서 구현한다.
    // Write 관련은 PacketWriter.cc에서 구현한다.

    int Packet::getOwnerCompletionKey() {
        return ownerCompletionKey;
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