#pragma once

#include <winsock2.h>

// ���� Packet�� �ִ� 65535 ����Ʈ(uint16 ǥ������)�̹Ƿ�,
// 65536���� ����غ������� 65530�� �ް� ���� ����� ��������
// wsaBuf�� len�� 0�� �Ǵ� ���� 0����Ʈ�� �������� �õ��ϴ� ����
// 0����Ʈ�� ����ؼ� �����Ϸ��� ������ ���� �� �ִ�.
// �׷��� ������ �˳��ϰ� x2 ���� ũ�⸦ ����ش�.
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