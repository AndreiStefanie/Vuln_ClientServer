#define _CRT_SECURE_NO_WARNINGS
#include "commands.h"
#include <stdio.h>
#include "data.h"
#include "server.h"
#include "map.h"
#include <process.h>


struct threadData {
	MAPPING map;
	size_t startIndex;
	size_t count;
};


const char *cryptKey = "123";
volatile long cryptProgress = 0;


static char *xor(
	_In_reads_bytes_(count) char *string,
	size_t count,
	_In_ const char *key
	);

unsigned ComputeNrOfThreads(size_t FileSize);
unsigned WINAPI CryptoThreadFunc(void* Arguments);


int
ExceptionFilter(
	DWORD Code,
	EXCEPTION_POINTERS *ExceptionPointers
)
{
	printf("[EXCEPTION] ExceptionCode=0x%x, EIP=%d, ESP=%d\n",
		Code,
		ExceptionPointers->ContextRecord->Eip,
		ExceptionPointers->ContextRecord->Esp);
	Log("[EXCEPTION] ExceptionCode=0x%x, EIP=%d, ESP=%d\n",
		Code,
		ExceptionPointers->ContextRecord->Eip,
		ExceptionPointers->ContextRecord->Esp);

	return EXCEPTION_EXECUTE_HANDLER;
}


BOOLEAN
CmdHandleUser(
	char* User,
	int *UserId
)
{
	DWORD i;

	__try
	{
		//
		// Now search if the given username exists
		// 
		for (i = gUserCount - 1; i >= 0; i--)
		{
			if (0 == _stricmp(gUserData[i].Username, User))
			{
				*UserId = i;
				return TRUE;
			}
		}
	}
	__except (ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
	{
	}

	*UserId = -1;
	return FALSE;
}


BOOLEAN
CmdHandlePass(
	int UserId,
	char *Password
)
{
	char tempPass[DEFAULT_MAX_PASS_LEN];

	tempPass[0] = '\0';
	strcpy(tempPass, Password);

	if (0 == strcmp(gUserData[UserId].Password, tempPass))
	{
		return TRUE;
	}

	return FALSE;
}


void
_AddFileToOutput(
	char *File,
	char *Output,
	int *OutLength
)
{
	int len = strlen(File);

	// Add file name to Output, but replace '.txt' with ' '
	strcat_s(Output, DEFAULT_BUFLEN, File);
	*OutLength += len - 3;
	Output[*OutLength - 1] = ' ';
	Output[*OutLength] = 0;
}


BOOLEAN
CmdHandleInfo(
	int UserId,
	char *Details,
	char *Output,
	int *OutLength
)
{
	DWORD param = 1;
	DWORD fields;
	WORD size;
	BOOLEAN ret = TRUE;
	int i;

	__try
	{
		fields = sscanf_s(Details, "%d", &param);           // Get the number of fields to return
		size = (WORD)(fields * param * FIELD_SIZE);         // 'fields' should be 1 (one field identified by sscanf_s), 'param' - number of fields to return
															// If either of those is 0, nothing will be returned
		if (param > 3)
		{
			//
			// Make sure we don't return more than needed (size will be maximum 3 * FIELD_SIZE = 75)
			//
			ret = FALSE;
			__leave;
		}

		*OutLength = size;
		printf("size = %d\n", size);

		//
		// Copy in Output the computed size
		//
		for (i = 0; i < size; i++)
		{
			Output[i] = ((char*)(&gUserData[UserId]))[i];
		}

		ret = TRUE;
	}
	__except (ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
	{
	}

	return ret;
}


void
CmdHandleList(
	int UserId,
	char *Output,
	int *OutLength
)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	char folder[32];

	sprintf_s(folder, 20, ".\\%d\\*.txt", UserId);
	printf("Searching in folder: %s\n", folder);

	hFind = FindFirstFileA(folder, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return;
	}

	//printf ("The first file found is %s\n", FindFileData.cFileName);
	_AddFileToOutput(FindFileData.cFileName, Output, OutLength);

	while (FindNextFileA(hFind, &FindFileData))
	{
		//printf ("The next file found is %s\n", FindFileData.cFileName);
		_AddFileToOutput(FindFileData.cFileName, Output, OutLength);
	}

	//printf("That's it\n");

	FindClose(hFind);
}


void
CmdHandleGet(
	int UserId,
	char *Message,
	char *Output,
	int *OutLength
)
{
	FILE *file;
	char filename[64];

	sprintf_s(filename, 64, ".\\%d\\%s.txt", UserId, Message);

	printf("Opening file: %s\n", filename);

	fopen_s(&file, filename, "r");
	if (NULL == file)
	{
		sprintf_s(Output, DEFAULT_BUFLEN, "[ERROR] Message not found.");
		*OutLength = strlen(Output);

		printf("Error opening message file\n");
		return;
	}

	sprintf_s(Output, DEFAULT_BUFLEN, "[OK]\n");
	*OutLength = strlen(Output);

	*OutLength += fread_s(&Output[*OutLength], DEFAULT_BUFLEN, 1, DEFAULT_BUFLEN, file);

	fclose(file);
}

//
// New commands
//

//
// Creates a new file in the user's directory. File name is given as input parameter.
//
void
CmdHandleNewFile(
	int UserId,
	_In_ char *Message,
	_Out_ char *Output,
	_Out_ size_t *OutLength
)
{
	FILE *file;
	char filename[DEFAULT_MAX_FILENAME];

	if ((NULL != strstr(Message, "..")) ||
		(NULL != strstr(Message, "CON")) ||
		(NULL != strstr(Message, "\\")))
	{
		sprintf_s(Output, DEFAULT_BUFLEN, "[ERROR] Filename contains illegal characters.");
		*OutLength = strlen(Output);

		printf("Filename contains illegal characters.");
		return;
	}

	sprintf_s(filename, DEFAULT_MAX_FILENAME, ".\\%d\\%s", UserId, Message);

	fopen_s(&file, filename, "w");
	if (NULL == file)
	{
		sprintf_s(Output, DEFAULT_BUFLEN, "[ERROR] File could not be created.");
		*OutLength = strlen(Output);

		printf("File %s could not be created.\n", filename);
		return;
	}

	fclose(file);

	sprintf_s(Output, DEFAULT_BUFLEN, "[OK] File created.");
	*OutLength = strlen(Output);
}


//
// Writes the given data into the first file from the user's directory.
//
void CmdHandleWriteFile(
	int UserId,
	_In_ char *Message
)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	char folder[32];
	FILE *file;
	char filename[DEFAULT_MAX_FILENAME];

	// Search for the created file in the user's directory
	sprintf_s(folder, 20, ".\\%d\\*.txt", UserId);
	printf("Searching in folder: %s\n", folder);

	hFind = FindFirstFileA(folder, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("Could not find user's file. Error: %d\n", GetLastError());
		return;
	}
	FindClose(hFind);

	// Open the (found) user's file for writing
	sprintf_s(filename, DEFAULT_MAX_FILENAME, ".\\%d\\%s", UserId, FindFileData.cFileName);
	fopen_s(&file, filename, "w");
	if (NULL == file)
	{
		printf("File %s could not be opened.\n", filename);
		return;
	}

	fwrite(Message, sizeof(char), strlen(Message), file);

	fclose(file);
}


//
// Encrypts the file given as input parameter
//
void CmdHandleEncryptFile(
	int UserId,
	_In_ char *Message,
	_Out_ char *Output,
	_Out_ size_t *OutLength
)
{
	char filename[DEFAULT_MAX_FILENAME];
	MAPPING map;
	DWORD result;
	HANDLE hThreads[DEFAULT_MAX_THREADS];
	unsigned threadIDs[DEFAULT_MAX_THREADS];
	unsigned threadNr = 1;
	struct threadData threadArg[DEFAULT_MAX_THREADS];

	if ((NULL != strstr(Message, "..")) ||
		(NULL != strstr(Message, "CON")) ||
		(NULL != strstr(Message, "\\")))
	{
		sprintf_s(Output, DEFAULT_BUFLEN, "[ERROR] Filename contains illegal characters.");
		*OutLength = strlen(Output);

		printf("Filename contains illegal characters.");
		return;
	}

	sprintf_s(filename, DEFAULT_MAX_FILENAME, ".\\%d\\%s", UserId, Message);

	result = MapFile(filename, GENERIC_READ | GENERIC_WRITE, &map);
	if (ERROR_SUCCESS != result)
	{
		sprintf_s(Output, DEFAULT_BUFLEN, "[ERROR] Could not map user's file.");
		*OutLength = strlen(Output);

		printf("Could not map user's file. Error: %d\n", GetLastError());
		return;
	}

	// Create the threads and split the work
	cryptProgress = 0;
	threadNr = ComputeNrOfThreads(map.DataSize);

	printf("Starting encryption with %u threads.\n", threadNr);
	for (size_t i = 0; i < threadNr; i++)
	{
		threadArg[i].map = map;
		threadArg[i].startIndex = WORKLOAD_PER_THREAD * i;
		threadArg[i].count = WORKLOAD_PER_THREAD;
		hThreads[i] = (HANDLE)_beginthreadex(NULL, 0, &CryptoThreadFunc, &threadArg[i], 0, &threadIDs[i]);
	}

	WaitForMultipleObjects(threadNr, hThreads, TRUE, 5000);
	for (size_t i = 0; i < threadNr; i++)
	{
		//WaitForSingleObject(hThreads[0], INFINITE);
		CloseHandle(hThreads[i]);
	}

	UnmapFile(&map);

	printf("Progress after encryption: %d.\n", cryptProgress);

	sprintf_s(Output, DEFAULT_BUFLEN, "[OK] File encrypted.");
	*OutLength = strlen(Output);
}


//
// Decide the number of threads based on the file size
//
unsigned ComputeNrOfThreads(size_t FileSize)
{
	unsigned exactDiv;

	exactDiv = FileSize % WORKLOAD_PER_THREAD ? 1 : 0;
	return FileSize / WORKLOAD_PER_THREAD + exactDiv;
}


//
// Thread function responsible for encryption of the map portion
// assigned to the given thread
//
unsigned WINAPI CryptoThreadFunc(void* Arguments)
{
	struct threadData *mapInfo;

	mapInfo = (struct threadData*) Arguments;
	char *submap = mapInfo->map.Data + mapInfo->startIndex;

	submap = xor (submap, WORKLOAD_PER_THREAD, cryptKey);

	_endthreadex(0);
	return EXIT_SUCCESS;
}


//
// Basic XOR encryption
//
static char *xor(
	_In_reads_bytes_(count) char *string,
	size_t count,
	_In_ const char *key
	)
{
	size_t keyLength = 0;
	size_t i = 0;
	char *s;

	keyLength = strlen(key);
	s = string;

	while (*s && (i < count))
	{
		*s++ ^= key[i++ % keyLength];
		if (0 == i % DEFAULT_PROGRESS_INC)
		{
			InterlockedAdd(&cryptProgress, (long)DEFAULT_PROGRESS_INC);
		}
	}

	InterlockedAdd(&cryptProgress, (long)(i % DEFAULT_PROGRESS_INC));

	return string;
}
