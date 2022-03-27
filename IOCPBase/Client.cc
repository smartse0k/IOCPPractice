#include "Client.h"

namespace phodobit {
	Logger* Client::logger = Logger::getLogger("Client")->setLogLevel(LogLevel::DEBUG);
	std::map<int, Client*>* Client::clients;

	Client *Client::getClient(int completionKey) {
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

		recvOverlapped.type = Overlapped::TYPE::RECV;
		sendOverlapped.type = Overlapped::TYPE::SEND;
	}

	void Client::bind(HANDLE iocpHandle) {
		logger->debug() << "bind()\n";

		CreateIoCompletionPort((HANDLE)socket, iocpHandle, completionKey, 0);
	}

	void Client::recv() {
		DWORD readSize = 0;
		DWORD readFlag = 0;

		recvOverlapped.wsaBuf.buf = &recvOverlapped.buffer[recvOverlapped.currentBufferSize];
		recvOverlapped.wsaBuf.len = IOCP_BUFFER_SIZE - recvOverlapped.currentBufferSize;

		int ret = WSARecv(socket, &recvOverlapped.wsaBuf, 1, &readSize, &readFlag, &recvOverlapped, nullptr);

		if (ret != SOCKET_ERROR) {
			logger->err() << "recv failed. ret is not SOCKET_ERROR\n";
			throw "recv failed. ret is not SOCKET_ERROR";
		}

		int lastError = WSAGetLastError();

		// TODO : 나중에 에러 분기 추가할 것. 지금은 끊어지는 소켓에 recv하면 throw 날듯
		// 아래 doc의 return value 참조
		// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv

		switch (lastError) {
		case WSA_IO_PENDING:
			return;
		default:
			logger->err() << "recv failed. lastError is " << lastError << "\n";
			throw "recv failed. ret is not WSA_IO_PENDING";
			break;
		}
	}

	void Client::send() {
		// TODO
		throw "No implement function";
	}

	void Client::onRecv(unsigned int length) {
		logger->debug() << "onRecv()\n";

		recvOverlapped.currentBufferSize += length;

		logger->debug() << "Receive info...\n"
			            << "  received size: " << length << "\n"
						<< "  buffer size: " << recvOverlapped.currentBufferSize << "\n";

		// TODO : process protocol
	}

	void Client::onSend(unsigned int length) {
		// TODO
		throw "No implement function";
	}
}