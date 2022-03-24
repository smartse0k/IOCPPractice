#pragma once

#include <winsock2.h>

#include "Logger.h"

namespace phodobit {
	class IOCP {
	public:
		void initialize();
		void bind(unsigned short port, int backlog);
	private:
		static Logger* logger;

		HANDLE iocpHandle;
	};
}