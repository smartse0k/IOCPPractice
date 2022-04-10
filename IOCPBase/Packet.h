#pragma once

#include "Logger.h"

namespace phodobit {
    class Packet {
    public:
        static Packet *createFromByteArray(char *byteArray, unsigned int start, unsigned short length);
        Packet();

        void write(int& in);
        void write(unsigned int& in);
        void write(std::string& in);

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
        unsigned short length; // ���ǹ��� ������ ����
        unsigned short _length; // data ��ü ũ��

        unsigned short readOffset;

        void extend(unsigned short needSize);
    };
}