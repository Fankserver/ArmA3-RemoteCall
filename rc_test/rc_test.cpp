#define LOGCOUT
#include "../remote_call/RemoteCall/RemoteCall.hpp"

RemoteCall rc;

void main(void) {
	rc.initServer();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
		std::string stackItem = rc.getStackItem(4096);
		if (stackItem.size() > 2) {
			std::cout << "stackdone" << std::endl;
			//std::cout << stackItem << std::endl;
		}
	}

	getchar();
}