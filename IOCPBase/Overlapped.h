#pragma once

#include <winsock2.h>

constexpr int IOCP_BUFFER_SIZE = 8192;

namespace phodobit {
    class Overlapped : public OVERLAPPED {
    public:
        enum class TYPE {
            NONE = 0,
            RECV,
            SEND
        };

        Overlapped() {
            buffer = new char[IOCP_BUFFER_SIZE];
            memset(buffer, 0, IOCP_BUFFER_SIZE);
            type = TYPE::NONE;
            currentBufferSize = 0;
        }

        ~Overlapped() {
            delete buffer;
        }

        WSABUF wsaBuf;
        char *buffer;
        TYPE type;
        unsigned int currentBufferSize;
    };
}