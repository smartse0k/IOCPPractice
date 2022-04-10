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
        unsigned int size = 0;

        if (!read(size)) {
            return false;
        }

        if (readOffset + size > length) {
            return false;
        }

        out.assign(&data[readOffset], size);
        readOffset += size;

        return true;
    }
}