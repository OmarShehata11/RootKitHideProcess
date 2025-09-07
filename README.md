# Windows Rootkit – DKOM Process/Driver Hider

## Overview

This project is a basic Windows kernel rootkit demonstrating **Direct Kernel Object Manipulation (DKOM)** to hide both a user-mode process (by PID) and the kernel driver itself.

- For process hiding, it modifies internal kernel structures (`EPROCESS.ActiveProcessLinks`) to unlink the target process from the system’s active process list, making it invisible to enumeration tools like Task Manager or Process Hacker — while the process continues running.
- For driver hiding, it unlinks the driver’s loader entry (`KLDR_DATA_TABLE_ENTRY.InLoadOrderLinks`) so it does not appear in typical module/driver enumeration lists.

**Educational use only — Do not run this on production systems or any machine you do not fully control.**

## Features

- Windows Kernel Driver (WDM)
- Uses DKOM to unlink a process from the active process list
- Hides the kernel driver by unlinking `KLDR_DATA_TABLE_ENTRY.InLoadOrderLinks`
- Communicates with user-mode app via IOCTL (DeviceIoControl)
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

3. Driver hiding (core capability):
   - Identifies its loader entry (`KLDR_DATA_TABLE_ENTRY`) from `DriverObject->DriverSection`
   - Unlinks `InLoadOrderLinks` by fixing neighbors’ `Flink`/`Blink`
   - Self-points the driver’s list links so it no longer appears in typical loader/module enumerations

## Project Structure

| File/Folder                               | Description                                       |
|-------------------------------------------|---------------------------------------------------|
| `RootKitHideProcess/Source.cpp`           | Kernel-mode driver source code                    |
| `RootKitHideProcess/Header.h`             | Shared header with IOCTL and offset definitions   |
| `RootKitHideProcess/DriverHeader.h`       | Loader entry (`KLDR_DATA_TABLE_ENTRY`) definition |
| `RootKitTargetProcess/Source.cpp`         | User-mode application to trigger hide operations  |
| `README.md`                               | This file                                         |

## Usage

### Build and Load the Driver

1. Open the driver project (`RootKitHideProcess`) in Visual Studio with WDK installed.
2. Compile in **x64 Debug** mode.
3. Disable driver signature enforcement or enable test signing.
4. Load the driver using the following commands:

```cmd
sc create RootKit type= kernel binPath= C:\Path\To\Rootkit.sys
sc start RootKit
```

### Run the User-Mode App

1. Open and build the user app project (`RootKitTargetProcess`) in Visual Studio.
2. Run the resulting executable as Administrator.
3. The app will:
   - Print its own PID
   - Wait for your selection
   - Send the PID to the kernel driver to hide the process
   - Or request the driver to hide itself from module lists
   - Report whether the process was hidden

### Hide the Kernel Driver

- Trigger via the user-mode app option for driver hiding, which sends `RK_CTL_DRIVER_HIDE`.
- The driver unlinks its own loader entry (`InLoadOrderLinks`) so tools that enumerate loaded modules will not list it.

### Test Result

- Before process hiding, locate your process in Task Manager or Process Hacker.
- After process hiding, it should disappear but remain running.
- Before driver hiding, enumerate loaded drivers/modules using your preferred tool.
- After driver hiding, the driver should no longer appear in module/driver lists.

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
- Custom IOCTLs: `RK_CTL_PROCESS_HIDE`, `RK_CTL_DRIVER_HIDE`
- The rootkit does not terminate the process; it only hides it from enumeration

## Disclaimer

This project is for **educational and ethical use only**.  
Do not use this code on systems you do not own or without explicit permission.  
Run it only in isolated, virtualized test environments.  
The author is not responsible for any misuse or damage caused.

## Author

**Omar Shehata**  
GitHub: [https://github.com/OmarShehata11](https://github.com/OmarShehata11)
