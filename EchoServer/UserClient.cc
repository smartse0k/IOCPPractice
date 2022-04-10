#include "UserClient.h"

UserClient::UserClient(int key, SOCKET socket) : Client(key, socket) {

}

void UserClient::onPacket(phodobit::Packet* recvPacket) {
    phodobit::Packet* sendPacket = phodobit::Packet::createFromByteArray(recvPacket->getData(), 0, recvPacket->getLength());
    send(sendPacket);
}
