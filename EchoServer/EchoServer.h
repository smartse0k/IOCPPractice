#pragma once

#include <IOCP.h>
#include <PacketProcessor.h>

#include "UserClient.h"

class EchoServer : public phodobit::IOCP {
public:
    void startServer();
    phodobit::Client* onAccept(int key, SOCKET socket) override;
};
