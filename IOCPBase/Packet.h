#pragma once

#include "Logger.h"

namespace phodobit {
    class Packet {
    public:
        static Packet *createFromByteArray(char *byteArray, unsigned int start, unsigned short length);

        void printInfoToCLI();

        ~Packet();
    private:
        static Logger* logger;

        char* data;
        unsigned short length;
    };
}