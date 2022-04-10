#include "EchoServer.h"
#include "UserClient.h"

void EchoServer::startServer() {
    // TODO : �ΰŸ� public���� �ٲ�� �ұ�?
    //logger->setLogLevel(LogLevel::DEBUG);

    initialize();
    bind(9999);
    listen();

    createAcceptThread(1);
    createWorkerThread(2);
}

phodobit::Client* EchoServer::onAccept(int key, SOCKET socket) {
    phodobit::Client* client = new UserClient(key, socket);
    return client;
}
