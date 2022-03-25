#include "Client.h"

namespace phodobit {
	Logger* Client::logger = Logger::getLogger("Client")->setLogLevel(LogLevel::DEBUG);
	std::map<int, Client*>* Client::clients;

	Client *Client::getClient(int completionKey) {
		logger->debug() << "getClient()\n";

		if (clients == nullptr) {
			logger->info() << "create client map\n";
			clients = new std::map<int, Client*>();
		}

		std::map<int, Client*>::iterator it = clients->find(completionKey);
		if (it == clients->end()) {
			return nullptr;
		}

		return it->second;
	}

	void Client::setClient(int completionKey, Client* client) {
		logger->debug() << "setClient()\n";

		if (clients == nullptr) {
			logger->info() << "create client map\n";
			clients = new std::map<int, Client*>();
		}

		clients->insert({ completionKey, client });
	}

	Client::Client(SOCKET socket, int completionKey) {
		this->socket = socket;
		this->completionKey = completionKey;

		memset(&wsaBuf, 0, sizeof(wsaBuf));
		wsaBuf.buf = recvBuf;
		wsaBuf.len = sizeof(recvBuf);

		readSize = 0;
		readFlag = 0;

		memset(&wsaOverlapped, 0, sizeof(wsaOverlapped));
	}

	void Client::bind(HANDLE iocpHandle) {
		logger->debug() << "bind()\n";

		CreateIoCompletionPort((HANDLE)socket, iocpHandle, completionKey, 0);
	}

	void Client::recv() {
		WSARecv(socket, &wsaBuf, 1, &readSize, &readFlag, &wsaOverlapped, nullptr);
	}
}