#include "Client.h"
#include "Packet.h"

namespace phodobit {
    Logger* Client::logger = Logger::getLogger("Client")->setLogLevel(LogLevel::DEBUG);
    std::map<int, Client*>* Client::clients;

    Client *Client::getClient(int completionKey) {
        if (clients == nullptr) {
            logger->info() << "create client map\n";
            clients = new std::map<int, Client*>();
        }

        std::map<int, Client*>::iterator it = clients->find(completionKey);
        if (it == clients->end()) {
            return nullptr;
        }

        return it->second;
    }

    void Client::setClient(int completionKey, Client* client) {
        logger->debug() << "setClient()\n";

        if (clients == nullptr) {
            logger->info() << "create client map\n";
            clients = new std::map<int, Client*>();
        }

        clients->insert({ completionKey, client });
    }

    Client::Client(SOCKET socket, int completionKey) {
        this->socket = socket;
        this->completionKey = completionKey;

        recvOverlapped.type = Overlapped::TYPE::RECV;
        sendOverlapped.type = Overlapped::TYPE::SEND;
    }

    void Client::bind(HANDLE iocpHandle) {
        logger->debug() << "bind()\n";

        CreateIoCompletionPort((HANDLE)socket, iocpHandle, completionKey, 0);
    }

    void Client::recv() {
        DWORD readSize = 0;
        DWORD readFlag = 0;

        recvOverlapped.wsaBuf.buf = &recvOverlapped.buffer[recvOverlapped.currentBufferSize];
        recvOverlapped.wsaBuf.len = IOCP_BUFFER_SIZE - recvOverlapped.currentBufferSize;

        int ret = WSARecv(socket, &recvOverlapped.wsaBuf, 1, &readSize, &readFlag, &recvOverlapped, nullptr);

        if (ret != SOCKET_ERROR) {
            logger->err() << "recv failed. ret is not SOCKET_ERROR\n";
            throw "recv failed. ret is not SOCKET_ERROR";
        }

        int lastError = WSAGetLastError();

        // TODO : ���߿� ���� �б� �߰��� ��. ������ �������� ���Ͽ� recv�ϸ� throw ����
        // �Ʒ� doc�� return value ����
        // https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv

        switch (lastError) {
        case WSA_IO_PENDING:
            return;
        default:
            logger->err() << "recv failed. lastError is " << lastError << "\n";
            throw "recv failed. ret is not WSA_IO_PENDING";
            break;
        }
    }

    void Client::send() {
        // TODO
        throw "No implement function";
    }

    void Client::onRecv(unsigned int length) {
        logger->debug() << "onRecv()\n";

        recvOverlapped.currentBufferSize += length;

        logger->debug() << "Receive info...\n"
                        << "  received size: " << length << "\n"
                        << "  buffer size: " << recvOverlapped.currentBufferSize << "\n";

        if (true) {
            *logger << "  received buffer: ";
            logger->printByteArray(recvOverlapped.buffer,
                                   recvOverlapped.currentBufferSize - length,
                                   length);
            *logger << "\n";

            *logger << "  total buffer: ";
            logger->printByteArray(recvOverlapped.buffer, 0, recvOverlapped.currentBufferSize);
            *logger << "\n";
        }

        while (true) {
            unsigned short packetLength = 0;

            // ���� ��Ŷ ũ�Ⱑ �������� ���� ���
            if (recvOverlapped.currentBufferSize < sizeof(packetLength)) {
                logger->debug() << "Can not resolve packet size. waiting for next data.";
                break;
            }

            // ��Ŷ ���� ����
            // 2��° ���ڸ� "recvOverlapped.buffer"�� �ص� �� �� ������ C6385 ��� ��Ÿ����.
            std::memcpy(&packetLength, &recvOverlapped.buffer[0], sizeof(packetLength));

            // ��Ŷ ���̰� 0���� ���޵Ǿ��ٸ� �����Ѵ�. ���� �� Ŭ���̾�Ʈ�� ��Ŷ�� ó���� �� ���� ���� �ִ�.
            if (packetLength == 0) {
                logger->warn() << "Packet length is zero.";
                packetLength = sizeof(packetLength);
                break;
            }

            // ���� ��Ŷ ��ü�� �������� ���� ���
            if (recvOverlapped.currentBufferSize < packetLength) {
                logger->debug() << "Can not resolve packet. waiting for next data. packetLength=" << packetLength
                    << ", currentLength=" << recvOverlapped.currentBufferSize << "\n";
                break;
            }

            // ��Ŷ ����
            Packet* packet = Packet::createFromByteArray(completionKey, recvOverlapped.buffer, 0, packetLength);
            packet->printInfoToCLI();

            recvOverlapped.currentBufferSize -= packetLength;

            // �� ��찡 ���ͼ��� �ȵȴ�.
            if (recvOverlapped.currentBufferSize < 0) {
                logger->err() << "recvOverlapped.currentBufferSize is less than 0";
                throw "recvOverlapped.currentBufferSize is less than 0";
            }

            // �� �̻� ���� ��Ŷ�� ���� ���
            if (recvOverlapped.currentBufferSize == 0) {
                break;
            }

            // ���� ������ �̵�
            std::memmove(recvOverlapped.buffer, &recvOverlapped.buffer[packetLength], recvOverlapped.currentBufferSize);
        }
    }

    void Client::onSend(unsigned int length) {
        // TODO
        throw "No implement function";
    }
}