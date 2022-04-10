#pragma once

#include <Client.h>

class UserClient : public phodobit::Client {
public:
    UserClient(int key, SOCKET socket);
    void onPacket(phodobit::Packet* packet) override;
};
