// rc_test.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//
#include <Windows.h>

int main(int argc, char* argv[])
{
	HINSTANCE handleProcessDll = LoadLibrary(L"remote_call.dll");
	FARPROC getProccessId = GetProcAddress(HMODULE(handleProcessDll), "RVExtension");

	typedef void(__stdcall * extCall)(char *output, int outputSize, const char *function);

	extCall RVExtension;
	RVExtension = extCall(getProccessId);

	char output[1000];
	RVExtension(output, sizeof(output), "0");

	FreeLibrary(handleProcessDll);

	return 0;
}

