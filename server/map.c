#include "map.h"

_Success_(return == ERROR_SUCCESS)
DWORD
MapFile(
	_In_	PCSTR		Filename,
	_In_	DWORD		AccessRights,
	_Out_	MAPPING*	Mapping
)
{
	DWORD result;
	DWORD flProtect;
	DWORD mapAccessRights;

	if (NULL == Filename)
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (NULL == Mapping)
	{
		return ERROR_INVALID_PARAMETER;
	}

	result = ERROR_SUCCESS;
	Mapping->FileHandle = INVALID_HANDLE_VALUE;
	Mapping->MapHandle = NULL;
	Mapping->DataSize = 0;
	Mapping->Data = NULL;

	if (0 != (GENERIC_WRITE & AccessRights))
	{
		flProtect = PAGE_READWRITE;
		mapAccessRights = FILE_MAP_READ | FILE_MAP_WRITE;
	}
	else
	{
		flProtect = PAGE_READONLY;
		mapAccessRights = FILE_MAP_READ;
	}

	__try
	{
		Mapping->FileHandle = CreateFileA(Filename,
			AccessRights,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (INVALID_HANDLE_VALUE == Mapping->FileHandle)
		{
			result = GetLastError();
			__leave;
		}

		Mapping->MapHandle = CreateFileMapping(Mapping->FileHandle,
			NULL,
			flProtect,
			0,
			0,
			NULL);
		if (NULL == Mapping->MapHandle)
		{
			result = GetLastError();
			__leave;
		}

		Mapping->Data = MapViewOfFile(Mapping->MapHandle,
			mapAccessRights,
			0,
			0,
			0);
		if (NULL == Mapping->Data)
		{
			result = GetLastError();
			__leave;
		}


		if (!GetFileSizeEx(Mapping->FileHandle, &Mapping->DataSize))
		{
			result = GetLastError();
			__leave;
		}
	}
	__finally
	{
		if (ERROR_SUCCESS != result)
		{
			UnmapFile(Mapping);
		}
	}

	return result;
}

void
UnmapFile(
	_In_	MAPPING*	Mapping
)
{
	if (NULL == Mapping)
	{
		return;
	}

	Mapping->DataSize = 0;

	if (NULL != Mapping->Data)
	{
		UnmapViewOfFile(Mapping->Data);
		Mapping->Data = NULL;
	}

	if (NULL != Mapping->MapHandle)
	{
		CloseHandle(Mapping->MapHandle);
		Mapping->MapHandle = NULL;
	}

	if (INVALID_HANDLE_VALUE != Mapping->FileHandle)
	{
		CloseHandle(Mapping->FileHandle);
		Mapping->FileHandle = INVALID_HANDLE_VALUE;
	}
}