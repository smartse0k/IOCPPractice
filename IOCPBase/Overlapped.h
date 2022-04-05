#pragma once

#include <winsock2.h>

// 현재 Packet은 최대 65535 바이트(uint16 표현범위)이므로,
// 65536으로 충분해보이지만 65530을 받고 남은 사이즈를 받으려다
// wsaBuf의 len이 0이 되는 순간 0바이트를 받으려고 시도하는 순간
// 0바이트를 계속해서 수신하려는 문제가 생길 수 있다.
// 그렇기 때문에 넉넉하게 x2 정도 크기를 잡아준다.
constexpr int IOCP_BUFFER_SIZE = 65536 * 2;

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
            std::memset(buffer, 0, IOCP_BUFFER_SIZE);
            type = TYPE::NONE;
            currentBufferSize = 0;
        }

        ~Overlapped() {
            delete[] buffer;
        }

        WSABUF wsaBuf;
        char *buffer;
        TYPE type;
        unsigned int currentBufferSize;
    };
}