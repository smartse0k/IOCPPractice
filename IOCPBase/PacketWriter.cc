#include <cstring>

#include "Packet.h"

namespace phodobit {
    void Packet::extend(unsigned short needSize) {
        // 부족한 경우에만 확장한다
        if (needSize + length <= _length) {
            return;
        }

        // TODO : needSize를 고려해서 확장할 수 있도록 바꿔야함.

        // 버퍼가 아예 생성되지 않았다면 생성만 한다.
        if (data == nullptr || _length == 0) {
            data = new char[1024];
            _length = 1024;
            return;
        }

        char* newData = new char[_length * 2];
        std::memcpy(newData, data, _length);
        delete[] data;

        data = newData;
        _length *= 2;
    }

    void Packet::write(int& in) {
        extend(sizeof(int));
        std::memcpy(&data[length], &in, sizeof(int));
        length += sizeof(int);
    }

    void Packet::write(unsigned int& in) {
        extend(sizeof(unsigned int));
        std::memcpy(&data[length], &in, sizeof(unsigned int));
        length += sizeof(unsigned int);
    }

    void Packet::write(std::string& in) {
        unsigned int size = in.size();
        write(size);

        extend(size);
        const char* str = in.c_str();
        std::memcpy(&data[length], str, size);
        length += size;
    }
}