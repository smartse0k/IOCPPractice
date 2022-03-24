#pragma once

#include <map>
#include <winsock2.h>

#include "Logger.h"

namespace phodobit {
	class Client {
	public:
		static Client* getClient(int completionKey);
		static void setClient(int completionKey, Client*);

		Client(SOCKET socket, int completionKey);
		void bind(HANDLE iocpHandle);
		void recv();
	private:
		static Logger* logger;

		static std::map<int, Client*> *clients;

		SOCKET socket;
		int completionKey;

		WSABUF wsaBuf;
		char recvBuf[8 * 1024];
		DWORD readSize;
		DWORD readFlag;
		WSAOVERLAPPED wsaOverlapped;
	};
}