# Windows Rootkit – DKOM Process Hider

## Overview

This project is a basic Windows kernel rootkit demonstrating **Direct Kernel Object Manipulation (DKOM)** to hide a user-mode process based on its PID.

It works by modifying internal kernel structures (`EPROCESS.ActiveProcessLinks`) to remove the target process from the system’s active process list, making it invisible to enumeration tools like Task Manager or Process Hacker — while the process continues running.

**Educational use only — Do not run this on production systems or any machine you do not fully control.**

## Features

- Windows Kernel Driver (WDM)
- Uses DKOM to unlink a process from the active process list
- Communicates with user-mode app via IOCTL (DeviceIoControl)
- User-mode app hides the current process
- Tested on Windows 11

## How It Works

1. The kernel driver:
   - Locates the `EPROCESS` structure of the specified PID
   - Traverses the `ActiveProcessLinks` list
   - Modifies the `Flink` and `Blink` pointers to unlink the process

2. The user-mode app:
   - Retrieves its own PID
   - Sends the PID to the kernel driver
   - Receives confirmation of success/failure

## Project Structure

| File/Folder                   | Description                                      |
|------------------------------|--------------------------------------------------|
| `Rootkit.c`                  | Kernel-mode driver source code                   |
| `Header.h`                   | Shared header with IOCTL definitions             |
| `RootKitUserApp.cpp`         | User-mode application to trigger process hiding  |
| `README.md`                  | This file                                        |

## Usage

### Build and Load the Driver

1. Open the driver project in Visual Studio with WDK installed.
2. Compile in **x64 Debug** mode.
3. Disable driver signature enforcement or enable test signing.
4. Load the driver using the following commands:

```cmd
sc create RootKit type= kernel binPath= C:\Path\To\Rootkit.sys
sc start RootKit
```

### Run the User-Mode App

1. Compile the `RootKitUserApp.cpp` in Visual Studio.
2. Run the resulting executable as Administrator.
3. The app will:
   - Print its own PID
   - Wait for a key press
   - Send the PID to the kernel driver
   - Report whether the process was hidden

### Test Result

- Before hiding, locate your process in Task Manager or Process Hacker.
- After hiding, it should disappear but remain running.

## Windows Compatibility

This project is built and tested on **Windows 11**.

To use it on other Windows versions (e.g., Windows 10), update the `EPROCESS` offsets in the kernel driver Header:

```c
// change according to windows version..
#define UniqueProcessIdOffset		(UINT64)0x1d0
#define ActiveProcessLinksOffset	(UINT64)0x1d8
```

These values can vary by build. Use WinDbg or Rekall to determine the correct offsets.

## Technical Notes

- Device name: `\Device\RootKitDevice`
- Symbolic link: `\??\RootKitSym`
- Custom IOCTL: `RK_CTL`
- The rootkit does not terminate the process; it only hides it from enumeration

## Disclaimer

This project is for **educational and ethical use only**.  
Do not use this code on systems you do not own or without explicit permission.  
Run it only in isolated, virtualized test environments.  
The author is not responsible for any misuse or damage caused.

## Author

**Omar Shehata**  
GitHub: [https://github.com/OmarShehata11](https://github.com/OmarShehata11)
