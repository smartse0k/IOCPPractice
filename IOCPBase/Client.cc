#include "Client.h"
#include "Packet.h"
#include "PacketProcessor.h"

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

        // TODO : 나중에 에러 분기 추가할 것. 지금은 끊어지는 소켓에 recv하면 throw 날듯
        // 아래 doc의 return value 참조
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

        while (true) {
            unsigned short packetLength = 0;

            // 아직 패킷 크기가 도착하지 않은 경우
            if (recvOverlapped.currentBufferSize < sizeof(packetLength)) {
                logger->debug() << "Can not resolve packet size. waiting for next data.\n";
                break;
            }

            // 패킷 길이 복사
            // 2번째 인자를 "recvOverlapped.buffer"로 해도 될 것 같은데 C6385 경고가 나타난다.
            std::memcpy(&packetLength, &recvOverlapped.buffer[0], sizeof(packetLength));

            // 패킷 길이가 0으로 전달되었다면 무시한다. 이후 이 클라이언트의 패킷을 처리할 수 없을 수도 있다.
            if (packetLength == 0) {
                logger->warn() << "Packet length is zero.";
                packetLength = sizeof(packetLength);
            }

            // 아직 패킷 전체가 도착하지 않은 경우
            if (recvOverlapped.currentBufferSize < packetLength) {
                logger->debug() << "Can not resolve packet. waiting for next data. packetLength=" << packetLength
                    << ", currentLength=" << recvOverlapped.currentBufferSize << "\n";
                break;
            }

            // 패킷 생성 (패킷 길이는 제외)
            Packet* packet = Packet::createFromByteArray(completionKey, &recvOverlapped.buffer[sizeof(packetLength)], 0, packetLength - sizeof(packetLength));
            PacketProcessor::enqueuePacket(packet);

            recvOverlapped.currentBufferSize -= packetLength;

            // 이 경우가 나와서는 안된다.
            if (recvOverlapped.currentBufferSize < 0) {
                logger->err() << "recvOverlapped.currentBufferSize is less than 0";
                throw "recvOverlapped.currentBufferSize is less than 0";
            }

            // 더 이상 남은 패킷이 없는 경우
            if (recvOverlapped.currentBufferSize == 0) {
                break;
            }

            // 남은 데이터 이동
            std::memmove(recvOverlapped.buffer, &recvOverlapped.buffer[packetLength], recvOverlapped.currentBufferSize);
        }
    }

    void Client::onSend(unsigned int length) {
        // TODO
        throw "No implement function";
    }

    void Client::enqueuePacket(Packet* packet) {
        mutexReceivedPacketQueue.lock();
        receivedPacketQueue.push(packet);
        mutexReceivedPacketQueue.unlock();

        // Atomic Thread Barrier
        bool isAlreadyProcessing = isProcessing.exchange(true);
        if (isAlreadyProcessing) {
            return;
        }

        unsigned int recursiveDepth = 0;
        processPacket(recursiveDepth);

        isProcessing.store(false);
    }

    // from onPacket
    void Client::processPacket(unsigned int& recursiveDepth) {
        // DoS를 방지하기 위해 n번의 큐 처리만 진행해준다.
        const int maxRecursiveDepth = 3;
        recursiveDepth++;

        if (recursiveDepth >= maxRecursiveDepth) {
            logger->warn() << "processPacket ignored. exceed max recursive depth. completionKey=" << completionKey << "\n";
            return;
        }

        // 큐 처리
        std::queue<Packet*> processQueue;

        mutexReceivedPacketQueue.lock();
        processQueue.swap(receivedPacketQueue);
        mutexReceivedPacketQueue.unlock();

        while (!processQueue.empty()) {
            Packet* packet = processQueue.front();
            processQueue.pop();

            onPacket(packet);

            delete packet;
        }

        // 큐 처리 이후에 쌓인 패킷이 있다면 더 처리 해준다.
        bool hasPacket = false;
        mutexReceivedPacketQueue.lock();
        if (receivedPacketQueue.size() > 0) {
            hasPacket = true;
        }
        mutexReceivedPacketQueue.unlock();

        if (hasPacket) {
            processPacket(recursiveDepth);
        }
    }

    void Client::onPacket(Packet* packet) {
        logger->debug() << "onPacket()\n";
        logger->err() << "onPacket is not implemented!\n";
        packet->printInfoToCLI();

        // 패킷 샘플 : 0F 00 01 00 00 00 05 00 00 00 48 65 6c 6c 6f
        // [unsigned short 15 = SIZE] => Size는 Packet 객체에서는 생략되어 있으므로 주의.
        // [unsigned int 1 = OP Code]
        // [unsigned int 5 = string size]
        // [char(5) = "Hello"]
    }
}