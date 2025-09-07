#pragma once

typedef struct _KLDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    VOID* ExceptionTable;
    ULONG ExceptionTableSize;
    VOID* GpValue;
    VOID* NonPagedDebugInfo;
    VOID* DllBase;
    VOID* EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    union
    {
        USHORT SignatureLevel : 4;
        USHORT SignatureType : 3;
        USHORT Frozen : 2;
        USHORT HotPatch : 1;
        USHORT Unused : 6;
        USHORT EntireField;
    } u1;
    VOID* SectionPointer;
    ULONG CheckSum;
    ULONG CoverageSectionSize;
    VOID* CoverageSection;
    VOID* LoadedImports;
    union
    {
        VOID* Spare;
        VOID* NtDataTableEntry;
    };
    ULONG SizeOfImageNotRounded;
    ULONG TimeDateStamp;
}KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;