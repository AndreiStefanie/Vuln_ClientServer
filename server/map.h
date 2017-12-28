#pragma once

#include <Windows.h>

typedef unsigned long long QWORD;

typedef struct _MAPPING
{
	HANDLE			FileHandle;
	HANDLE			MapHandle;
	QWORD			DataSize;
	PBYTE			Data;
} MAPPING, *PMAPPING;

_Success_(return == ERROR_SUCCESS)
DWORD
MapFile(
	_In_	PCSTR		Filename,
	_In_	DWORD		AccessRights,
	_Out_	MAPPING*	Mapping
);

void
UnmapFile(
	_In_	MAPPING*	Mapping
);