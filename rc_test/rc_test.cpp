#include "../remote_call/RemoteCall/RemoteCall.hpp"

RemoteCall rc;

void main(void) {
	rc.initServer();
	getchar();
}