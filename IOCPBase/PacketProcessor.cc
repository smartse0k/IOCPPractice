#include "PacketProcessor.h"
#include "Client.h"

namespace phodobit {
    Logger* PacketProcessor::logger = Logger::getLogger("PacketProcessor")->setLogLevel(LogLevel::DEBUG);

    unsigned int PacketProcessor::processorCount = 0;
    std::vector<std::thread*> PacketProcessor::threadList;

    unsigned int PacketProcessor::nextEnqueueProcessorIndex = 0;
    std::vector<std::queue<Packet*>*> PacketProcessor::queueList;

    std::vector<std::mutex*> PacketProcessor::mutexQueueList;
    std::mutex PacketProcessor::mutexNextEnqueueProcessorIndex;

    void PacketProcessor::createPacketProcessor(int threadCount) {
        for (int i = 0; i < threadCount; i++) {
            std::queue<Packet*>* queue = new std::queue<Packet*>();
            queueList.push_back(queue);

            std::mutex* mutex = new std::mutex();
            mutexQueueList.push_back(mutex);

            std::thread* thread = new std::thread(processor, i);
            threadList.push_back(thread);
        }

        processorCount = threadCount;
    }

    void PacketProcessor::enqueuePacket(Packet* packet) {
        // ���� ���μ��� �ε��� ȹ��
        mutexNextEnqueueProcessorIndex.lock();
        nextEnqueueProcessorIndex++;
        if (nextEnqueueProcessorIndex >= processorCount) {
            nextEnqueueProcessorIndex = 0;
        }
        mutexNextEnqueueProcessorIndex.unlock();

        // ���μ����� ��Ŷ ��ť
        mutexQueueList[nextEnqueueProcessorIndex]->lock();
        queueList[nextEnqueueProcessorIndex]->push(packet);
        mutexQueueList[nextEnqueueProcessorIndex]->unlock();
    }

    // Thread Entry Point
    void PacketProcessor::processor(const unsigned int processorId) {
        std::mutex* mutex = mutexQueueList[processorId];
        std::queue<Packet*>* queue = queueList[processorId];

        while (isRunningAndSleep()) {
            if (queue->empty()) {
                continue;
            }

            std::queue<Packet*> q;
            mutex->lock();
            queue->swap(q);
            mutex->unlock();

            while (!q.empty()) {
                Packet* packet = q.front();
                q.pop();

                int completionKey = packet->getOwnerCompletionKey();
                Client *client = Client::getClient(completionKey);

                if (client == nullptr) {
                    logger->err() << "Not exist client.";
                    continue;
                }

                client->enqueuePacket(packet);
            }
        }
    }
}