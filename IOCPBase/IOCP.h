#pragma once

#include <winsock2.h>
#include <vector>
#include <thread>
#include <mutex>

#include "Logger.h"

namespace phodobit {
	class IOCP {
	public:
		void initialize();
		void bind(unsigned short port);
		void listen(int backlog = 5);
		int getNextCompletionKey();
		void createAcceptThread(int threadCount = 1);
		void createWorkerThread(int threadCount = 2);
	private:
		static Logger* logger;

		HANDLE iocpHandle;
		SOCKET serverSocket;
		unsigned short port;
		std::vector<std::thread*> acceptThreads;
		std::vector<std::thread*> workerThreads;

		std::mutex mutexGetNextCompletionKey;

		void acceptor();
		void worker();
		
		int _nextCompletionKey;
	};
}