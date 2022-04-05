#pragma once

#include "Logger.h"

namespace phodobit {
    class Packet {
    public:
        static Packet *createFromByteArray(int ownerCompletionKey, char *byteArray, unsigned int start, unsigned short length);

        bool read(int& out);
        bool read(unsigned int& out);
        bool read(std::string& out);

        int getOwnerCompletionKey();
        char* getData();
        unsigned short getLength();

        void printInfoToCLI();

        ~Packet();
    private:
        static Logger* logger;

        int ownerCompletionKey;
        char* data;
        unsigned short length;

        unsigned short readOffset;
    };
}