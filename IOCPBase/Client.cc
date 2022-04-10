#include "Client.h"
#include "Packet.h"
#include "PacketProcessor.h"

namespace phodobit {
    Logger* Client::logger = Logger::getLogger("Client")->setLogLevel(LogLevel::DEBUG);
    std::map<int, Client*>* Client::clients;

    Client::Client(int completionKey, SOCKET socket) {
        this->socket = socket;
        this->completionKey = completionKey;

        recvOverlapped.type = Overlapped::TYPE::RECV;
        sendOverlapped.type = Overlapped::TYPE::SEND;

        isSending.store(false);
        isProcessing.store(false);
        isClosed.store(false);
    }

    void Client::bind(HANDLE iocpHandle) {
        logger->debug() << "bind()\n";

        CreateIoCompletionPort((HANDLE)socket, iocpHandle, completionKey, 0);
    }

    void Client::recv() {
        // 연결이 끊어졌으므로 스킵한다.
        if (isClosed.load()) {
            return;
        }

        DWORD readSize = 0;
        DWORD readFlag = 0;

        recvOverlapped.wsaBuf.buf = &recvOverlapped.buffer[recvOverlapped.currentBufferSize];
        recvOverlapped.wsaBuf.len = IOCP_BUFFER_SIZE - recvOverlapped.currentBufferSize;

        int ret = WSARecv(socket, &recvOverlapped.wsaBuf, 1, &readSize, &readFlag, &recvOverlapped, nullptr);

        if (ret == 0) {
            return;
        } else if (ret != SOCKET_ERROR) {
            logger->err() << "recv failed. ret is not SOCKET_ERROR. ret=" << ret << "\n";
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

    void Client::send(Packet* packet) {
        mutexSend.lock();

        unsigned short originalLength = sendOverlapped.currentBufferSize;
        unsigned short packetLength = sizeof(packetLength) + packet->getLength();
        unsigned short nextLength = originalLength + packetLength;

        if (nextLength > IOCP_BUFFER_SIZE) {
            throw "buffer is full";
        }

        std::memcpy(&sendOverlapped.buffer[originalLength], &packetLength, sizeof(packetLength));
        std::memcpy(&sendOverlapped.buffer[originalLength + sizeof(packetLength)], packet->getData(), packet->getLength());
        sendOverlapped.currentBufferSize = nextLength;

        sendOverlapped.wsaBuf.buf = sendOverlapped.buffer;
        sendOverlapped.wsaBuf.len = nextLength;

        send();
        
        mutexSend.unlock();
    }

    void Client::send() {
        // 연결이 끊어졌으므로 스킵한다.
        if (isClosed.load()) {
            return;
        }

        // 이미 보내고 있는 중이므로 스킵한다. send(Packet*)과 onSend()에서 호출된다.
        if (isSending.load()) {
            return;
        }

        if (sendOverlapped.currentBufferSize <= 0) {
            logger->err() << "send failed. current buffer is empty\n";
            return;
        }

        if (sendOverlapped.wsaBuf.len <= 0) {
            logger->err() << "send failed. wsaBuf.len is zero\n";
            return;
        }

        // sendSize는 WSASend 호출했을 당시 바로 전송된 크기인데,
        // 이 크기도 IOCP Event에 포함되어 응답되므로 유의미하지 않다.
        DWORD sendSize;
        int ret = WSASend(socket, &sendOverlapped.wsaBuf, 1, &sendSize, 0, &sendOverlapped, NULL);

        if (ret == 0) {
            // 즉시 전송된 경우 아무것도 하지 않는다.
        } else if (ret == SOCKET_ERROR) {
            int lastError = WSAGetLastError();

            switch (lastError) {
            case WSA_IO_PENDING:
                // IOCP 전송 대기 상태.
                break;
            default:
                logger->err() << "send failed. lastError is " << lastError << "\n";
                throw "send failed. ret is not WSA_IO_PENDING";
            }
        } else {
            logger->err() << "send failed. ret is not SOCKET_ERROR. ret=" << ret << "\n";
            throw "send failed. ret is not SOCKET_ERROR";
        }
    }

    void Client::onRecv(unsigned int length) {
        logger->debug() << "onRecv()\n";

        // 연결이 끊어진 경우
        if (length == 0) {
            onClose();
            return;
        }

        recvOverlapped.currentBufferSize += length;

        if (false) {
            logger->debug() << "Receive info...\n"
                            << "  received size: " << length << "\n"
                            << "  buffer size: " << recvOverlapped.currentBufferSize << "\n";
        }

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
            Packet* packet = Packet::createFromByteArray(&recvOverlapped.buffer[sizeof(packetLength)], 0, packetLength - sizeof(packetLength));
            PacketProcessor::enqueuePacket(this, packet);

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

        recv();
    }

    void Client::onSend(unsigned int length) {
        // 연결이 끊어진 경우
        if (length == 0) {
            onClose();
            return;
        }

        mutexSend.lock();

        isSending.store(false);

        sendOverlapped.currentBufferSize -= length;

        // 이 경우가 나와서는 안된다.
        if (sendOverlapped.currentBufferSize < 0) {
            logger->err() << "sendOverlapped.currentBufferSize is less than 0";
            throw "sendOverlapped.currentBufferSize is less than 0";
        }

        std::memmove(sendOverlapped.buffer, &sendOverlapped.buffer[length], length);
        sendOverlapped.wsaBuf.len = sendOverlapped.currentBufferSize;

        // 아직 보낼 데이터가 남아있다.
        if (sendOverlapped.currentBufferSize > 0) {
            send();
        }

        mutexSend.unlock();
    }

    void Client::onClose() {
        isClosed.store(true);

        _close();
    }

    void Client::_close() {
        logger->debug() << "_close()\n";

        mutexClosing.lock();

        // 처리중인 Recv가 있다면 현재는 생략하고, Recv가 다시 호출하도록 기다린다.
        if (isProcessing.load()) {
            logger->debug() << "_close ignored. the client is receiving\n";
            return mutexClosing.unlock();
        }

        // 처리중인 Send가 있다면 현재는 생략하고, Send가 다시 호출하도록 기다린다.
        if (isSending.load()) {
            logger->debug() << "_close ignored. the client is sending\n";
            return mutexClosing.unlock();
        }

        shutdown(socket, SD_BOTH);
        closesocket(socket);

        logger->debug() << "socket closed. completionKey=" << completionKey << "\n";
        
        mutexClosing.unlock();
    }

    // TODO : 받은 패킷에 대해서 인큐하는 것이므로... 네이밍을 바꾸던지... 해야할듯
    void Client::enqueuePacket(Packet* packet) {
        mutexReceivedPacketQueue.lock();

        receivedPacketQueue.push(packet);

        // Atomic Thread Barrier
        bool isAlreadyProcessing = isProcessing.exchange(true);

        mutexReceivedPacketQueue.unlock();

        if (isAlreadyProcessing) {
            return;
        }

        unsigned int recursiveDepth = 0;
        processPacket(recursiveDepth);

        isProcessing.store(false);

        if (isClosed.load()) {
            _close();
        }
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
    }
}