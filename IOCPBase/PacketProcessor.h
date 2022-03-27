#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>

#include "Packet.h"

// TODO : static�� ���ߵǾ� �ִµ� ���߿� ��ü���������� ����...

namespace phodobit {
    class PacketProcessor {
    public:
        static void createPacketProcessor(int threadCount);
        static void enqueuePacket(Packet *packet);

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
        static std::vector<std::queue<Packet*>*> queueList;

        static std::vector<std::mutex*> mutexQueueList;
        static std::mutex mutexNextEnqueueProcessorIndex;
    };
}