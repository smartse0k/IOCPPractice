#pragma once

#include "Logger.h"

namespace phodobit {
    class Packet {
    public:
        static Packet *createFromByteArray(char *byteArray, unsigned int start, unsigned short length);
        Packet();

        bool read(int& out);
        bool read(unsigned int& out);
        bool read(std::string& out);

        char* getData();
        unsigned short getLength();

        void printInfoToCLI();

        ~Packet();
    private:
        static Logger* logger;

        char* data;
        unsigned short length;

        unsigned short readOffset;
    };
}