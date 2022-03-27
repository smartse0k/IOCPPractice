#include "IOCP.h"
#include "Overlapped.h"
#include "Client.h"

namespace phodobit {
	Logger* IOCP::logger = Logger::getLogger("IOCP")->setLogLevel(LogLevel::DEBUG);

	void IOCP::initialize() {
		logger->debug() << "initialize()\n";

		WSAData wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			logger->err() << "WSAStartup failed.\n";
			throw "WSAStartup failed.";
		}

		// 새로운 IOCP 생성해야하므로 존재하지 않는 핸들 전달
		iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (iocpHandle == nullptr) {
			logger->err() << "CreateIoCompletionPort failed.\n";
			throw "CreateIoCompletionPort failed.";
		}

		_nextCompletionKey = 0;
	}

	void IOCP::bind(unsigned short port) {
		logger->debug() << "bind()\n";

		this->port = port;

		serverSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN socketAddrIn;
		memset(&socketAddrIn, 0, sizeof(socketAddrIn));
		socketAddrIn.sin_family = AF_INET;
		socketAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
		socketAddrIn.sin_port = htons(port);

		// IOCP 핸들에 소켓을 감시 대상으로 추가
		// TODO : 만약 listen ~ craeteIocp 순서일때, 이 사이에 클라가 connect 했을때,
		//        이벤트를 정상적으로 수신할 수 있는지에 대해서 조사해볼것.
		CreateIoCompletionPort((HANDLE)socket, iocpHandle, 0, 0);

		int ret = ::bind(serverSocket, (sockaddr*)&socketAddrIn, sizeof(socketAddrIn));
		if (ret == SOCKET_ERROR) {
			logger->err() << "bind failed.\n";
			throw "bind failed.";
		}
	}

	void IOCP::listen(int backlog) {
		int ret = ::listen(serverSocket, backlog);
		if (ret == SOCKET_ERROR) {
			logger->err() << "listen failed.\n";
			throw "listen failed.";
		}

		logger->info() << "listening on port " << port << ".\n";
	}

	int IOCP::getNextCompletionKey() {
		mutexGetNextCompletionKey.lock();
		_nextCompletionKey++;
		mutexGetNextCompletionKey.unlock();
		return _nextCompletionKey;
	}

	// Acceptor Thread Entry Point
	void IOCP::acceptor() {
		while (true) {
			SOCKET clientSocket;
			SOCKADDR_IN socketAddrIn;
			int socketAddrInSize = sizeof(socketAddrIn);

			memset(&socketAddrIn, 0, socketAddrInSize);

			clientSocket = accept(serverSocket, (SOCKADDR *)&socketAddrIn, &socketAddrInSize);

			int nextCompletionKey = getNextCompletionKey();

			Client* client = new Client(clientSocket, nextCompletionKey);
			Client::setClient(nextCompletionKey, client);
			client->bind(iocpHandle);
			client->recv();
		}
	}

	// Worker Thread Entry Point
	void IOCP::worker() {
		DWORD threadId = GetCurrentThreadId();

		while (true) {
			DWORD transferByteSize;
			int completionKey = -1;
			Overlapped *overlapped = nullptr;
			
			bool ret = GetQueuedCompletionStatus(
				iocpHandle,
				&transferByteSize,
				(ULONG_PTR*)&completionKey,
				(LPOVERLAPPED *)&overlapped,
				INFINITE
			);

			logger->debug() << "transferByteSize: " << transferByteSize << "\n";

			Client *client = Client::getClient(completionKey);
			if (client == nullptr) {
				logger->err() << "client is null\n";
				continue;
			}

			if (overlapped->type == Overlapped::TYPE::RECV) {
				client->onRecv(transferByteSize);
				client->recv();
			} else if (overlapped->type == Overlapped::TYPE::SEND) {
				// TODO : Send에 대해서 만들자
			} else {
				logger->err() << "not initilized overlapped detected. threadId=" << threadId << ", completionKey=" << completionKey;
				continue;
			}
		}
	}

	void IOCP::createAcceptThread(int threadCount) {
		logger->debug() << "createAcceptThread()\n";

		std::thread *thread;
		for (int i = 0; i < threadCount; i++) {
			thread = new std::thread(&IOCP::acceptor, this);
			acceptThreads.push_back(thread);
		}

		logger->info() << threadCount << " of accept thread are created.\n";
	}

	void IOCP::createWorkerThread(int threadCount) {
		logger->debug() << "createWorkerThread()\n";

		std::thread* thread;
		for (int i = 0; i < threadCount; i++) {
			thread = new std::thread(&IOCP::worker, this);
			workerThreads.push_back(thread);
		}

		logger->info() << threadCount << " of work thread are created.\n";
	}
}