#include "EchoServer.h"
#include "UserClient.h"

void EchoServer::startServer() {
    // TODO : 로거를 public으로 바꿔야 할까?
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
