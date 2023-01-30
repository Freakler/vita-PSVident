#include <stdio.h> 
#include <string.h>

#include "registry.h"


int getRegistryInteger(int* result, const char* reg, const char* key) { ///type02 - int
	return sceRegMgrGetKeyInt(reg, key, result);
}

int getRegistryString(char *result_str, const char* reg, const char* key) { ///type03 - string
	return sceRegMgrGetKeyStr(reg, key, result_str, 2048);  //todo sizeof result_str
}

int getRegistryBinary(char *result_str, const char* reg, const char* key) { ///type04 - binary
	return sceRegMgrGetKeyBin(reg, key, result_str, 2048);  //todo sizeof result_str
}

/*
	Errors from SceMgrGetKey
	0x800D000E - Not supported on this device
	0x800D000A - Path doesn't exist
	0x800D0014 - Key doesn't exist
	0x800D0015 - ?
	0x800D0017 - Something not found ex (/CONFIG/US_ENGLISH/input_type: 0x800D0017)
	0x800D000B - Wrong data type
	0x80022005 - No size provided
*/
