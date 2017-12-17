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
	char *Message
);

void CmdHandleWriteFile(
	int UserId,
	char *Message
);

void CmdHandleEncryptFile(
	int UserId,
	char *Message
);

#endif _COMMANDS_H_
