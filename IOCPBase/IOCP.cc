#include "IOCP.h"

namespace phodobit {
	Logger* IOCP::logger = Logger::getLogger("IOCP")->setLogLevel(LogLevel::DEBUG);

	void IOCP::initialize() {
		logger->debug() << "initialize()\n";

		WSAData wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			logger->err() << "WSAStartup failed.\n";
			throw "WSAStartup failed.";
		}

		// ���ο� IOCP �����ؾ��ϹǷ� �������� �ʴ� �ڵ� ����
		iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (iocpHandle == nullptr) {
			logger->err() << "CreateIoCompletionPort failed.\n";
			throw "CreateIoCompletionPort failed.";
		}
	}

	void IOCP::bind(unsigned short port, int backlog = 5) {
		logger->debug() << "bind()\n";

		SOCKET socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN socketAddrIn;
		memset(&socketAddrIn, 0, sizeof(socketAddrIn));
		socketAddrIn.sin_family = AF_INET;
		socketAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
		socketAddrIn.sin_port = htons(port);

		// IOCP �ڵ鿡 ������ ���� ������� �߰�
		// TODO : ���� listen ~ craeteIocp �����϶�, �� ���̿� Ŭ�� connect ������,
		//        �̺�Ʈ�� ���������� ������ �� �ִ����� ���ؼ� �����غ���.
		CreateIoCompletionPort((HANDLE)socket, iocpHandle, 0, 0);

		int ret;

		ret = ::bind(socket, (sockaddr*)&socketAddrIn, sizeof(socketAddrIn));
		if (ret == SOCKET_ERROR) {
			logger->err() << "bind failed.\n";
			throw "bind failed.";
		}

		ret = listen(socket, backlog);
		if (ret == SOCKET_ERROR) {
			logger->err() << "listen failed.\n";
			throw "listen failed.";
		}
	}
}