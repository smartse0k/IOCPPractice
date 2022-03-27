#include <cstring>

#include "Packet.h"

namespace phodobit {
    bool Packet::read(int& out) {
        if (readOffset + sizeof(int) > length) {
            return false;
        }

        std::memcpy(&out, &data[readOffset], sizeof(int));
        readOffset += sizeof(int);

        return true;
    }

    bool Packet::read(unsigned int& out) {
        if (readOffset + sizeof(unsigned int) > length) {
            return false;
        }

        std::memcpy(&out, &data[readOffset], sizeof(unsigned int));
        readOffset += sizeof(unsigned int);

        return true;
    }

    bool Packet::read(std::string& out) {
        // TODO : unsigned int·Î º¯°æ

        int size = -1;
        bool ret;
        ret = read(size);

        if (!ret) {
            return false;
        }

        if (readOffset + size > length) {
            return false;
        }

        char* temp = new char[size + 1];
        std::memcpy(temp, &data[readOffset], size);
        temp[size] = 0;
        out = std::string(temp);

        delete[] temp;

        return true;
    }
}