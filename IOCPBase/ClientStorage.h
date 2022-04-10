#pragma once

#include <map>
#include <mutex>

#include "Client.h"

namespace phodobit {
    class ClientStorage {
    public:
        void set(int key, Client* client);
        Client* get(int key);
        bool remove(int key);
        
    private:
        std::map<int, Client*> clients;
        std::mutex mutex;
    };
}