#include <psp2/kernel/processmgr.h>
#include <psp2/vshbridge.h> 
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/sysmem.h> 
#include <psp2/io/fcntl.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/apputil.h>
#include <psp2/notificationutil.h>

#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "../main.h"
#include "../utils/aes.h"
#include "utils.h"


int logPrintf(char *file, const char *text, ...) {
	va_list list;
	char string[2048];

	va_start(list, text);
	vsprintf(string, text, list);
	va_end(list);

	char buffer[1024];
	sprintf(buffer, "%s", file);

	SceUID fd = sceIoOpen(buffer, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if( fd >= 0 ) {
		sceIoWrite(fd, string, strlen(string));
		sceIoWrite(fd, "\n", 1);
		sceIoClose(fd);
	}

	return 0;
}

// https://github.com/island-games/LibBartender/blob/main/main.c#L63
void ascii2utf(uint16_t* dst, char* src){ //Converts the char to UTF
	if( !src || !dst ) return;
	while(*src)*(dst++)=(*src++);
	*dst=0x00;
}
void sendNotification(char *text, ...) { 
	SceNotificationUtilProgressFinishParam param;
	memset(&param,0,sizeof(SceNotificationUtilProgressFinishParam));
	ascii2utf(param.notificationText, text);
	sceNotificationUtilSendNotification(param.notificationText);
}

void initSceAppUtil() {
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	memset(&init_param, 0, sizeof(SceAppUtilInitParam));
	memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);
}

void closeSceAppUtil() {
	sceAppUtilShutdown();
}

int initNet() {
	static void *net_memory = NULL;

	if( sceSysmoduleIsLoaded(SCE_SYSMODULE_HTTP) != SCE_SYSMODULE_LOADED ) 
		sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    if( sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED )
		sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	
    SceNetInitParam initparam;
    net_memory = malloc(1*1024*1024);
    initparam.memory = net_memory;
    initparam.size = NET_INIT_SIZE;
    initparam.flags = 0;
 
    int ret = sceNetInit(&initparam);
    if( ret < 0 ) { // Error
        free(net_memory);
        net_memory = NULL;
        return -1;
    } else { // Exit
        ret = sceNetCtlInit();
        if( ret < 0 && ret != NET_CTL_ERROR_NOT_TERMINATED ) { // Error
            sceNetTerm();
            free(net_memory);
            net_memory = NULL;
            return -2;
        }
    }
    return 0;
}

int doesFileExist(const char *file) {
  SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
  if( fd < 0 ) 
	  return -1;
  sceIoClose(fd);
  return 0;
}

//https://github.com/kakaroto/ps3xport/blob/master/tools.c
void aes256cbc(uint8_t *key, uint8_t *iv_in, uint8_t *in, uint16_t len, uint8_t *out) { 
	AES_KEY k;
	u32 i;
	u8 tmp[16];
	u8 iv[16];

	memcpy(iv, iv_in, 16);
	memset(&k, 0, sizeof k);
	AES_set_decrypt_key(key, 256, &k);

	while( len > 0 ) {
		memcpy(tmp, in, 16);
		AES_decrypt(in, out, &k);

		for( i = 0; i < 16; i++ )
			out[i] ^= iv[i];

		memcpy(iv, tmp, 16);

		out += 16;
		in += 16;
		len -= 16;

	}
}

// https://github.com/devnoname120/RegistryEditorMOD/blob/master/main.c#L203
unsigned char int2hex[] = "0123456789ABCDEF";
void convert2hex(unsigned char *buf, char *buf2, int len) { // buf2 needs to be at least 3x sizeof(buf)
	int i;
	for( i = 0; i < len; i++ ) {
		buf2[3*i+0] = int2hex[buf[i] >> 4];
		buf2[3*i+1] = int2hex[buf[i] & 0xf];
		buf2[3*i+2] = ' ';
	}
	buf2[3*i+0] = 0;
}

// thx TheFloW
void firmware_string(char string[8], unsigned int version) { // 0x1234ABCD
  char a = (version >> 24) & 0xf; // 2
  char b = (version >> 20) & 0xf; // 3
  char c = (version >> 16) & 0xf; // 4
  char d = (version >> 12) & 0xf; // A

  memset(string, 0, 8);
  string[0] = '0' + a;
  string[1] = '.';
  string[2] = '0' + b;
  string[3] = '0' + c;
  string[4] = '\0';

  if (d) { // eg: FW 1.69.1
    string[4] = '0' + d;
    string[5] = '\0';
  }
}

void baryon_string(char string[16], unsigned int version) { // eg: 0x0100060B -> 1.0.6.11
  char a = (version >> 24) & 0xff; // 0x01
  char b = (version >> 16) & 0xff; // 0x00
  char c = (version >> 8) & 0xff; // 0x06
  char d = (version >> 0) & 0xff; // 0x0B
  sprintf(string, "%d.%d.%d.%d", a, b, c, d);
}

void model_string(char out[32], char in[32]) { 
	// PCH01000ZA040000 -> PCH-1000 ZA04
	// VTE01001AB120000 -> VTE-1001 AB12
	// PCH01104ZA010003 -> PCH-1104 ZA01 v3 ?? todo
	// PTEL200200110000 -> PTEL-2002 0011
	// CPV02000ZA160000 -> CPV-2000 ZA16
	// CPVB2000ZA260000 -> CPV-B2000 ZA26

	char temp[32];
	sprintf(temp, "%s", in);
	memset(out, 0, 32);
	
	int i = 0, j = 0;
	
	// PCH- / PTEL- / CPV-B
	out[i++] = temp[j++];
	out[i++] = temp[j++];
	out[i++] = temp[j++];
	if( temp[i] != '0' ) {
		if( temp[i] == 'L' ) { // P_EL-..
			out[i++] = temp[j++];
			out[i++] = '-'; // 
		} else { // CPV-B.. or other
			out[i++] = '-';
			out[i++] = temp[j++];	
		}
	} else {
		out[i++] = '-'; // 
		j++;
	}
	
	// 1004
	out[i++] = temp[j++];
	out[i++] = temp[j++];
	out[i++] = temp[j++];
	out[i++] = temp[j++];
	out[i++] = ' '; //
	
	// ZA04
	if( temp[3] != 'L' ) {
		out[i++] = temp[j++];
		out[i++] = temp[j++];
		out[i++] = temp[j++];
		out[i++] = temp[j++];
	}
	
	// version ? 
	// todo..

	out[i++] = '\0';
}

//https://github.com/TheOfficialFloW/VitaShell/blob/a7a9d5d9d23a2bc251edb60ea9d8a5c901ef65f2/utils.c#L321
void getSizeString(char string[16], uint64_t size) { 
  double double_size = (double)size;

  int i = 0;
  static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
  while( double_size >= 1024.0 ) {
    double_size /= 1024.0;
    i++;
  }
  snprintf(string, 16, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

