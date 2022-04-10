#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <utility>

#include "Client.h"
#include "Packet.h"

// TODO : static이 남발되어 있는데 나중에 객체지향적으로 수정...

namespace phodobit {
    class PacketProcessor {
    public:
        static void createPacketProcessor(int threadCount);
        static void enqueuePacket(Client* client, Packet *packet);

        static void processor(const unsigned int processorId);
        
        inline static bool isRunningAndSleep() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return true;
        }

    private:
        static Logger* logger;

        static unsigned int processorCount;
        static std::vector<std::thread*> threadList;

        static unsigned int nextEnqueueProcessorIndex;
        static std::vector<std::queue<std::pair<Client*, Packet*>>*> queueList;

        static std::vector<std::mutex*> mutexQueueList;
        static std::mutex mutexNextEnqueueProcessorIndex;
    };
}