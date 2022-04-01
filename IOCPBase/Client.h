#pragma once

#include <map>
#include <winsock2.h>
#include <queue>
#include <mutex>

#include "Logger.h"
#include "Overlapped.h"
#include "Packet.h"

namespace phodobit {
    class Client {
    public:
        static Client* getClient(int completionKey);
        static void setClient(int completionKey, Client*);

        Client(SOCKET socket, int completionKey);
        void bind(HANDLE iocpHandle);
        void recv();
        void send();
        void onRecv(unsigned int length);
        void onSend(unsigned int length);
        void enqueuePacket(Packet* packet);
        void processPacket(unsigned int& recursiveDepth);
        virtual void onPacket(Packet* packet);
    private:
        static Logger* logger;

        static std::map<int, Client*> *clients;

        SOCKET socket;
        int completionKey;

        Overlapped recvOverlapped;
        Overlapped sendOverlapped;

        std::queue<Packet*> receivedPacketQueue;
        std::mutex mutexReceivedPacketQueue;
        std::atomic<bool> isProcessing;
    };
}