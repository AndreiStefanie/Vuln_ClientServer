#define _CRT_SECURE_NO_WARNINGS
#include "commands.h"
#include <stdio.h>
#include "data.h"
#include "server.h"

int
ExceptionFilter(
	DWORD Code,
	EXCEPTION_POINTERS *ExceptionPointers
)
{
	printf("[EXCEPTION] ExceptionCode=0x%x, EIP=%p, ESP=%p\n", Code, ExceptionPointers->ContextRecord->Eip, ExceptionPointers->ContextRecord->Esp);
	Log("[EXCEPTION] ExceptionCode=0x%x, EIP=%p, ESP=%p\n", Code, ExceptionPointers->ContextRecord->Eip, ExceptionPointers->ContextRecord->Esp);

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
	Output[*OutLength - 1] = ' ';   //
	Output[*OutLength] = 0;         //
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

	//printf("Success!\n");

	sprintf_s(Output, DEFAULT_BUFLEN, "[OK]\n");
	*OutLength = strlen(Output);

	*OutLength += fread_s(&Output[*OutLength], DEFAULT_BUFLEN, 1, DEFAULT_BUFLEN, file);

	///
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
	_In_ char *Message
)
{
	FILE *file;
	char filename[64];

	sprintf_s(filename, 64, ".\\%d\\%s", UserId, Message);

	fopen_s(&file, filename, "w");
	if (NULL == file)
	{
		printf("File &s could not be created.\n", filename);
		return;
	}

	fclose(file);
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
	char filename[64];

	// Search for the created file in the user's directory
	sprintf_s(folder, 20, ".\\%d\\*", UserId);
	printf("Searching in folder: %s\n", folder);

	hFind = FindFirstFileA(folder, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("Could not find user's file. Error: (%d)\n", GetLastError());
		return;
	}
	FindClose(hFind);
	
	sprintf_s(filename, 64, ".\\%d\\%s", UserId, FindFileData.cFileName);
	fopen_s(&file, filename, "a");
	if (NULL == file)
	{
		printf("File &s could not be opened.\n", filename);
		return;
	}

	fwrite(Message, sizeof(char), sizeof(Message) / sizeof(char), file);

	fclose(file);
}


//
// Encrypts the file given as input parameter
//
void CmdHandleEncryptFile(
	int UserId,
	_In_ char *Message
)
{
	


}
