#include "ClientStorage.h"

namespace phodobit {
    void ClientStorage::set(int key, Client* client) {
        mutex.lock();
        clients.erase(key);
        clients.emplace(key, client);
        mutex.unlock();
    }

    Client* ClientStorage::get(int key) {
        mutex.lock();
        std::map<int, Client*>::iterator it = clients.find(key);
        Client* ret = nullptr;
        if (it != clients.end()) {
            ret = it->second;
        }
        mutex.unlock();
        return ret;
    }

    bool ClientStorage::remove(int key) {
        mutex.lock();
        bool ret = clients.erase(key) > 0;
        mutex.unlock();
        return ret;
    }
}