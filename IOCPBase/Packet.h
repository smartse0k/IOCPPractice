#pragma once

#include "Logger.h"

namespace phodobit {
    class Packet {
    public:
        static Packet *createFromByteArray(int ownerCompletionKey, char *byteArray, unsigned int start, unsigned short length);

        void printInfoToCLI();

        ~Packet();
    private:
        static Logger* logger;

        int ownerCompletionKey;
        char* data;
        unsigned short length;
    };
}