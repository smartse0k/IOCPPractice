#include <cstring>

#include "Packet.h"

namespace phodobit {
    void Packet::extend(unsigned short needSize) {
        // ������ ��쿡�� Ȯ���Ѵ�
        if (needSize + length <= _length) {
            return;
        }

        // TODO : needSize�� ����ؼ� Ȯ���� �� �ֵ��� �ٲ����.

        // ���۰� �ƿ� �������� �ʾҴٸ� ������ �Ѵ�.
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