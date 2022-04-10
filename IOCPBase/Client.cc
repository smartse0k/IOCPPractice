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
        // ������ ���������Ƿ� ��ŵ�Ѵ�.
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
        // ������ ���������Ƿ� ��ŵ�Ѵ�.
        if (isClosed.load()) {
            return;
        }

        // �̹� ������ �ִ� ���̹Ƿ� ��ŵ�Ѵ�. send(Packet*)�� onSend()���� ȣ��ȴ�.
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

        // sendSize�� WSASend ȣ������ ��� �ٷ� ���۵� ũ���ε�,
        // �� ũ�⵵ IOCP Event�� ���ԵǾ� ����ǹǷ� ���ǹ����� �ʴ�.
        DWORD sendSize;
        int ret = WSASend(socket, &sendOverlapped.wsaBuf, 1, &sendSize, 0, &sendOverlapped, NULL);

        if (ret == 0) {
            // ��� ���۵� ��� �ƹ��͵� ���� �ʴ´�.
        } else if (ret == SOCKET_ERROR) {
            int lastError = WSAGetLastError();

            switch (lastError) {
            case WSA_IO_PENDING:
                // IOCP ���� ��� ����.
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

        // ������ ������ ���
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

            // ���� ��Ŷ ũ�Ⱑ �������� ���� ���
            if (recvOverlapped.currentBufferSize < sizeof(packetLength)) {
                logger->debug() << "Can not resolve packet size. waiting for next data.\n";
                break;
            }

            // ��Ŷ ���� ����
            // 2��° ���ڸ� "recvOverlapped.buffer"�� �ص� �� �� ������ C6385 ��� ��Ÿ����.
            std::memcpy(&packetLength, &recvOverlapped.buffer[0], sizeof(packetLength));

            // ��Ŷ ���̰� 0���� ���޵Ǿ��ٸ� �����Ѵ�. ���� �� Ŭ���̾�Ʈ�� ��Ŷ�� ó���� �� ���� ���� �ִ�.
            if (packetLength == 0) {
                logger->warn() << "Packet length is zero.";
                packetLength = sizeof(packetLength);
            }

            // ���� ��Ŷ ��ü�� �������� ���� ���
            if (recvOverlapped.currentBufferSize < packetLength) {
                logger->debug() << "Can not resolve packet. waiting for next data. packetLength=" << packetLength
                    << ", currentLength=" << recvOverlapped.currentBufferSize << "\n";
                break;
            }

            // ��Ŷ ���� (��Ŷ ���̴� ����)
            Packet* packet = Packet::createFromByteArray(&recvOverlapped.buffer[sizeof(packetLength)], 0, packetLength - sizeof(packetLength));
            PacketProcessor::enqueuePacket(this, packet);

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

        recv();
    }

    void Client::onSend(unsigned int length) {
        // ������ ������ ���
        if (length == 0) {
            onClose();
            return;
        }

        mutexSend.lock();

        isSending.store(false);

        sendOverlapped.currentBufferSize -= length;

        // �� ��찡 ���ͼ��� �ȵȴ�.
        if (sendOverlapped.currentBufferSize < 0) {
            logger->err() << "sendOverlapped.currentBufferSize is less than 0";
            throw "sendOverlapped.currentBufferSize is less than 0";
        }

        std::memmove(sendOverlapped.buffer, &sendOverlapped.buffer[length], length);
        sendOverlapped.wsaBuf.len = sendOverlapped.currentBufferSize;

        // ���� ���� �����Ͱ� �����ִ�.
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

        // ó������ Recv�� �ִٸ� ����� �����ϰ�, Recv�� �ٽ� ȣ���ϵ��� ��ٸ���.
        if (isProcessing.load()) {
            logger->debug() << "_close ignored. the client is receiving\n";
            return mutexClosing.unlock();
        }

        // ó������ Send�� �ִٸ� ����� �����ϰ�, Send�� �ٽ� ȣ���ϵ��� ��ٸ���.
        if (isSending.load()) {
            logger->debug() << "_close ignored. the client is sending\n";
            return mutexClosing.unlock();
        }

        shutdown(socket, SD_BOTH);
        closesocket(socket);

        logger->debug() << "socket closed. completionKey=" << completionKey << "\n";
        
        mutexClosing.unlock();
    }

    // TODO : ���� ��Ŷ�� ���ؼ� ��ť�ϴ� ���̹Ƿ�... ���̹��� �ٲٴ���... �ؾ��ҵ�
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
        // DoS�� �����ϱ� ���� n���� ť ó���� �������ش�.
        const int maxRecursiveDepth = 3;
        recursiveDepth++;

        if (recursiveDepth >= maxRecursiveDepth) {
            logger->warn() << "processPacket ignored. exceed max recursive depth. completionKey=" << completionKey << "\n";
            return;
        }

        // ť ó��
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

        // ť ó�� ���Ŀ� ���� ��Ŷ�� �ִٸ� �� ó�� ���ش�.
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