#include "PacketProcessor.h"
#include "Client.h"

namespace phodobit {
    Logger* PacketProcessor::logger = Logger::getLogger("PacketProcessor")->setLogLevel(LogLevel::DEBUG);

    unsigned int PacketProcessor::processorCount = 0;
    std::vector<std::thread*> PacketProcessor::threadList;

    unsigned int PacketProcessor::nextEnqueueProcessorIndex = 0;
    std::vector<std::queue<std::pair<Client*, Packet*>>*> PacketProcessor::queueList;

    std::vector<std::mutex*> PacketProcessor::mutexQueueList;
    std::mutex PacketProcessor::mutexNextEnqueueProcessorIndex;

    void PacketProcessor::createPacketProcessor(int threadCount) {
        for (int i = 0; i < threadCount; i++) {
            std::queue<std::pair<Client*, Packet*>>* queue = new std::queue<std::pair<Client*, Packet*>>();
            queueList.push_back(queue);

            std::mutex* mutex = new std::mutex();
            mutexQueueList.push_back(mutex);

            std::thread* thread = new std::thread(processor, i);
            threadList.push_back(thread);
        }

        processorCount = threadCount;
    }

    void PacketProcessor::enqueuePacket(Client* client, Packet* packet) {
        // 다음 프로세서 인덱스 획득
        mutexNextEnqueueProcessorIndex.lock();
        nextEnqueueProcessorIndex++;
        if (nextEnqueueProcessorIndex >= processorCount) {
            nextEnqueueProcessorIndex = 0;
        }
        mutexNextEnqueueProcessorIndex.unlock();

        // 프로세서에 패킷 인큐
        mutexQueueList[nextEnqueueProcessorIndex]->lock();
        queueList[nextEnqueueProcessorIndex]->push(std::make_pair(client, packet));
        mutexQueueList[nextEnqueueProcessorIndex]->unlock();
    }

    // Thread Entry Point
    void PacketProcessor::processor(const unsigned int processorId) {
        std::mutex* mutex = mutexQueueList[processorId];
        std::queue<std::pair<Client*, Packet*>>* queue = queueList[processorId];

        while (isRunningAndSleep()) {
            if (queue->empty()) {
                continue;
            }

            std::queue<std::pair<Client*, Packet*>> q;
            mutex->lock();
            queue->swap(q);
            mutex->unlock();

            while (!q.empty()) {
                std::pair<Client*, Packet*> front = q.front();
                q.pop();

                Client *client = front.first;
                Packet* packet = front.second;

                if (client == nullptr) {
                    logger->err() << "Not exist client.";
                    continue;
                }

                client->enqueuePacket(packet);
            }
        }
    }
}