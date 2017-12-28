#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <windows.h>

BOOLEAN
CmdHandleUser(
	char* User,
	int *UserId
);


BOOLEAN
CmdHandlePass(
	int UserId,
	char *Password
);

void
CmdHandleList(
	int UserId,
	char *Output,
	int *OutLength
);

BOOLEAN
CmdHandleInfo(
	int UserId,
	char *Details,
	char *Output,
	int *OutLength
);

void
CmdHandleGet(
	int UserId,
	char *Message,
	char *Output,
	int *OutLength
);

void
CmdHandleNewFile(
	int UserId,
	_In_ char *Message,
	_Out_ char *Output,
	_Out_ size_t *OutLength
);

void CmdHandleWriteFile(
	int UserId,
	_In_ char *Message
);

void CmdHandleEncryptFile(
	int UserId,
	_In_ char *Message,
	_Out_ char *Output,
	_Out_ size_t *OutLength
);

#endif _COMMANDS_H_
