#pragma once

#include "stddefs.h"

#pragma pack(1)
#pragma byte_order(BigEndian)

enum COP<BYTE>
{
	cop_Syn	= 0,	
	cop_Ack	= 1,	
	cop_Data = 2,	
	cop_Fin	= 3
};

enum ERROR<DWORD>
{
	error_Ok,
	error_Len,
	error_Unknown,
	error_NoMemory
};

[category(protocol), display(title)] public struct TUP
{
	DWORD len;
	DWORD ver;
	DWORD headerCrc;
	DWORD j;
	COP cop;
	switch (cop)
	{
		case cop_Syn:
			DWORD windowSize;
			break;
			
		case cop_Ack:
			ERROR error;
			break;
			
		case cop_Data:
			BYTE end;
			BYTE payload[len - 6];
	}
	DWORD bodyCrc;
};