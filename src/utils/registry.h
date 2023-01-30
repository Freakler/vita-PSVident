#ifndef __REGISTRY_H__
#define __REGISTRY_H__

int sceRegMgrGetRegVersion(int version, char * buf) ;	
int sceRegMgrGetKeyInt(const char* category, const char* name, int* buf); //type02
int sceRegMgrGetKeyStr(const char* category, const char* name, char* buf, const int size); //type03
int sceRegMgrGetKeyBin(const char* category, const char *name, char *buf, const int size); //type04
 
int getRegistryInteger(int *result, const char* reg, const char* key);
int getRegistryString(char *result_str, const char* reg, const char* key );
int getRegistryBinary(char *result_str, const char* reg, const char* key );

#endif