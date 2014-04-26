#include <vector>
#include <thread>
#include <memory>
#include <iostream>
#include <fstream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include "RemoteCall\RemoteCall.hpp"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3310"

RemoteCall rCall;

extern "C" {
	__declspec (dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function);
}

std::string getProfileFolder() {
	std::string profileFolder = "";
	LPTSTR cmdLine = GetCommandLine();
	int numCmdLineArgs = 0;
	LPTSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &numCmdLineArgs);

	std::vector<std::string> commandLine;
	commandLine.reserve(numCmdLineArgs);

	for (int i = 0; i < numCmdLineArgs; i++) {
		std::wstring args(cmdLineArgs[i]);
		std::string utf8(args.begin(), args.end());
		commandLine.push_back(utf8);
	}

	for (std::vector<std::string>::iterator it = commandLine.begin(); it != commandLine.end(); it++) {
		std::string starter = "-profiles=";
		if (it->length() < starter.length()) {
			continue;
		}

		std::string compareMe = it->substr(0, starter.length());
		if (compareMe.compare(starter) != 0) {
			continue;
		}

		profileFolder = it->substr(compareMe.length());
	}

	return profileFolder;
}
int getServerPort() {
	std::string profileFolder = "";
	LPTSTR cmdLine = GetCommandLine();
	int numCmdLineArgs = 0;
	LPTSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &numCmdLineArgs);

	std::vector<std::string> commandLine;
	commandLine.reserve(numCmdLineArgs);

	for (int i = 0; i < numCmdLineArgs; i++) {
		std::wstring args(cmdLineArgs[i]);
		std::string utf8(args.begin(), args.end());
		commandLine.push_back(utf8);
	}

	for (std::vector<std::string>::iterator it = commandLine.begin(); it != commandLine.end(); it++) {
		std::string starter = "-port=";
		if (it->length() < starter.length()) {
			continue;
		}

		std::string compareMe = it->substr(0, starter.length());
		if (compareMe.compare(starter) != 0) {
			continue;
		}

		profileFolder = it->substr(compareMe.length());
	}

	return atoi(profileFolder.c_str());
}

void __stdcall RVExtension(char *output, int outputSize, const char *function) {
	if (strcmp(function, "Init") == 0) {
		rCall.initServer();
		strncpy(output, "diag_log 'remote_call startup';", outputSize);
	}
	else if (strcmp(function, "WorkerStack") == 0) {
		std::string sqf = rCall.getStackItem();
		strncpy_s(output, outputSize, sqf.c_str(), _TRUNCATE);
	}
	else if (strcmp(function, "WorkerStackCount") == 0) {
		//strncpy_s(output, outputSize, sqf.c_str(), _TRUNCATE);
	}
}