#include "Logger.h"
#include "IOCP.h"

int main(void)
{
	phodobit::Logger* logger = phodobit::Logger::getLogger("main")->setLogLevel(phodobit::LogLevel::DEBUG);
	logger->debug() << "main()\n";

	logger->info() << "IOCP Base starting\n";

	phodobit::IOCP *iocp = new phodobit::IOCP();
	iocp->initialize();
	iocp->bind(9999, 5);

	logger->info() << "IOCP Base started\n";

	return 0;
}
