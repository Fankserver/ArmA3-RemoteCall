#define LOGCOUT
#include "../remote_call/RemoteCall/Server.hpp"

RemoteCall::Server rc;

void work() {
	std::string stackItem = rc.getStackItem(4096);
	if (stackItem.size() > 2) {
		std::cout << "stackdone" << std::endl;
		//std::cout << stackItem << std::endl;
	}
}

void main(void) {
	rc.initServer();
	while (true) {
		std::thread asd(work);
		asd.join();
	}

	getchar();
}