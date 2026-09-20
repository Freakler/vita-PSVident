#include <psp2/kernel/processmgr.h>
#include <psp2/vshbridge.h> 
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/sysmem.h> 
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h> 
#include <psp2/kernel/modulemgr.h> 
#include <psp2/rtc.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/apputil.h>
#include <psp2/appmgr.h>
#include <psp2/system_param.h>
#include <psp2/motion_dev.h> 
#include <psp2/kernel/openpsid.h> 
#include <psp2kern/bt.h> 
#include <psp2/motion_dev.h> 
#include <psp2/io/devctl.h> 
#include <psp2/compat.h>
#include <psp2/udcd.h> 
#include <psp2/perf.h> 

#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

#include "main.h"
#include "psvident.h"
#include "utils/aes.h"
#include "utils/utils.h"
#include "utils/registry.h"
#include "plugin/common.h"

extern int kernmode;


SceBool GetDipsw(SceUInt32 no) {
  int ret = -1;
  SceKblParam kblparam;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  uint8_t *base = (uint8_t *)&kblparam.dipsw;
  uint32_t word = *(uint32_t *)(base + ((no >> 5) * 4));
  return (word >> (no & 31)) & 1;
}

SceBool GetQAFlag(SceUInt32 no, SceUInt32 mask) {
  int ret = -1;
  SceKblParam kblparam;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  /* no = bit 0-128
  SceUInt32 byteIndex = no >> 3;
  SceUInt32 bitIndex  = no & 7;
  SceUInt8 b = kblparam.qa_flags[byteIndex];
  return (b & (mask << bitIndex)) ? SCE_TRUE : SCE_FALSE;
  */

  // no = byte 0-15
  SceUInt8 b = kblparam.qa_flags[no];
  return (b & mask) ? SCE_TRUE : SCE_FALSE;
}


void GetSfoValueByKey(char *sfo, char *key, int *value) { // value
   SFOHeader *header = (SFOHeader *)sfo;
   SFODir *entries = (SFODir *)(sfo+0x14);
   int i;
   for (i = 0; i < header->nitems; i++) {
      if (strcmp(sfo+header->fields_table_offs+entries[i].field_offs, key) == 0) {
         *value = *(int*)(sfo + header->values_table_offs + entries[i].val_offs);
      }
   }
}

void GetSfoStringByKey(char *sfo, char *key, char *value, int size) { // string
   SFOHeader *header = (SFOHeader *)sfo;
   SFODir *entries = (SFODir *)(sfo+0x14);
   int i;
   for (i = 0; i < header->nitems; i++) {
      if (strcmp(sfo+header->fields_table_offs+entries[i].field_offs, key) == 0) {
         memset(value, 0, size);
         strncpy(value, sfo+header->values_table_offs+entries[i].val_offs, size);
      }
   }
}


char *getRegistryStringNow(char *path, char *key) {
  //static unsigned char str[16];
  static unsigned char string[64];
  //int ret = getRegistryBinary(str, "/DEVENV/HOST", "devkitname");
  int ret = getRegistryString(string, path, key);
  if( ret != 0 )
    return error(ret, "ERROR");
  
  //convert2hex(str, string, 8);

  return string;
}

char *getRegistryBinaryNow(char *path, char *key, int len) { 
  static unsigned char str[64];
  static unsigned char string[512];
  int ret = getRegistryBinary(str, path, key);
  if( ret != 0 )
    return error(ret, "ERROR");
  
  convert2hex(str, string, len);

  return string;
}

char *getRegistryIntegerNow(char *path, char *key) {
  int result = -1;
  static unsigned char string[64];
  int ret = getRegistryInteger(&result, path, key);
  if( ret != 0 )
    return error(ret, "ERROR");

  sprintf(string,"%d", result);

  return string;
}

int getRegistryIntegerNow_(char *path, char *key) {
  int result = -1;
  int ret = getRegistryInteger(&result, path, key);
  if( ret != 0 )
    return ret;
  return result;
}

char *getRegistryBooleanNow(char *path, char *key) {
  int result = -1;
  static unsigned char string[64];
  int ret = getRegistryInteger(&result, path, key);
  if( ret != 0 )
    return error(ret, "ERROR");

  sprintf(string,"%s", result ? "TRUE" : "FALSE");

  return string;
}


char *getSmiString1(int part) {
  int ret = -1;
  static char string[128];
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x100, buf);
  if( ret != 0 )
    return error(ret, "ERROR");

  // Example:
  // 0001,0095,0071,0001,0001,3,1,1,1,01.810_20120914_01_E/cex,0.3.0.0_20120926_01_E,1
  //    0,   1,   2,   3, ...
  
  if( part < 0 ) // print full string
    snprintf(string, 128, "%s", buf);
  
  if( part >= 0 ) { // print part only
    //int init_size = strlen(buf);
    int counter = 0;
    char delim[] = ",";
    char *ptr = strtok(buf, delim);
    while( ptr != NULL ){
      if( counter == part ) { // decide which substr to print
        sprintf(string, "%s", ptr);
        break;
      }
      ptr = strtok(NULL, delim);
      counter++;
    }
  }  

  return string;
}

char *getSmiString2(int part) {
  int ret = -1;
  static char string[128];
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x100, buf);
  if( ret != 0 )
    return error(ret, "ERROR");

  // Example:
  // 0001,0001,2,1,1,0122,0034,0043,0050,0109,1,0062,0022,0022,0024,0005
  //    0,   1,2,3,   ...
  
  if( part < 0 ) // print full string
    snprintf(string, 128, "%s", &buf[0x100]);
  
  if( part >= 0 ) { // print part only
    //int init_size = strlen(&buf[0x100]);
    int counter = 0;
    char delim[] = ",";
    char *ptr = strtok(&buf[0x100], delim);
    while( ptr != NULL ){
      if( counter == part ) { // decide which substr to print
        sprintf(string, "%s", ptr);
        break;
      }
      ptr = strtok(NULL, delim);
      counter++;
    }
  }  

  return string;
}

char *getVersionTxtString() {
  unsigned char dat_iv[0x10]      = {0x37,0xFA,0x4E,0xD2,0xB6,0x61,0x8B,0x59,0xB3,0x4F,0x77,0x0F,0xBB,0x92,0x94,0x7B}; // 37FA4ED2B6618B59B34F770FBB92947B
  unsigned char dat_key_100[0x20] = {0x06,0xCC,0x2E,0x8F,0xD4,0x08,0x05,0xA7,0x36,0xF1,0x7C,0xF2,0xC1,0x3D,0x58,0xA6,0xC8,0xCF,0x10,0x7E,0x9E,0x4A,0x66,0xAE,0x25,0xD3,0x9C,0xA2,0x1C,0x25,0x31,0xCC}; // 06CC2E8FD40805A736F17CF2C13D58A6C8CF107E9E4A66AE25D39CA21C2531CC //1.00 - 1.691 
  unsigned char dat_key_180[0x20] = {0x27,0x2A,0xE4,0x37,0x8C,0xB0,0x6B,0xF3,0xF6,0x58,0xF5,0x1C,0x77,0xAC,0xA2,0x76,0x9B,0xE8,0x7F,0xB1,0x9B,0xBF,0x3D,0x4D,0x6B,0x1B,0x0E,0xD2,0x26,0xE3,0x9C,0xC6}; // 272AE4378CB06BF3F658F51C77ACA2769BE87FB19BBF3D4D6B1B0ED226E39CC6 //1.80+
  
  static unsigned char string[2048];
  memset(&string, 0, sizeof(string));
  
  int i;
  size_t length = -1;
  unsigned char *data = NULL;
  
  /// check firmware version
  unsigned int firmware;
  SceKernelFwInfo fwinfo;
  fwinfo.size = sizeof(SceKernelFwInfo);
  _vshSblGetSystemSwVersion(&fwinfo);
  firmware = (unsigned int)fwinfo.version;
  
  SceUID fp = sceIoOpen("vs0:vsh/etc/index.dat", SCE_O_RDONLY, 0);
  if( fp < 0 ) {
    return error(fp, "ERROR");
    
  } else {

    /// get and check filesize
    length = sceIoLseek(fp, 0, SCE_SEEK_END);
    if(length < 0x20) 
      return error(-1, "ERROR");
    
    /// read in data
    sceIoLseek(fp, 0, SCE_SEEK_SET);
    data = malloc(length);
    if( sceIoRead(fp, data, length) != length )
      return error(-1, "ERROR");
    
    sceIoClose(fp);
    
    /// decrypt depending on firmware
    if( firmware < 0x01800000 ) 
      aes256cbc(dat_key_100, dat_iv, data, length, data);
    else
      aes256cbc(dat_key_180, dat_iv, data, length, data);
    
    sprintf(string, "%c", '\0');
    for(i = 0x20; i < length; i++) {
      int len = strlen(string);
      string[len] = data[i];
      string[len+1] = '\0';
    }
    
    // remove trailing newlines -.-
    for( i = strlen(string); string[i-1] == '\n' || string[i-1] == '\0'; i-- )
      string[i-1] = '\0';
  }
  
  return string;
}

char *getVersionTxtStringValueForKey(const char *key) { // in "build" out eg "CEX"
  const char *text = getVersionTxtString();
  if (!text || !key)
    return error(-1, "ERROR"); // todo getVersionTxtString returns text error

  size_t keylen = strlen(key);
  const char *p = text;
  static char value[128];

  while (*p) {
    const char *line_start = p;
    const char *line_end = strchr(p, '\n');
    if (!line_end)
      line_end = p + strlen(p);

    if (strncmp(line_start, key, keylen) == 0 && line_start[keylen] == ':') {

      const char *value_start = line_start + keylen + 1;

      // führende Leerzeichen, Tabs, CR entfernen
      while (*value_start == ' ' || *value_start == '\t' || *value_start == '\r')
        value_start++;

      // Value-Länge bestimmen
      size_t len = line_end - value_start;
      if (len >= sizeof(value))
        len = sizeof(value) - 1;

      memcpy(value, value_start, len);
      value[len] = '\0';

      // trailing CR entfernen
      size_t last = strlen(value);
      if (last > 0 && (value[last-1] == '\r' || value[last-1] == ' '))
        value[last-1] = '\0';

      return value;
    }
    p = (*line_end == '\n') ? line_end + 1 : line_end;
  }
  return error(-2, "ERROR");
}

char *getIddatString() {
  static unsigned char string[2048];
  memset(&string, 0, sizeof(string));
  
  int i;
  size_t length = -1;
  unsigned char *data = NULL;

  SceUID fp = sceIoOpen("ux0:id.dat", SCE_O_RDONLY, 0);
    if( fp < 0 ) {
        return error(fp, "ERROR");
    
    } else {

    /// get and check filesize
    length = sceIoLseek(fp, 0, SCE_SEEK_END);
    if(length < 0x20) 
      return error(-1, "ERROR");
    
    /// read in data
    sceIoLseek(fp, 0, SCE_SEEK_SET);
    data = malloc(length);
    if( sceIoRead(fp, data, length) != length )
      return error(-1, "ERROR");
    
    sceIoClose(fp);

    if( data[0x6] != '=' ) // error check if "xxx=abcd...." format inside
      return error(-1, "DATA ERROR");
    
    sprintf(string, "%c", '\0');
    for(i = 0x3; i < length; i++) { // skip first 3 "ï»¿MID=d3ac...""
      int len = strlen(string);
      string[len] = data[i];
      string[len+1] = '\0';
    }
    
    // remove trailing newlines -.-
    //for( i = strlen(string); string[i-1] == '\n' || string[i-1] == '\0'; i-- )
    //  string[i-1] = '\0';

  }
  
  return string;
}

char *getPspemuVersionTxt() {
  
  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
   *  release:6.61:
   *  build:2,0,3,1,0:emubuild@rd.scei.sony.co.jp
   *  system:58401@release_661,0x06060110:
   *  vsh:p6621@release_661,v58692@release_661,20141113:
   *  target::WorldWide
   * 
   *  release:6.60:
   *  build:1,0,3,1,0:emubuild@rd.scei.sony.co.jp
   *  system:57716@release_660,0x06060010:
   *  vsh:p6616@release_660,v58533@release_660,20110731:
   *  target::WorldWide
   * 
   * 
   *  3.36 -> 6.61
   *  1.0X -> 6.60
   *  0.995 -> 6.37 (May 2011)
   *  0.990 -> 6.36 (Nov 2010)
   */

  static char *sixsixone = "release:6.61:\n" \
                           "build:2,0,3,1,0:emubuild@rd.scei.sony.co.jp\n" \
                           "system:58401@release_661,0x06060110:\n" \
                           "vsh:p6621@release_661,v58692@release_661,20141113:\n" \
                           "target::WorldWide";
  
  static char *sixsixzero = "release:6.60\n" \
                            "build:1,0,3,1,0:emubuild@rd.scei.sony.co.jp\n" \
                            "system:57716@release_660,0x06060010:\n" \
                            "vsh:p6616@release_660,v58533@release_660,20110731:\n" \
                            "target::WorldWide";

 static char *empty = "release:\n" \
                      "build:\n" \
                      "system:\n" \
                      "vsh:\n" \
                      "target:";

  SceUID fp = sceIoOpen("vs0:app/NPXS10028/pcff.skprx", SCE_O_RDONLY, 0);
  if( fp < 0 )
    return error(fp, "ERROR");

  int length = sceIoLseek(fp, 0, SCE_SEEK_END);
  sceIoClose(fp);

/* pcff_entry pcff_table[] = { // CEX
    { "PSP2UPDAT-1_04", 0x0086A7DE, 0x0104 },
    { "PSP2UPDAT-1_05", 0x0086A7DE, 0x0105 },
    { "PSP2UPDAT-1_06", 0x0086A7DE, 0x0106 },
    { "PSP2UPDAT-1_50", 0x007AE32E, 0x0150 },
    { "PSP2UPDAT-1_51", 0x007AE32E, 0x0151 },
    { "PSP2UPDAT-1_52", 0x007AE32E, 0x0152 },
    { "PSP2UPDAT-1_60", 0x007E17CE, 0x0160 },
    { "PSP2UPDAT-1_61", 0x007E181E, 0x0161 },
    { "PSP2UPDAT-1_65", 0x007E64EE, 0x0165 },
    { "PSP2UPDAT-1_66", 0x007E64EE, 0x0166 },
    { "PSP2UPDAT-1_67", 0x007E64EE, 0x0167 },
    { "PSP2UPDAT-1_69", 0x007E64EE, 0x0169 },
    { "PSP2UPDAT-1_691", 0x007E64EE, 0x0169 },
    { "PSP2UPDAT-1_80", 0x007EAAEE, 0x0170 },
    { "PSP2UPDAT-1_81", 0x007EAAEE, 0x0181 },
    { "PSP2UPDAT-2_00", 0x007ED0DE, 0x0200 },
    { "PSP2UPDAT-2_01", 0x007ED0DE, 0x0201 },
    { "PSP2UPDAT-2_02", 0x007ED0DE, 0x0202 },
    { "PSP2UPDAT-2_05", 0x007ED6DE, 0x0205 },
    { "PSP2UPDAT-2_06", 0x007ED6DE, 0x0206 },
    { "PSP2UPDAT-2_10", 0x007D22D4, 0x0210 },
    { "PSP2UPDAT-2_11", 0x007D22C4, 0x0211 },
    { "PSP2UPDAT-2_12", 0x007D22F4, 0x0212 },
    { "PSP2UPDAT-2_60", 0x007D3384, 0x0260 },
    { "PSP2UPDAT-2_61", 0x007D3624, 0x0261 },
    { "PSP2UPDAT-3_00", 0x007D37C4, 0x0300 },
    { "PSP2UPDAT-3_01", 0x007D3FC4, 0x0301 },
    { "PSP2UPDAT-3_10", 0x0077795D, 0x0310 },
    { "PSP2UPDAT-3_12", 0x0077795D, 0x0312 },
    { "PSP2UPDAT-3_15", 0x007779ED, 0x0315 },
    { "PSP2UPDAT-3_18", 0x0077883D, 0x0318 },
    { "PSP2UPDAT-3_30", 0x0076F36D, 0x0330 },
    { "PSP2UPDAT-3_35", 0x0076F38D, 0x0335 },
    { "PSP2UPDAT-3_36", 0x0077345D, 0x0336 },
    { "PSP2UPDAT-3_50", 0x00773A8D, 0x0350 },
    { "PSP2UPDAT-3_51", 0x0077413D, 0x0351 },
    { "PSP2UPDAT-3_52", 0x007741ED, 0x0352 },
    { "PSP2UPDAT-3_55", 0x0077429D, 0x0355 },
    { "PSP2UPDAT-3_57", 0x0077449D, 0x0357 },
    { "PSP2UPDAT-3_60", 0x007748FD, 0x0360 },
    { "PSP2UPDAT-3_61", 0x007748FD, 0x0361 },
    { "PSP2UPDAT-3_63", 0x007748FD, 0x0363 },
    { "PSP2UPDAT-3_65", 0x007749DD, 0x0365 },
    { "PSP2UPDAT-3_67", 0x007749DD, 0x0367 },
    { "PSP2UPDAT-3_68", 0x007749DD, 0x0368 },
    { "PSP2UPDAT-3_69", 0x007749DD, 0x0369 },
    { "PSP2UPDAT-3_70", 0x007749DD, 0x0370 },
    { "PSP2UPDAT-3_71", 0x007749DD, 0x0371 },
    { "PSP2UPDAT-3_72", 0x007749ED, 0x0372 },
    { "PSP2UPDAT-3_73", 0x007749ED, 0x0373 },
    { "PSP2UPDAT-3_74", 0x007749ED, 0x0374 },
}; */


  switch(length) { // hash would differ for builds CEX/DEX/TOOL but size is the same!
    case 0x007749ED: // 3.72 - 3.74
    case 0x007749DD: // 3.65 - 3.71
    case 0x007748FD: // 3.60 - 3.63
    case 0x0077449D: // 3.57
    case 0x0077429D: // 3.55
    case 0x007741ED: // 3.52
    case 0x0077413D: // 3.51
    case 0x00773A8D: // 3.50
    case 0x0077345D: // 3.36
      return sixsixone;
      
    case 0x0076F38D: // 3.35
    case 0x0076F36D: // 3.30
    case 0x00777EED: // 3.20
    case 0x0077883D: // 3.18
    case 0x007779ED: // 3.15
    case 0x0077795D: // 3.10 - 3.12
    case 0x0077729D: // 3.100.040
    case 0x007D3FC4: // 3.01
    case 0x007D37C4: // 3.00
    case 0x007D3624: // 2.61
    case 0x007D3384: // 2.60
    case 0x007D22F4: // 2.12
    case 0x007D22C4: // 2.11
    case 0x007D22D4: // 2.10
    case 0x007ED6DE: // 2.05 - 2.06
    case 0x007ED0DE: // 2.00 - 2.02
    case 0x007EAB0E: // 1.810.021
    case 0x007EAA8E: // 1.800.060
    case 0x007EAAEE: // 1.70 - 1.81
    case 0x007E64EE: // 1.65 - 1.692
    case 0x007E181E: // 1.61
    case 0x007E17CE: // 1.60
    case 0x007AE32E: // 1.50 - 1.52
    case 0x0086A7DE: // 1.03 - 1.06
      return sixsixzero;

    default: return empty;
  }
  
  return "error";
}


int getCurrentFirmware(int mode, uint8_t *retstr, uint32_t *rawval) {  // 0 via sceKernelGetSystemSwVersion(), 1 via active os0:, 2 via Index.Dat
  if( mode == 0 ) { /// from ModuleMgr (which might be spoofed, so last resort - but works in usermode)
    SceKernelSystemSwVersion sw_ver_param;
    sw_ver_param.size = sizeof(SceKernelSystemSwVersion);
    int ret = sceKernelGetSystemSwVersion(&sw_ver_param);
    if( ret >= 0 ) {
      *rawval = sw_ver_param.version; // 0x0XYZ0000
      sprintf(retstr, "%s", sw_ver_param.versionString); // X.YZ
    }
    return ret;
  }

  if( mode == 1 ) { // from active os0 
    SceUID fd = sceIoOpen("sdstor0:int-lp-act-os", SCE_O_RDONLY, 0777);
    if( fd >= 0 ) {
      char buf[0x100];
      unsigned int firmware = -1;
      sceIoLseek(fd, 0xB000, SCE_SEEK_SET); //0xB000 is first file (always?)
      sceIoRead(fd, buf, sizeof(buf));
      sceIoClose(fd);

      firmware = *(unsigned int *)(buf + 0x92); //at 0x92 in file is firmware
      *rawval = firmware;
      firmware_string(retstr, firmware);

    } return fd;
  }

  if( mode == 2 ) { // from "vs0:vsh/etc/index.dat"
    char fw[32];
    memset(fw, 0, 32);
    sprintf(fw, getVersionTxtStringValueForKey("release"));

    if( strlen(fw) != 10 ) // eg "0x03650011"
      return -1;

    *rawval = strtoul(fw, NULL, 0);
    sprintf(retstr, "%c.%c%c%c.%c%c%c", fw[3], fw[4], fw[5], fw[6], fw[7], fw[8], fw[9]);

    return 0;
  }

  return -1;
}

int getSerial(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  char temp[0x40];
  char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x112, buf);
  if( ret != 0 ) { // either error reading leaf or its prototype that has no modelstring
    return ret;
  }      
  // Examples
  // 0000000000003TG135F0H60070050420 // CPV-2000KD1  03-TG.. 6007005
  // 00000000000000000274599200100771 // PDEL-1000  00-27.. 0100771
  // 00000000000000000TG9B000K8200250 // DEM-3000L  00-TG.. 8200250 
  // 00000000000032745212505300760C61 // PTEL-2002   03-27.. 0530076
  // 00000000000000003TG16201H8120014 // CEM-3000NP1  03-TG.. 8120014 

  // we want it look like this: "03-27447091-2068012"
  
  memset(temp, 0, sizeof(temp));
  
  int i = 1, j;  
  while( buf[i] == 0x30 )
    i+=2;

  for( j = 0; i < 0x40; i+=2, j++ ) {
    temp[j] = buf[i];
  } memset(buf, 0, sizeof(buf));
  
  // 32744709120680120BD0
  
  snprintf(rawstr, 0x20, "%s", temp);
  
  if( strlen(temp) > 16 ) { // there are unwanted trailing numbers (ALWAYS 4!?)
    temp[strlen(temp)-4] = 0;
  } 
  
  // 3274470912068012
  
  for( i = 19, j = strlen(temp); i >= 0; i-- ) {
    if( i == 11 ) retstr[i] = '-'; // between serial and code
    else if( i == 2 ) retstr[i] = '-'; // "03-27.."
    else {
      if( j >= 0 ) { 
        retstr[i] = temp[j--];
      } else retstr[i] = '0'; // fill the rest in front
    }
  }
  
  // 03-27447091-2068012
  
  return 0;
}

int getRefurbished(uint8_t *retstr, uint32_t *rawval) {
  uint8_t str[256];
  uint8_t strraw[256];

  int ret = getSerial(str, strraw);
  if( ret >= 0 ) {
    if( strraw[0] == '8' ) {
      *rawval = TRUE;
      sprintf(retstr, "TRUE");
    } else {
      *rawval = FALSE;
      sprintf(retstr, "FALSE");
    }
  }
  
  return ret;
}

int getPreviousFirmware(uint8_t *retstr, uint32_t *rawval) { // from inactive os0 
  char buf[0x100];
  unsigned int previous_fw = -1;
  
  SceUID fd = sceIoOpen("sdstor0:int-lp-ina-os", SCE_O_RDONLY, 0777);
  if( fd < 0 ) // error
    return fd;
  
  sceIoLseek(fd, 0xB000, SCE_SEEK_SET); //0xB000 is first file (always?)
  sceIoRead(fd, buf, sizeof(buf));
  sceIoClose(fd);
  
  previous_fw = *(unsigned int *)(buf + 0x92); //at 0x92 in file is firmware

  *rawval = previous_fw; 
  firmware_string(retstr, previous_fw);
  return 0;
}

int getModelCode(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x102, buf);
  if( ret != 0 )
    return ret;
  
  snprintf(retstr, 9, "%s", &buf[0x68]); // eg: "TG135F0H" for proto | "27447180" for slim (03-27447180-SERIALNO)
  snprintf(rawstr, 0x20, "%s", &buf[0x68]); // eg: "27447180F0C604085868000000000000" (slim)
  
  return ret;
}

int getModelName(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static char string[64];
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x115, buf); // protos are missing this leaf!
  if( ret >= 0 ) {
    snprintf(rawstr, 0x10+1, "%s", buf); // write only x chars to string

    model_string(string, rawstr);
    sprintf(retstr, string);

    return ret;
  }
  
  ret = getModelCode(retstr, rawstr); // for protos  eg: "TG16201H"
  
  return ret;
}

int getModelColor(uint8_t *retstr, uint32_t *rawval) { // retstr the official name, rawval the apporx RGB (0xABGR) value
  int ret = -1;
  static char string[64];
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x115, buf); // eg: "PCH01104ZA010003"   // protos are missing this leaf!
  if( ret >= 0 ) {
    sprintf(string, "%c%c%c%c", buf[8], buf[9], buf[10], buf[11] ); // eg: "ZA01"

    // slightly altered because default text and bg
    int CUS_BLACK = 0xFF666666; // 0xFF000000
    int CUS_WHITE = 0xFFCCCCCC; // 0xFFFFFFFF

    /// Testkit/Devit
    if( strcmp("0001", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return 999; } // fat         return 999 to not print " (Black)" in overview..
    if( strcmp("0011", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return 999; } // slim                 ..they are always black anyways.


    /// PSTVs
    if( buf[0] == 'V') { // WHITE PSTV wtf? AA01/AB01 was black!! at this point they didn't care at all anymore..
      if( strcmp("AA01", string) == 0 ) { sprintf(retstr, "White"); *rawval = CUS_WHITE; return ret; } 
      if( strcmp("AB01", string) == 0 ) { sprintf(retstr, "White"); *rawval = CUS_WHITE; return ret; } 
      if( strcmp("AB12", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return ret; }
      if( strcmp("AB22", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return ret; }
    }

    /// Fats
    if( strcmp("ZA01", string) == 0 ) { sprintf(retstr, "Crystal Black"); *rawval = CUS_BLACK; return ret; } 
    if( strcmp("AA01", string) == 0 ) { sprintf(retstr, "Crystal Black"); *rawval = CUS_BLACK; return ret; } 
    if( strcmp("AB01", string) == 0 ) { sprintf(retstr, "Crystal Black"); *rawval = CUS_BLACK; return ret; } 
    if( strcmp("ZA02", string) == 0 ) { sprintf(retstr, "Crystal White"); *rawval = CUS_WHITE; return ret; } 
    if( strcmp("ZAZ1", string) == 0 ) { sprintf(retstr, "Crystal White"); *rawval = CUS_WHITE; return ret; } 
    if( strcmp("ABZ1", string) == 0 ) { sprintf(retstr, "Crystal White"); *rawval = CUS_WHITE; return ret; } 
    if( strcmp("ZX02", string) == 0 ) { sprintf(retstr, "Crystal White"); *rawval = CUS_WHITE; return ret; } 
    if( strcmp("ZAZ2", string) == 0 ) { sprintf(retstr, "Crystal Black"); *rawval = CUS_BLACK; return ret; } 
    if( strcmp("ZA04", string) == 0 ) { sprintf(retstr, "Sapphire Blue"); *rawval = 0xFFFF4400; return ret; } 
    if( strcmp("ZA03", string) == 0 ) { sprintf(retstr, "Cosmic Red");    *rawval = 0xFF0000FF; return ret; } 
    if( strcmp("AB03", string) == 0 ) { sprintf(retstr, "Cosmic Red");    *rawval = 0xFF0000FF; return ret; } 
    if( strcmp("AB04", string) == 0 ) { sprintf(retstr, "Sapphire Blue"); *rawval = 0xFFFF4400; return ret; } 
    if( strcmp("ZAZ3", string) == 0 ) { sprintf(retstr, "Cosmic Red");    *rawval = 0xFF0000FF; return ret; } 
    if( strcmp("ZX05", string) == 0 ) { sprintf(retstr, "Ice Silver");    *rawval = 0xFFC0C0C0; return ret; } 
    if( strcmp("ZAZ4", string) == 0 ) { sprintf(retstr, "Crystal Black"); *rawval = BLACK; return ret; } 


    /// Slims
    if( strcmp("ZX11", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return ret; } 

    if( strcmp("ZA11", string) == 0 ) { sprintf(retstr, "Black"); *rawval = CUS_BLACK; return ret; } 
    if( strcmp("ZA12", string) == 0 ) { sprintf(retstr, "White"); *rawval = CUS_WHITE; return ret; } 
    if( strcmp("ZA13", string) == 0 ) { sprintf(retstr, "Lime Green / White"); *rawval = 0xFF46EAC7; return ret; } 
    if( strcmp("ZA14", string) == 0 ) { sprintf(retstr, "Light Blue / White"); *rawval = 0xFFE6D8AD; return ret; } 
    if( strcmp("ZA15", string) == 0 ) { sprintf(retstr, "Pink / Black"); *rawval = 0xFFC000C0; return ret; } 
    if( strcmp("ZA16", string) == 0 ) { sprintf(retstr, "Khaki / Black"); *rawval = 0xFF4C7585; return ret; } 

    if( strcmp("ZAZ5", string) == 0 ) { sprintf(retstr, "Khaki / Black"); *rawval = 0xFF4C7585; return ret; } // PCHJ-10010
    if( strcmp("ZAZ6", string) == 0 ) { sprintf(retstr, "White"); *rawval = CUS_WHITE; return ret; } // PCHJ-10009
    if( strcmp("ZAZ7", string) == 0 ) { sprintf(retstr, "White"); *rawval = CUS_WHITE; return ret; } // PCHL-60001
    if( strcmp("ZAZ9", string) == 0 ) { sprintf(retstr, "Lime Green / White"); *rawval = 0xFF46EAC7; return ret; } // PCHJ-10027

    if( strcmp("ZX17", string) == 0 ) { sprintf(retstr, "Red / Black"); *rawval = 0xFF0000FF; return ret; }
    if( strcmp("ZX18", string) == 0 ) { sprintf(retstr, "Blue / Black"); *rawval = 0xFFE16941; return ret; }

    if( strcmp("ZA19", string) == 0 ) { sprintf(retstr, "Light Pink / White"); *rawval = 0xFFC1B6FF; return ret; }
    if( strcmp("ZA22", string) == 0 ) { sprintf(retstr, "Glacier White"); *rawval = 0xFFFFFFFF; return ret; }
    if( strcmp("ZA23", string) == 0 ) { sprintf(retstr, "Aqua Blue"); *rawval = 0xFFFFC123; return ret; }
    if( strcmp("ZA24", string) == 0 ) { sprintf(retstr, "Neon Orange"); *rawval = 0xFF1F5FFF; return ret; }
    if( strcmp("ZA25", string) == 0 ) { sprintf(retstr, "Silver"); *rawval = 0xFFC0C0C0; return ret; }
    if( strcmp("ZA26", string) == 0 ) { sprintf(retstr, "Metallic Red"); *rawval = 0xFF3442E3; return ret; }

    if( strcmp("ZX23", string) == 0 ) { sprintf(retstr, "Aqua Blue"); *rawval = 0xFFFFC123; return ret; }

  }

  
  // for protos (when leaf fails) eg: Fat black "TG16201H" / Slim metallic red "TG166M0H"
  /* ret = getModelCode(string, rawstr);
  if( ret >= 0 ) {

    if( string[2] == '9' ) { // DEM

    }

    if( string[2] == 'D' ) { // Consumer with DEX firmware

    }

    if( string[2] == '1' ) { // CEX + 
      /// 03-TG16510H 	= CPV-2000 WPx 
      /// 03-TG166M0H 	= CPV-B2000
      /// 03-TG16100H 	= CEM-3000VP1

      // -> not way to decide.. without other things like hwinfo
      
    }

    switch(string[5]) {
      // FAT ( 1 digit)
      case '0': sprintf(retstr, "Black"); 
                *rawval = 0xFF000000; 
                break;
      case '1': sprintf(retstr, "White"); 
                *rawval = 0xFFFFFFFF; 
                break;
      case 'A': sprintf(retstr, "Cosmic Red"); 
                *rawval = 0xFF0000FF; 
                break;
      case 'B': sprintf(retstr, "Sapphire Blue"); 
                *rawval = 0xFFFF0000; 
                break;

      // SLIM (2 digits) 
      //50 	Black
      //51 	White
      //53 	Pink
      //5F 	KHAKI
      //61 	Silver
      //6M 	Metallic Red 

    }
  } -> Protos always default color */


  //sprintf(retstr, "unknown"); 
  //*rawval = 0xFF808080; // grey

  return ret;
}

int getRegionForDev(uint8_t *retstr, uint32_t *rawval) { // from Model String #ugly 
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x115, buf); // protos are missing this leaf!
  if( ret != 0 ) { // error
    //return ret;

    ret = vshIdStorageReadLeaf(0x102, buf);
    if( ret != 0 ) // error
      return ret;

    /// via Serial digit -> 1 US, 2 Asia, 4 EU etc
    if( buf[0x68 + 0] == 'T' && buf[0x68 + 1] == 'G' ) {
      switch( buf[0x68 + 6] ) {
        case '0': sprintf(retstr, "JP"); break;
        case '1': sprintf(retstr, "US"); break;
        case '2': sprintf(retstr, "ASIA"); break;
        case '4': sprintf(retstr, "EU"); break;
        default: sprintf(retstr, " "); // unknown
      }
      return 0;
    }

    return -2468; // no "TG" code
  }

  char model = buf[1]; // D or T
  char version = buf[4]; // 1 or 2
  char code = buf[7]; // 0, 1, 2 ..
  
  if( model != 'D' && model != 'T' )
    return -2468;
  
  if( version != '1' && version != '2' )
    return -2468;
  
  if( model == 'D' ) { // Devkit
    switch( code ) {
      case '0': sprintf(retstr, "J1 (Japan)"); break;
      case '1': sprintf(retstr, "(EU & US)"); break;
      case '2': sprintf(retstr, "(Asia)"); break;
      default: sprintf(retstr, "error");
    }
  }
  
  if( model == 'T' ) { // Testkit
    if( version == '1' ){ // Fat
      switch( code ) { 
        case '0': sprintf(retstr, "J1 (Japan)"); break; // HJ..
        case '1': sprintf(retstr, "UC2 (North America)"); break; // HU..
        case '2': sprintf(retstr, "Korea"); break; // HE..
        case '3': sprintf(retstr, "CEL (Europe)"); break; // HC..
        default: sprintf(retstr, "error");
      }
    }
    if( version == '2' ){ // Slim
      switch( code ) {
        case '0': sprintf(retstr, "J1 (Japan)"); break;
        case '2': sprintf(retstr, "Korea"); break;
        case '4': sprintf(retstr, "US, CA, EU, TR, UK"); break;
        default: sprintf(retstr, "error");
      }
    }
  }
  
  return ret;
}

int getRegion(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];

  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret != 0 ) 
    return ret;
  
  *rawval = (CID[4] << 8) | CID[5];
  
  switch( *rawval ) {
    case 0x100:
      sprintf(retstr, "-");
      break;
      
    case 0x101: case 0x102: // Devkit / Testkit
      ret = getRegionForDev(retstr, rawval);
      break;
    
    case 0x103: sprintf(retstr, "J1 (Japan)"); break; // 0
    case 0x104: sprintf(retstr, "UC2 (North America)"); break;  // 1   // UCS later for slim?!
    case 0x105: sprintf(retstr, "CEL (Europe)"); break; // 4
    case 0x106: sprintf(retstr, "KR2 (South Korea)"); break; // 5
    case 0x107: sprintf(retstr, "CEK (United Kingdom)"); break; // 3
    case 0x108: sprintf(retstr, "MX2 (Mexico)"); break;
    case 0x109: sprintf(retstr, "AU3 (Australia/New Zealand)"); break;
    case 0x10A: sprintf(retstr, "E12 (Asia)"); break; // 6
    case 0x10B: sprintf(retstr, "TW1 (Taiwan)"); break; //7
    case 0x10C: sprintf(retstr, "RU3 (Russia)"); break; //8
    case 0x10D: sprintf(retstr, "CN9 (China)"); break;
    case 0x10E: sprintf(retstr, "HK5 (Hong Kong)"); break;
    case 0x10F: sprintf(retstr, "RSV1 (reserved)"); break;
    case 0x110: sprintf(retstr, "RSV2 (reserved)"); break;
    case 0x111: sprintf(retstr, "RSV3 (reserved)"); break;
    default: sprintf(retstr, "Unknown?!");
  }  
  
  return ret;
}

int getTarget(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage, 2 via vshSblAimgrIs
  int ret = -1;
  uint8_t CID[0x200];

  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else if( mode == 2 ) { // via 
    if( vshSblAimgrIsCEX() )       sprintf(retstr, "CEX (Consumer)"); // "Consumer Experience"
    else if( vshSblAimgrIsDEX() )  sprintf(retstr, "DEX (Testing)");  // "Developer Experience"
    else if( vshSblAimgrIsTool() ) sprintf(retstr, "TOOL (Development)");
    else if( vshSblAimgrIsTest() ) sprintf(retstr, "TEST (Internal)");
    else sprintf(retstr, "error");
  
    *rawval = 0;
    return 0;

  } else return -1;

  if( ret >= 0 ) { // success
  
    *rawval = (CID[4] << 8) | CID[5];
    
    switch( *rawval ) {
      case 0x100:
        sprintf(retstr, "TEST (Internal)");
        break;
        
      case 0x101: // Devkit
        sprintf(retstr, "TOOL (Development)");
        break;

      case 0x102: // Testkit
        sprintf(retstr, "DEX (Testing)");
        break;
      
      case 0x103:
      case 0x104:
      case 0x105:
      case 0x106:
      case 0x107:
      case 0x108: 
      case 0x109:
      case 0x10A:
      case 0x10B:
      case 0x10C:
      case 0x10D:
      case 0x10E:
      case 0x10F:
      case 0x110:
      case 0x111: 
        sprintf(retstr, "CEX (Consumer)");
        break;

      default: sprintf(retstr, "error");
    }
  }
  return ret;
}

int getMotherboard(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  unsigned char hwinfo[4];
  
  ret = _vshSysconGetHardwareInfo(hwinfo);
  if( ret != 0 )
    return ret;
  
  *rawval = *(uint32_t*)&hwinfo[0];

  switch( hwinfo[2] ) {
    case 0x10: sprintf(retstr, "IRS-001"); break; // early CEMs
    case 0x31: sprintf(retstr, "IRT-001"); break; // DEM G/H
    case 0x40: sprintf(retstr, "IRS-002"); break; // PCH-1XXX
    case 0x41: sprintf(retstr, "IRT-002"); break; // DEM & PDEL
    case 0x60: sprintf(retstr, "IRS-1001"); break; // PCH-1XXX (new)
    case 0x70: sprintf(retstr, "DOL-1001"); break; // VTE-10XX 
    case 0x72: sprintf(retstr, "DOL-1002"); break; // VTE-10XX (new)
    case 0x80: sprintf(retstr, "USS-1001"); break; // PCH-20XX 
    case 0x82: sprintf(retstr, "USS-1002"); break; // PCH-20XX (new)
    default: sprintf(retstr, "unknown"); break;
  }
  
  if( (hwinfo[2] &= ~0xF0) != 0x01 ) { // additional capabilities (only non-devkits)
    //if( (hwinfo[0] &= ~0xF0) == 0x0 ) // check wifi
      //strcat(string, " (Wifi)");
    
    if( (hwinfo[0] &= ~0xF0) == 0x2 ) // check for 0xX2 where the 2 is for 3g
      strcat(retstr, " (3g)");
  }
  
  return ret;
}

int getCpboard(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  SceKblParam kblparam;
  //static unsigned char string[32];

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -42;
  
  uint16_t version = kblparam.dipsw.cp_version;
  uint16_t id = kblparam.dipsw.cp_build_id;
  
  
  // Neighborhood outputs like this:
  // cp info.   : bid.4 ver.1301
  // (BoardID + Version)
  sprintf(rawstr, "bid.%X ver.%x", id, version);
  //*rawval =  
  

  /************************
  bid.3 ver.0851 (DEM-G00)
  bid.3 ver.0910 (DEM-G19)
  bid.3 ver.0920 (DEM-H84)
  0-835-185-02
  
  ?
  0-851-147-03 (DEM-L)
  
  ?
  0-851-147-07 (DEM-P)
  
  bid.4 ver.1301
  1-884-532-10 (DEM-R)
  1-884-532-11 (PDEL)
  ************************/
  
  switch( id ) {
    case 3: sprintf(retstr, "GCP-001"); break; // DEM G/H
    case 4: sprintf(retstr, "GCP-002"); break; // new DEM & PDEL
    default: sprintf(retstr, "unknown"); break;
  }
  
  return ret;
}

int getMinFirmware(uint8_t *retstr, uint32_t *rawval) { // via SblAimgr (backup via idstorage directly)
  int ret = -1;
  unsigned int version = -1;
  

  ret = _vshSblAimgrGetSMI(&version); // SMI = Service/System Manufacturing Information
  if( ret >= 0 ) { // success
    *rawval = version; // 0x0XYZ0000
    firmware_string(retstr, version); // X.YZ
    return ret;
  }
  
  
  /// try manually
  static char buf[0x200];
  ret = vshIdStorageReadLeaf(0x80, buf);
  if( ret >= 0 ) {
    version = *(unsigned int *)(buf + 0x8);
    *rawval = version; // 0x0XYZ0000
    firmware_string(retstr, version); // X.YZ
  }

  return ret;
}

int getFactoryFirmware(uint8_t *retstr, uint8_t *rawstr) { // the first firmware installed in the factory (though SMI string is NOT always correct :/)
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x100, buf);
  if( ret != 0 )
    return ret;
  
  // Examples:
  // 0001,0197,0096,0001,0001,3,1,1,1,03.150_20140430_01_E/cex,0.3.8.0_20131209_01_E,1  
  // 0001,0246,0108,0001,0001,3,1,1,1,03.60_20160404_01_E/cex,0.4.4.1_20150330_01_E,1
  
  if( buf[0x23] != '.' ) // until now all SMI Strings match this
    return -43;
  
  sprintf(retstr, "%c.%c%c", buf[0x22], buf[0x24], buf[0x25]);
  sprintf(rawstr, "%s", getSmiString1(9));
  
  return ret;
}

int getConsoleID(int mode, uint8_t *retstr, uint8_t *rawstr) { // 0 via SblAimgr (current), 1 via IDStorage (original)
  int i, ret = -1;
  uint8_t CID[0x200];
  static unsigned char string[64];
  static char helper[8];
  memset(string, 0, 64);
  
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  
  if( ret >= 0 ) { // success
    for( i = 0; i < 16; i++ ) {
      sprintf(helper, "%02X", CID[i]);
      strcat(string, helper);
    } sprintf(rawstr, string);

    memset(string, 0, 64);

    for( i = 0; i < 16; i++ ) {
      sprintf(helper, "%02X", CID[i]);
      
      if( i % 2 ) 
        strcat(helper, " ");
      
      strcat(string, helper);
    } sprintf(retstr, string);
  }

  // sprintf(retstr, "0000 0001 0103 0014 0C15 A3EE 341F 0DE0"); // dummy

  return ret;
}

int getConsoleIdCompanycode(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret >= 0 ) {
    *rawval = (CID[2] << 8) | CID[3];
    sprintf(retstr, "%X", *rawval);
  }
  return ret;
}

int getConsoleIdProductcode(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret >= 0 ) {

    *rawval = (CID[4] << 8) | CID[5];

    switch( (CID[4] << 8) | CID[5] ) {
      case 0x100:  sprintf(retstr, "TEST"); break;
      case 0x101:  sprintf(retstr, "TOOL"); break;
      case 0x102:  sprintf(retstr, "DEX"); break;
      case 0x103: sprintf(retstr, "CEX (J1)"); break; 
      case 0x104: sprintf(retstr, "CEX (UC2)"); break; 
      case 0x105: sprintf(retstr, "CEX (CEL)"); break;
      case 0x106: sprintf(retstr, "CEX (KR2)"); break; 
      case 0x107: sprintf(retstr, "CEX (CEK)"); break;
      case 0x108: sprintf(retstr, "CEX (MX2)"); break;
      case 0x109: sprintf(retstr, "CEX (AU3)"); break;
      case 0x10A: sprintf(retstr, "CEX (E12)"); break; 
      case 0x10B: sprintf(retstr, "CEX (TW1)"); break; 
      case 0x10C: sprintf(retstr, "CEX (RU3)"); break; 
      case 0x10D: sprintf(retstr, "CEX (CN9)"); break;
      case 0x10E: sprintf(retstr, "CEX (HK5)"); break;
      case 0x10F: sprintf(retstr, "RSV1"); break;
      case 0x110: sprintf(retstr, "RSV2"); break;
      case 0x111: sprintf(retstr, "RSV3"); break;
      default: sprintf(retstr, "unknown");
    }

    //sprintf(retstr, "%X", *rawval);
  }
  return ret;
}

int getConsoleIdSubcode(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
 if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret >= 0 ) {
    *rawval = (CID[6] << 8) | CID[7];
    
    // to be completed - maybe ven change nameing "Fat" / "Slim" / "PSTV" instead of codenames
    switch( (CID[6] << 8) | CID[7] ) { // http://wiki.corcovado.info/index.php?title=Vita_Codes#ConsoleID
      case 0x1: case 0x2:  case 0x3:
      case 0x4: case 0x5:  case 0x6:
      case 0x7: case 0x8:  case 0x9:
      case 0xA: case 0xB:  case 0xC:
      case 0xD: case 0xE:  case 0xF:
        sprintf(retstr, "Iris Prototype"); 
        break;

      // PDEL / PTEL-1xxx / PCH-1xxx
      case 0x10:  sprintf(retstr, "Iris"); break; 

        // CPV-B1xxxx likely but unconfirmed! 
        case 0x401:  sprintf(retstr, " "); break; 
        case 0x402:  sprintf(retstr, " "); break;
        case 0x403:  sprintf(retstr, " "); break;

      // PCH-1xxx (new Soc)
      case 0x11:  sprintf(retstr, " "); break;
      case 0x12:  sprintf(retstr, "Iris VA"); break; 

        // CPV-2000
        case 0x404:  sprintf(retstr, " "); break;
        case 0x406:  sprintf(retstr, "Ushiwaka Prototype"); break; // CPV-2000KD1
        case 0x407:  sprintf(retstr, " "); break;

      // PCH-2000 / PTEL-2000
      case 0x13:  sprintf(retstr, " "); break;
      case 0x14:  sprintf(retstr, "Ushiwaka"); break; 

        case 0x408:  sprintf(retstr, " "); break; // PSTV "TOOL" only -> no CEX / DEX !!!!

        case 0x601:  sprintf(retstr, " "); break;
        case 0x602:  sprintf(retstr, "Dolce Prototype"); break; // THV-1000 D1 (CEM-3000P01) DOL-1001

      // PSTV DOL-1001
      case 0x201:  sprintf(retstr, "Dolce"); break; 

        // PSTV (new) protos?
        case 0x603:  sprintf(retstr, " "); break; // "TOOL" only -> no CEX / DEX !!!!
        case 0x604:  sprintf(retstr, " "); break;
        case 0x605:  sprintf(retstr, " "); break;
        case 0x606:  sprintf(retstr, " "); break;

      // PSTV (new) DOL-1002
      case 0x202:  sprintf(retstr, "Dolce 1.2g"); break;

        // CPV-B2000
        case 0x409:  sprintf(retstr, " "); break; // "TOOL" only -> no CEX / DEX !!!!
        case 0x40A:  sprintf(retstr, " "); break;
        case 0x40D:  sprintf(retstr, " "); break;
        case 0x40E:  sprintf(retstr, " "); break;

      case 0x17:  sprintf(retstr, " "); break;
      case 0x18:  sprintf(retstr, "Ushiwaka VA"); break; /// CPV-B2xxx

      default: sprintf(retstr, "unknown");
    }

  }
  return ret;
}

int getConsoleIdChassis(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret >= 0 ) {
    *rawval = (CID[8] << 8) | CID[9];
    sprintf(retstr, "%X", *rawval);
  }
  return ret;
}

int getConsoleIdFactoryCode(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;

  if( ret >= 0 ) {
    *rawval = CID[8] >> 2;

    switch(CID[8] >> 2) { // via https://github.com/TeamFAPS/PS-ConsoleId-wiki/blob/master/PS-ConsoleId-wiki.txt#L270
      case  0x1: sprintf(retstr, "Japan"); break; // eg PDEL
      case  0x2: sprintf(retstr, "China 1"); break; 
      case  0x3: sprintf(retstr, "China 2"); break; // seen on most retail devices
      case  0x4: sprintf(retstr, "China 3"); break; 
      case 0x23: sprintf(retstr, "Japan Diag 1"); break;
      case 0x24: sprintf(retstr, "Japan Diag 2"); break; // we found this on Rickys dolphin VP1 for example! (no serial yet) ->  0000 0001 0103 0010 9000 0000 C8XX XXXX   // https://wiki.henkaku.xyz/vita/SceSysmem#sceSblAIMgrIsDiagForDriver
      case 0x3D: sprintf(retstr, "Service Center #2"); break;
      case 0x3E: sprintf(retstr, "Service Center #1"); break;
      default: sprintf(retstr, "unknown");
  
    }

    // sprintf(retstr, "%d", *rawval);
    
  }
  return ret;
}

int getConsoleIdSerial(int mode, uint8_t *retstr, uint32_t *rawval) { // 0 via SblAimgr, 1 via IdStorage
  int ret = -1;
  uint8_t CID[0x200];
    
  if( mode == 0 ) { // via SblAimgr
    ret = _vshSblAimgrGetConsoleId(CID);
  
  } else if( mode == 1 ) { // via IDStorage
    //ret = vshIdStorageReadLeaf(0x00, CID); // PSP leaves
    //ret = vshIdStorageReadLeaf(0x20, CID); // PSP leaves (copy)
    ret = vshIdStorageReadLeaf(0x40, CID); // SceIdStoragePsp2Certificates

  } else return -1;


  if( ret >= 0 ) {
    
    // https://github.com/TeamFAPS/PS-ConsoleId-wiki/blob/master/PS-ConsoleId-wiki.txt#L270

    // uint32_t serial_no_major  = CID[8] >> 6; // confirmed though?
    uint32_t serial_no_middle = CID[9];
    uint32_t serial_no_minor  = (CID[10] << 8) | CID[11];

    uint32_t serial = /*(serial_no_major << 24) |*/ (serial_no_middle << 16) | (serial_no_minor);

    *rawval = serial;
     sprintf(retstr, "%d", *rawval);
  }

  return ret;
}

int getMacAddressWifi(uint8_t *retstr, uint8_t *rawstr) { // via sceNet
  int ret = -1;
  SceNetEtherAddr mac;
    
  ret = sceNetGetMacAddress(&mac, 0);
  if( ret >= 0 ) {
    sprintf(rawstr, "%02X%02X%02X%02X%02X%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
    sprintf(retstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
  }

  // sprintf(retstr, "00:71:CC:B5:FF:00"); // dummy

  return ret;
}

int getMacAddressWifiViaIDStorage(uint8_t *retstr, uint8_t *rawstr) { // via IDStorage
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x111, buf); 
  if( ret >= 0 ) {

    if( buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF ) // no mac here
      return -1234; // "NO DATA"
  
    sprintf(rawstr, "%02X%02X%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    sprintf(retstr, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
  }
  return ret;
}

int getMacAddressWifiOui(uint8_t *retstr, uint8_t *rawstr) { // via sceNet
  int ret = -1;
  SceNetEtherAddr mac;
    
  ret = sceNetGetMacAddress(&mac, 0);
  if( ret >= 0 ) {
  
    sprintf(rawstr, "%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2]);
    
    int oui = mac.data[2] + (mac.data[1] * 0x100) + (mac.data[0] * 0x10000);
    
    switch( oui ) { // https://oui.is
      case 0x002258: 
      case 0xD44B5E: sprintf(retstr, "Taiyo Yuden Co., Ltd."); 
              break; 
      
      case 0x0015C1: sprintf(retstr, "Sony Interactive Entertainment Inc."); 
              break; 

      case 0x4439C4: 
      case 0xFC4DD4: sprintf(retstr, "Universal Global Scientific Industrial., Ltd"); 
              break; 
              
      case 0x0071CC: 
      case 0x2C337A: 
      case 0x346895:
      case 0x60F494: 
      case 0x681401: 
      case 0x707781: 
      case 0xC03896: 
      case 0xD46A6A: 
      case 0xEC0EC4: 
      case 0xF82FA8: sprintf(retstr, "Hon Hai Precision Ind. Co.,Ltd."); // aka Foxcon
              break;

      default: sprintf(retstr, "unknown"); 
    }

  }

  return ret;
}

int getMacAddressLan(uint8_t *retstr, uint8_t *rawstr) { // via 
  return -1; // todo
}

int getMacAddressLanViaIDStorage(uint8_t *retstr, uint8_t *rawstr) { // via IDStorage
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x119, buf); 
  if( ret >= 0 ) {

    if( buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF ) // no mac here
      return -1234; // "NO DATA"
  
    sprintf(rawstr, "%02X%02X%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    sprintf(retstr, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
  }
  return ret;
}

int getMacAddressBluetooth(uint8_t *retstr, uint8_t *rawstr) { // via ksceBtGetRegisteredInfo  TODO 
  int ret;  
  SceBtRegisteredInfo info;

  //int ksceBtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo * info, SceSize info_size);

  ret = psvident_BtGetRegisteredInfo(0, 0, &info, sizeof(SceBtRegisteredInfo)); // args??
  if( ret >= 0 ) {

    sprintf(rawstr, "%02X%02X%02X%02X%02X%02X", info.mac[0], info.mac[1], info.mac[2], info.mac[3], info.mac[4], info.mac[5]);
    sprintf(retstr, "%02X:%02X:%02X:%02X:%02X:%02X", info.mac[0], info.mac[1], info.mac[2], info.mac[3], info.mac[4], info.mac[5]);
  }

  return ret;
}

int getMacAddressBluetoothViaIDStorage(uint8_t *retstr, uint8_t *rawstr) { // via IDStorage 
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); 
  if( ret >= 0 ) {

    if( buf[0xF0] == 0xFF && buf[0xF1] == 0xFF && buf[0xF2] == 0xFF && buf[0xF3] == 0xFF ) // no mac here
      return -1234; // "NO DATA"
  
    sprintf(rawstr, "%02X%02X%02X%02X%02X%02X", buf[0xF0], buf[0xF1], buf[0xF2], buf[0xF3], buf[0xF4], buf[0xF5]);
    sprintf(retstr, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0xF0], buf[0xF1], buf[0xF2], buf[0xF3], buf[0xF4], buf[0xF5]);
  }
  return ret;
}

int getSoCRevision(uint8_t *retstr, uint32_t *rawval) { // aka Kermit 
  int ret = -1;
  unsigned int soc = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_pervasive_GetSoCRevision(&soc); // revision0 register
    if( ret != 0 )
      return ret;
  } else return -1111;
    
  // https://wiki.henkaku.xyz/vita/Pervasive#revision0
  // from 4.2 its not ES "Engineering Sample" // todo < 4.2 "ESx.yz"

  *rawval = soc;

  sprintf(retstr, "%X.%X %s", (soc & 0xF0) >> 4, soc & 0xF, ((soc & 0x1FF00) >> 8 == 0) ? "" : "(new)"); // Major.Minor
  
  return ret;
}

int getSoC(uint8_t *retstr, uint32_t *rawval) { // aka Kermit 
  int ret = -1;
  unsigned int soc = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_pervasive_GetSoCRevision(&soc); // revision0 register
    if( ret != 0 )
      return ret;
  } else return -1111;
  
  // https://wiki.henkaku.xyz/vita/Kermit
  // https://wiki.henkaku.xyz/vita/Pervasive#revision0
  
  *rawval = soc;

  switch( soc ) {
  //case 0x00000020: sprintf(retstr, "");  break;           // 2.0
  //case 0x80000032: sprintf(retstr, ""); break;            // 3.2             CEM
  //case 0x00000032: sprintf(retstr, "T9ML6TLxxxx"); break; // 3.2             DEM
    case 0x80000040: sprintf(retstr, "T9ML7MBG-S");  break; // 4.0             CEM
    case 0x00000040: sprintf(retstr, "T9ML7MBG-S");  break; // 4.0             DEM
    case 0x80000042: sprintf(retstr, "CXD5315GG");   break; // 4.2   IRS-002   Fat                        2x 256MiB LPDDR2
    case 0x00000042: sprintf(retstr, "CXD5315GG-1"); break; // 4.2   IRT-002   Devkit                     4x 256MiB LPDDR2
    case 0x80000115: sprintf(retstr, "CXD5316GG");   break; // new   IRS-1001  new Fat & Slim & PSTV      2x 256MiB LPDDR2 
    case 0x94000115: sprintf(retstr, "CXD5316BGG");  break; // new   USS-1002  new Slim                   1x 512MiB LPDDR2
    
    default: sprintf(retstr, "unknown"); break;
  }

  return ret;
}

int getSoCDram(uint8_t *retstr, uint32_t *rawval) { // aka Kermit 
  int ret = -1;
  unsigned int soc = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_pervasive_GetSoCRevision(&soc); // revision0 register
    if( ret != 0 )
      return ret;
  } else return -1111;
  
  // https://wiki.henkaku.xyz/vita/Kermit
  // https://wiki.henkaku.xyz/vita/Pervasive#revision0
  
  *rawval = soc;

  if( soc & 0x80000000 ) // "Disable LPDDR2SUB"
    sprintf(retstr, "512 MiB"); // -> retail
  else 
    sprintf(retstr, "1 GiB"); // -> devkit


  if( soc & 0x10000000 ) 
    strcat(retstr, " (1x LPDDR2)");

  else if( soc & 0x20000000 ) 
    strcat(retstr, " (4x LPDDR2)");

  else
    strcat(retstr, " (2x LPDDR2)");


  return ret;
}

int getHardwareInfo(uint8_t *retstr, uint32_t *rawval) { // IDU mode can alter this!!
  int ret = -1;
  unsigned char hwinfo[4];
  
  ret = _vshSysconGetHardwareInfo(hwinfo);
  if( ret >= 0 ) {
    *rawval = *(uint32_t*)&hwinfo[0];
    sprintf(retstr, "%02X %02X %02X %02X", hwinfo[3], hwinfo[2], hwinfo[1], hwinfo[0]);
  }

  return ret;
}

int getHardwareInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval) { // via IDStorage
  int ret = -1;
  unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103
  if( ret >= 0 ) {
    *rawval = *(uint32_t*)&buf[0];
    sprintf(retstr, "%02X %02X %02X %02X", buf[3], buf[2], buf[1], buf[0]);
  }

  return ret;
}

int getKibanId(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;

  uint8_t str[0x21];
  memset(str, 0, 0x21);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x4E0, str, 0x20);
    if( ret < 0 )
      return ret;
  } else return -1111;
  
  //snprintf(str, 0x20, &nvs[0xE0]);

  sprintf(rawstr, "%s", str);
  sprintf(retstr, "%c%c%c-%c%c%c%c%c%c%c-%c%c%c%c%c-%c%c%c%c%c%c%c", str[0x00], str[0x01], str[0x02], str[0x03], str[0x04], str[0x05], str[0x06], str[0x07], str[0x08], str[0x09], str[0x0A], str[0x0B], str[0x0C], str[0x0D], str[0x0E], str[0x0F], str[0x10], str[0x11], str[0x12], str[0x13], str[0x14], str[0x15]);

  return ret;
}  
  
int getCpKibanId(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  uint8_t str[0x21];
  unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x102, buf);
  if( ret >= 0 ) { // if fails then either error reading leaf or its prototype that has no modelstring
      
    snprintf(str, 0x21, "%s", &buf[0x48]);
    
    if( str[0] == 0xFF && str[1] == 0xFF && str[2] == 0xFF )
      return -1234; // "NO DATA"

    sprintf(rawstr, "%s", str);
    sprintf(retstr, "%c%c%c-%c%c%c%c%c%c%c-%c%c%c%c%c-%c%c%c%c%c%c%c", str[0x00], str[0x01], str[0x02], str[0x03], str[0x04], str[0x05], str[0x06], str[0x07], str[0x08], str[0x09], str[0x0A], str[0x0B], str[0x0C], str[0x0D], str[0x0E], str[0x0F], str[0x10], str[0x11], str[0x12], str[0x13], str[0x14], str[0x15]);
  }

  return ret;
}

int getCpInfo(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -1111;  
  
  uint16_t version = kblparam.dipsw.cp_version;
  uint16_t id = kblparam.dipsw.cp_build_id;
  
  sprintf(rawstr, "%04X %04X", version, id);

  // Neighborhood outputs like this:
  // cp info.   : bid.4 ver.1301
  // (BoardID + Version)
  sprintf(retstr, "bid.%X ver.%x", id, version);

  return ret;
}

int getCpTimestamp(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -1111;  
  
  uint32_t cptime1 = kblparam.dipsw.cp_timestamp_1;
  //uint32_t cptime2 = kblparam.dipsw.cp_timestamp_2;
  
  //sprintf(rawstr, "0x%X", cptime1);
  *rawval = cptime1;

  SceDateTime time;
  sceRtcConvertTime_tToDateTime(cptime1, &time);
  sprintf(retstr, "%04d-%02d-%02d %02d:%02d", sceRtcGetYear(&time), sceRtcGetMonth(&time), sceRtcGetDay(&time), sceRtcGetHour(&time), sceRtcGetMinute(&time)); 

  return ret;
}

int getOpenPsid(uint8_t *retstr, uint8_t *rawstr) { // could also get via sysroot / kBL param directly
  int i, ret;
  SceKernelOpenPsId id;
  static unsigned char string[64];
  static char helper[8];
  memset(string, 0, 64);
  
  ret = sceKernelGetOpenPsId(&id);
  if( ret >= 0 ) { 
    for( i = 0; i < 16; i++ ) {
      sprintf(helper, "%02X", (unsigned char)id.id[i]);
      strcat(string, helper);
    } 
    sprintf(rawstr, string);
    sprintf(retstr, string);
  }

  return ret;
}

int getOpenPsidViaIDStorage(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1, i =0;
  static unsigned char string[64];
  static unsigned char buf[0x200];
  static char helper[8];
  memset(string, 0, 64);

  ret = vshIdStorageReadLeaf(0x1, buf); // the leaf where the system / sceKernelGetOpenPsId() actually gets it from is 0x46 (+0x47)  and rest are copies.. like this one
  if( ret >= 0 ) { // success
    for( i = 0; i < 16; i++ ) {
      sprintf(helper, "%02X", buf[i+0x1D0]);
      strcat(string, helper);
    } 
    sprintf(rawstr, string);
    sprintf(retstr, string);
  }

  return ret;

}

int getPscode(uint8_t *retstr, uint8_t *rawstr) { // https://www.psdevwiki.com/vita/PSCode // could also get via sysroot / kBL param directly
  int ret = -1;
  ScePsCode psc;
  
  ret = _vshSblAimgrGetPscode(&psc);
  if( ret >= 0 ) {
    sprintf(rawstr, "%04X%04X%04X%04X", psc.company_code, psc.product_code, psc.product_sub_code, psc.factory_code);
    sprintf(retstr, "%04X %04X %04X %04X", psc.company_code, psc.product_code, psc.product_sub_code, psc.factory_code);
  }

  return ret;
}

int getPscodeFactory(uint8_t *retstr, uint32_t *rawval) { 
  int ret = -1;
  ScePsCode psc;
  
  ret = _vshSblAimgrGetPscode(&psc);
  if( ret >= 0 ) {
    *rawval = psc.factory_code;
    sprintf(retstr, "0x%X", psc.factory_code);
  }

  return ret;
}

int getEmmcSize(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  unsigned int size = -1;
  static SceMbr mbr;

  if( kernmode ) { // helper plugins available
    ret = psvident_mbr_Get(&mbr);
    if( ret != 0 )
      return ret;
  } else return -1111;
  

  if( strncmp(mbr.magic, "Sony Computer Entertainment Inc.", 0x20) == 0 ) {
    size = mbr.n_sectors * 512; // blocks * block size
  }
  
  *rawval = size;
  //sprintf(string, "%d bytes", size);
  getSizeString(retstr, size);
    
  return ret;
}

int getMbrPartitionEntry(uint8_t *retstr, uint32_t *rawval, int entry, int member) {
  int ret = -1;
  //static unsigned char string[32];
  static SceMbr mbr;

  if( entry > 0x10 ) // there are 0x10 partitions at most (derived from code and data structure of SdStor driver))
    return -2468;

  if( kernmode ) { // helper plugins available
    ret = psvident_mbr_Get(&mbr);
    if( ret != 0 )
      return ret;
  } else return -1111;
  
  if( strncmp(mbr.magic, "Sony Computer Entertainment Inc.", 0x20) == 0 ) {
    switch(member) {
      case 0: // Partition offset (blocks) 
        *rawval = mbr.partitions[entry].start_lba;
        sprintf(retstr, "0x%X", mbr.partitions[entry].start_lba); 
        break; 

      case 1: // Partition size (blocks) 
        *rawval = mbr.partitions[entry].n_sectors; 
        getSizeString(retstr, mbr.partitions[entry].n_sectors * 512); 
        break;

      case 2: // Partition ID
        *rawval =  mbr.partitions[entry].part_id; 
        sprintf(retstr, "%s", part_id(mbr.partitions[entry].part_id)); 
        break; 

      case 3: // Partition Type
        *rawval =  mbr.partitions[entry].part_type; 
        sprintf(retstr, "%s", part_type(mbr.partitions[entry].part_type)); 
        break; 

      case 4: // Partition Flag (boolean)
        *rawval =  mbr.partitions[entry].part_flag; 
        sprintf(retstr, "%s", mbr.partitions[entry].part_flag ? "INACTIVE ": "ACTIVE" );
        break; 

      case 5: // Access control lists
        *rawval =  mbr.partitions[entry].acl;
        sprintf(retstr, "%s", part_acl(mbr.partitions[entry].acl)); 
        break; 

      case 6:  // unused
        *rawval = mbr.partitions[entry].unk;
        sprintf(retstr, "0x%08X", mbr.partitions[entry].unk); 
        break;

      default: sprintf(retstr, "invalid"); *rawval = -1;
    }
  }
    
  return ret;
}

int getEnso(uint8_t *retstr, uint32_t *rawval) { /// bascially check if a mbr copy is located at 0x200
  int ret = -1, val = -1;
  
  // https://github.com/SKGleba/IMCUnlock/blob/master/plugin/imcunlock.c#L17

  if( kernmode ) { // helper plugins available
    ret = psvident_mbr_CheckEnso(&val);
    if( ret >= 0 ) {
      *rawval = val;
      sprintf(retstr, val ? "TRUE" : "FALSE");
    }
  } else return -1111;

  return ret;
}

int getEnsoUninstalled(uint8_t *retstr, uint32_t *rawval) { 
  int ret = -1, val = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_mbr_CheckPreviousEnso(&val);
    if( ret >= 0 ) {
      *rawval = val;
      sprintf(retstr, val ? "TRUE" : "FALSE");
    }
  } else return -1111;

  return ret;
}

int getMtpSerial(uint8_t *retstr, uint8_t *rawstr) { // via IDStorage
  int ret = -1;
  static unsigned char buf[0x200];
  memset(retstr, 0, 256);
  memset(rawstr, 0, 256);

  /********************************
   * PDEL  ..00000000274599200100561
   * PTEL  ..00000003274521200420076
   * PCH1  ..00000003274172224439035 
   * 
   * PCHB1 ..00032744709120680140BD0 (4 extra suffix)
   * PCH2  ..00032744714053442180C61 "
   * VTE   ..00032744721046324840C81 "
   * CPVB2 ..0003TG166M0H800210608A0 "
   */
  
  ret = vshIdStorageReadLeaf(0x112, buf); 
  if( ret >= 0) {        
    int i = 0, j = 0;
    for(i = 1; i < 0x40; i+=2, j++) {
      retstr[j] = buf[i];
      rawstr[j] = buf[i];
    }
  }

  return ret;
}

int getBaryonVersion(uint8_t *retstr, uint32_t *rawval) { 
  int ret = -1, baryonver = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_syscon_GetBaryonVersion(&baryonver);
    if( ret != 0 )
      return ret;
  } else return -1111;

  *rawval = baryonver;
  baryon_string(retstr, baryonver);
  
  return ret;
}

int getBaryonTimestamp(uint8_t *retstr, uint8_t *rawstr) { 
  int ret = -1;
  unsigned long long baryonver = -1;
  
  if( kernmode ) { // helper plugins available
    ret = psvident_syscon_GetBaryonTimestamp(&baryonver);
    if( ret != 0 )
      return ret;
  } else return -1111;

  //eg: 32 30 31 32 31 31 30 38 31 37 30 34 | 201211081704 
  sprintf(retstr, "0x%llX", baryonver);  // 0x2ED91D73E8 ???
  sprintf(rawstr, "0x%llX", baryonver);  // 0x2ED91D73E8 ???

  return ret;
}

int getBootloaderVersion(uint8_t *retstr, uint32_t *rawval) {  // via kblparam 
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  *rawval = kblparam.bootldr_revision;
  sprintf(retstr, "0x%X", kblparam.bootldr_revision); // uint32_t

  return ret;
}

int getDipSwitches(uint8_t *retstr, uint32_t *rawval, int no) { // via kblparam 
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  uint8_t ds[0x20];
  memcpy(ds, &kblparam.dipsw, sizeof(SceDipsw));

  switch( no ) {
    case 0: // cp_timestamp_1 (0-
      *rawval = kblparam.dipsw.cp_timestamp_1; 
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x00], ds[0x01], ds[0x02], ds[0x03]);
      break; 
    case 1: // cp_version
      *rawval = kblparam.dipsw.cp_version;  
      sprintf(retstr, "%02X%02X", ds[0x04], ds[0x05]);
      break;
    case 2: // cp_build_id
      *rawval = kblparam.dipsw.cp_build_id;  
      sprintf(retstr, "%02X%02X", ds[0x06], ds[0x07]);
      break; 
    case 3: // cp_timestamp_2 -95)
      *rawval = kblparam.dipsw.cp_timestamp_2;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x08], ds[0x09], ds[0x0A], ds[0x0B]);
      break; 
    case 4: // aslr_seed (96-127)
      *rawval = kblparam.dipsw.aslr_seed;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x0C], ds[0x0D], ds[0x0E], ds[0x0F]);
      break; 

    case 5: // sce_sdk_flags (128-159)
      *rawval = kblparam.dipsw.sce_sdk_flags;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x10], ds[0x11], ds[0x12], ds[0x13]);
      break; 
    case 6: // shell_flags (160-191)
      *rawval = kblparam.dipsw.shell_flags;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x14], ds[0x15], ds[0x16], ds[0x17]);
      break; 
    case 7: // debug_control_flags (192-223)
      *rawval = kblparam.dipsw.debug_control_flags;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x18], ds[0x19], ds[0x1A], ds[0x1B]);
      break; 
    case 8: // system_control_flags (224-255)
      *rawval = kblparam.dipsw.system_control_flags;  
      sprintf(retstr, "%02X%02X%02X%02X", ds[0x1C], ds[0x1D], ds[0x1E], ds[0x1F]);
      break; 
  }

  return ret;
}

int getBootFlags(uint8_t *retstr, uint8_t *rawstr) {  // via kblparam 
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.boot_flags[0], kblparam.boot_flags[1], kblparam.boot_flags[2], kblparam.boot_flags[3], kblparam.boot_flags[4], kblparam.boot_flags[5], kblparam.boot_flags[6], kblparam.boot_flags[7], kblparam.boot_flags[8], kblparam.boot_flags[9], kblparam.boot_flags[0xA], kblparam.boot_flags[0xB], kblparam.boot_flags[0xC], kblparam.boot_flags[0xD], kblparam.boot_flags[0xE], kblparam.boot_flags[0xF]);
  sprintf(rawstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.boot_flags[0], kblparam.boot_flags[1], kblparam.boot_flags[2], kblparam.boot_flags[3], kblparam.boot_flags[4], kblparam.boot_flags[5], kblparam.boot_flags[6], kblparam.boot_flags[7], kblparam.boot_flags[8], kblparam.boot_flags[9], kblparam.boot_flags[0xA], kblparam.boot_flags[0xB], kblparam.boot_flags[0xC], kblparam.boot_flags[0xD], kblparam.boot_flags[0xE], kblparam.boot_flags[0xF]);

  return ret;
}

int getHardwareFlags(uint8_t *retstr, uint8_t *rawstr) {  // via kblparam 
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.hardware_flags[0], kblparam.hardware_flags[1], kblparam.hardware_flags[2], kblparam.hardware_flags[3], kblparam.hardware_flags[4], kblparam.hardware_flags[5], kblparam.hardware_flags[6], kblparam.hardware_flags[7], kblparam.hardware_flags[8], kblparam.hardware_flags[9], kblparam.hardware_flags[0xA], kblparam.hardware_flags[0xB], kblparam.hardware_flags[0xC], kblparam.hardware_flags[0xD], kblparam.hardware_flags[0xE], kblparam.hardware_flags[0xF]);
  sprintf(rawstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.hardware_flags[0], kblparam.hardware_flags[1], kblparam.hardware_flags[2], kblparam.hardware_flags[3], kblparam.hardware_flags[4], kblparam.hardware_flags[5], kblparam.hardware_flags[6], kblparam.hardware_flags[7], kblparam.hardware_flags[8], kblparam.hardware_flags[9], kblparam.hardware_flags[0xA], kblparam.hardware_flags[0xB], kblparam.hardware_flags[0xC], kblparam.hardware_flags[0xD], kblparam.hardware_flags[0xE], kblparam.hardware_flags[0xF]);

  return ret;
}

int getQAFlags(uint8_t *retstr, uint8_t *rawstr) {  // via kblparam 
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  sprintf(rawstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.qa_flags[0], kblparam.qa_flags[1], kblparam.qa_flags[2], kblparam.qa_flags[3], kblparam.qa_flags[4], kblparam.qa_flags[5], kblparam.qa_flags[6], kblparam.qa_flags[7], kblparam.qa_flags[8], kblparam.qa_flags[9], kblparam.qa_flags[0xA], kblparam.qa_flags[0xB], kblparam.qa_flags[0xC], kblparam.qa_flags[0xD], kblparam.qa_flags[0xE], kblparam.qa_flags[0xF]);
  sprintf(retstr, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", kblparam.qa_flags[0], kblparam.qa_flags[1], kblparam.qa_flags[2], kblparam.qa_flags[3], kblparam.qa_flags[4], kblparam.qa_flags[5], kblparam.qa_flags[6], kblparam.qa_flags[7], kblparam.qa_flags[8], kblparam.qa_flags[9], kblparam.qa_flags[0xA], kblparam.qa_flags[0xB], kblparam.qa_flags[0xC], kblparam.qa_flags[0xD], kblparam.qa_flags[0xE], kblparam.qa_flags[0xF]);
  sprintf(rawstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", kblparam.qa_flags[0], kblparam.qa_flags[1], kblparam.qa_flags[2], kblparam.qa_flags[3], kblparam.qa_flags[4], kblparam.qa_flags[5], kblparam.qa_flags[6], kblparam.qa_flags[7], kblparam.qa_flags[8], kblparam.qa_flags[9], kblparam.qa_flags[0xA], kblparam.qa_flags[0xB], kblparam.qa_flags[0xC], kblparam.qa_flags[0xD], kblparam.qa_flags[0xE], kblparam.qa_flags[0xF]);

  return ret;
}

int getQATokenName(uint8_t *retstr) { // via SceSblQafMgr
  int ret = -1;
  static unsigned char string[64];
  
  memset(string, 0, sizeof(string));

  /// https://wiki.henkaku.xyz/vita/SceSblSsMgr#SceSblQafMgr
  ret = sceSblQafManagerGetQafNameForUser(string, sizeof(string)); // return 0 on success
  if( ret >= 0 ) { // returns 0x800F0726 when nothing found
    sprintf(retstr, string);
    retstr[strlen(retstr)-1] = 0;
  }

  return ret;
}

int getQATokenNameViaNvs(uint8_t *retstr) { // via NVS
  int ret = -1;

  static uint8_t nvs[0x80];
  memset(nvs, 0, 0x80);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x400, nvs, 0x80);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  // check magic
  if( nvs[0x0] == 'q' && nvs[0x1] == 'a' && nvs[0x2] == 'f' ) {
    sprintf(retstr, &nvs[0x8]);
    retstr[strlen(retstr)-1] = 0;
  } else {
    return -2468; // UNEXPECTED DATA
  }

  return ret;
}

int getQATokenTarget(uint8_t *retstr, uint32_t *rawval) { // via Template suffix (1 = Internal / 0 = External Token)
  int ret = -1;
  SceKblParam kblparam;
  static unsigned char string[64];
  memset(string, 0, sizeof(string));

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;

  *rawval = -1; // error
  memset(retstr, 0, 256);

  ret = getQATokenName(string); // via sceSblQafMgr
  if( ret < 0 ) { // failed
    ret = getQATokenNameViaNvs(string); // via NVS directly
    if( ret < 0 ) {
      return ret;
    }
  }

  if( string[strlen(string)-1] == 'I' ) {
    *rawval = 1; // True
    sprintf(retstr, "Internal");

  } else if( string[strlen(string)-1] == 'E' ) {
    *rawval = 0; // False
      sprintf(retstr, "External");

  } else {
      sprintf(retstr, "unknown");
  }
  
  return ret;
}

int getBootTypeIndicator1(uint8_t *retstr, uint32_t *rawval) { // via kblparam
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -1111;  
  
  *rawval = kblparam.boot_type_indicator_1;
  sprintf(retstr, "0x%X", kblparam.boot_type_indicator_1); // todo https://wiki.henkaku.xyz/vita/KBL_Param#Boot_type_indicator_1

  return ret;
}

int getWakeupFactor(uint8_t *retstr, uint32_t *rawval) { // via kblparam  
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -1111;
  
  *rawval = kblparam.wakeup_factor;
  sprintf(retstr, "0x%X", kblparam.wakeup_factor); // todo https://wiki.henkaku.xyz/vita/KBL_Param#Wakeup_Factor

  return ret;
}

int getWakeupReqFactor(uint8_t *retstr, uint32_t *rawval) { // via kblparam  
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); 
    if( ret != 0 )
      return ret;
  } else return -1111;
  
  *rawval = kblparam.unk_C0;
  sprintf(retstr, "0x%X", kblparam.unk_C0); // todo https://wiki.henkaku.xyz/vita/KBL_Param#Sleep_Factor

  return ret;
}

int getEnterButton(uint8_t *retstr, uint32_t *rawval) {  
  int enter_button = 0;
  int ret = sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
  if( ret >= 0 ) {
    *rawval = enter_button;
    sprintf(retstr, enter_button ? "Cross" : "Circle");
  }
  return ret;
}

int getSystemUsername(uint8_t *retstr, uint8_t *rawstr) {
  static SceChar8 username[SCE_SYSTEM_PARAM_USERNAME_MAXSIZE];
  int ret = sceAppUtilSystemParamGetString(SCE_SYSTEM_PARAM_ID_USERNAME, username, SCE_SYSTEM_PARAM_USERNAME_MAXSIZE);
  if( ret >= 0 ) {
    sprintf(retstr, (char *)username);
    sprintf(rawstr, (char *)username);
  }
  return ret;
}

int getTouchpanelInfoViaSyscon(uint8_t *retstr, uint32_t *rawval, int option) { // actual current values (seems when backtouch is disconnected getting front values fails too)
  //SceKernelTouchpanelDeviceInfo info; // 0.931-3.60
  SceKernelTouchpanelDeviceInfo2 info; // 0.990-3.60
  int ret = -1;
  
  if( kernmode ) { // helper plugins available
    //ret = psvident_syscon_GetTouchpanelDeviceInfo(&info);
    ret = psvident_syscon_GetTouchpanelDeviceInfo2(&info); 
    if( ret != 0 )
      return ret;
  } else return -1111;  
  
  switch(option) {
    case 0: sprintf(retstr, "%04X", info.FrontVendorID); *rawval = info.FrontVendorID; break;
    case 1: sprintf(retstr, "%04X", info.FrontFwVersion); *rawval = info.FrontFwVersion; break;
    case 2: sprintf(retstr, "%04X", info.FrontConfRev); *rawval = info.FrontConfRev; break;
    case 3: sprintf(retstr, "%04X", info.BackVendorID); *rawval = info.BackVendorID; break;
    case 4: sprintf(retstr, "%04X", info.BackFwVersion); *rawval = info.BackFwVersion; break;
    case 5: sprintf(retstr, "%04X", info.BackConfRev); *rawval = info.BackConfRev; break;
    default: sprintf(retstr, "error");
  }    
  
  return ret;
}

int getTouchpanelInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval, int option) { // factory installed values 
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103
  if( ret >= 0 ) {
    switch(option) {
      case 0: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xC0]); *rawval = *(uint16_t*)&buf[0xC0]; break; // FrontVendorID
      case 1: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xC2]); *rawval = *(uint16_t*)&buf[0xC2]; break; // FrontFwVersion
      case 2: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xC8]); *rawval = *(uint16_t*)&buf[0xC8]; break; // FrontConfRev
      case 3: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xC4]); *rawval = *(uint16_t*)&buf[0xC4]; break; // BackVendorID
      case 4: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xC6]); *rawval = *(uint16_t*)&buf[0xC6]; break; // BackFwVersion
      case 5: sprintf(retstr, "%04X", *(uint16_t*)&buf[0xCA]); *rawval = *(uint16_t*)&buf[0xCA]; break; // BackConfRev
      case 6: sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X", buf[0xD0], buf[0xD1], buf[0xD2], buf[0xD3], buf[0xD4], buf[0xD5], buf[0xD6], buf[0xD7]); *rawval = *(uint32_t*)&buf[0xD0]; break; // FrontTouchpanelLotInfo
      case 7: sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X", buf[0xD8], buf[0xD9], buf[0xDA], buf[0xDB], buf[0xDC], buf[0xDD], buf[0xDE], buf[0xDF]); *rawval = *(uint32_t*)&buf[0xD8]; break; // BackTouchpanelLotInfo
      default: sprintf(retstr, "error");
    }
  }
  return ret;
}

int getMotionSensorInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval, int option) {
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103
  if( ret >= 0 ) {
    switch(option) {
      case 0: sprintf(retstr, "%04X", *(uint16_t*)&buf[0x80]); *rawval = *(uint16_t*)&buf[0x80]; break; // "Motion Firmware Revision"
      case 1: sprintf(retstr, "%04X", *(uint16_t*)&buf[0x82]); *rawval = *(uint16_t*)&buf[0x82]; break; // "Motion Sensor Hardware Information"
      default: sprintf(retstr, "error");
    }
  }
  return ret;
}

int getActivationStatus(uint8_t *retstr, uint32_t *rawval) { 
  int ret = -1;

  if( kernmode ) { // helper plugins available
    ret = psvident_GetActivationStatus(); 
  } else return -1111;

  *rawval = ret;
  
  switch(ret) {
    case -1: sprintf(retstr, "Not initialized"); break;
    case  0: sprintf(retstr, "Activated"); break;
    case  1: sprintf(retstr, "Expired"); break;
    case  2: sprintf(retstr, "RTC battery failure"); break;      
    default: sprintf(retstr, "error"); // error
  }
  
  return 0;
}

int getActivationPeriodNvs(uint8_t *retstr, uint32_t *rawval) { // from NVS
  int ret = -1;
  int nvs_act_start = -1;
  int nvs_act_end = -1;

  static uint8_t nvs[0x20];
  memset(nvs, 0, 0x20);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x520, nvs, 0x20);
    if( ret < 0 )
      return ret;
  } else return -1111;
  
  /// check valid magic
  if( nvs[0x0] == 'a' && nvs[0x1] == 'c' && nvs[0x2] == 't' ) {
    nvs_act_end   = *(int *)(nvs + 0x8);
    nvs_act_start = *(int *)(nvs + 0xC);
  } else return -1234; // "NO DATA" rather then "UNEXPECTED DATA" here because kits may never been activated so no magic either


  SceDateTime start, end;
  sceRtcConvertTime_tToDateTime(nvs_act_start, &start);
  sceRtcConvertTime_tToDateTime(nvs_act_end, &end);
  sprintf(retstr, "%04d/%02d/%02d %02d:%02d to %04d/%02d/%02d %02d:%02d", sceRtcGetYear(&start), sceRtcGetMonth(&start), sceRtcGetDay(&start), sceRtcGetHour(&start), sceRtcGetMinute(&start), sceRtcGetYear(&end), sceRtcGetMonth(&end), sceRtcGetDay(&end), sceRtcGetHour(&end), sceRtcGetMinute(&end));

  *rawval = nvs_act_start; 
  //sprintf(rawstr, "0x%08X to 0x%08X", nvs_act_start, nvs_act_end);

  return ret;
}

int getActivationPeriodDat(uint8_t *retstr, uint32_t *rawval) { // from act.dat
  SceUID fd;
  char buf[0x100];
  unsigned int start_date = -1, end_date = -1;
  
  fd = sceIoOpen("tm0:activate/act.dat", SCE_O_RDONLY, 0777);
  if( fd < 0 )
    return fd;
  
  sceIoRead(fd, buf, sizeof(buf));
  sceIoClose(fd);
  
  start_date = *(unsigned int *)(buf + 0xC);
  end_date = *(unsigned int *)(buf + 0x10);
  
  *rawval = start_date; 
  //sprintf(rawstr, "0x%08X to 0x%08X", start_date, end_date);
  
  SceDateTime start, end;
  sceRtcConvertTime_tToDateTime(start_date, &start);
  sceRtcConvertTime_tToDateTime(end_date, &end);
  sprintf(retstr, "%04d-%02d-%02d to %04d-%02d-%02d", sceRtcGetYear(&start), sceRtcGetMonth(&start), sceRtcGetDay(&start), sceRtcGetYear(&end), sceRtcGetMonth(&end), sceRtcGetDay(&end)); 

  return 0;
}

int getActivationCountNvs(uint8_t *retstr, uint32_t *rawval) { // from NVS
  int ret = -1;
  int nvs_act_count = -1;

  static uint8_t nvs[0x20];
  memset(nvs, 0, 0x20);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x520, nvs, 0x20);
    if( ret < 0 )
      return ret;
  } else return -1111;
  
  // check magic
  if( nvs[0x0] == 'a' && nvs[0x1] == 'c' && nvs[0x2] == 't' ) {
    nvs_act_count = *(int *)(nvs + 0x4);
  } else return -1234; // "NO DATA" rather then "UNEXPECTED DATA" here because kits may never been activated so no magic either

  *rawval = nvs_act_count; 
  sprintf(retstr, "%d", nvs_act_count);
  
  return ret;
}

int getActivationCountDat(uint8_t *retstr, uint32_t *rawval) { // from act.dat
  SceUID fd;
  char buf[0x100];
  unsigned int value = -1;
  
  fd = sceIoOpen("tm0:activate/act.dat", SCE_O_RDONLY, 0777);
  if( fd < 0 )
    return fd;
  
  sceIoRead(fd, buf, sizeof(buf));
  sceIoClose(fd);
  
  value = *(unsigned int *)(buf + 0x8); //at 0x8 in file
  
  *rawval = value; 
  sprintf(retstr, "%d", value); 

  return 0;
}

int getMemCardType(uint8_t *retstr, uint32_t *rawval) { 
  /*
    type 5 for final hardware. Lower would be Engineering Sample Stages there were 4 (those labeled "Sample" - Not for sale)
  */
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    *rawval = info.unk_0x00; // info.ms_type;
    sprintf(retstr, "%d", info.unk_0x00); 
  }
  return ret;
}

int getMemCardSize(uint8_t *retstr, uint8_t *rawstr) { 
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    sprintf(rawstr, "%llu", info.nbytes); 
    getSizeString(retstr, info.nbytes);
  }
  return ret;
}

int getMemCardDate(uint8_t *retstr, uint32_t *rawval) { // Samples: https://pastebin.com/raw/VZ1MSzs1 thx cat
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    // 20 07 DC 01 14 .... -> 07DC = 2012
    *rawval = info.id[1] * 0x100 + info.id[2]; // info.id.manuf_year;
    sprintf(retstr, "%04d-%02d-%02d %02d:%02d:%02d (%X)", info.id[1] * 0x100 + info.id[2], info.id[3], info.id[4], info.id[5], info.id[6], info.id[7], info.id[0]); 
  }
  return ret;
}

int getMemCardFsoffset(uint8_t *retstr, uint32_t *rawval) { 
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    *rawval = info.fs_offset; 
    sprintf(retstr, "0x%X", info.fs_offset); 
  }
  return ret;
}

int getMemCardSectorSize(uint8_t *retstr, uint32_t *rawval) { 
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    *rawval = info.sector_size; // .sector_size_low
    sprintf(retstr, "%d", info.sector_size); 
  }
  return ret;
}

int getMemCardReadonly(uint8_t *retstr, uint32_t *rawval) { 
  SceMsInfo info;
  int ret = vshMsifGetMsInfo(&info);
  if( ret >= 0 ) {
    *rawval = info.unk_0x04; // .is_read_only
    sprintf(retstr, "%s", info.unk_0x04 ? "TRUE" : "FALSE"); 
  }
  return ret;
}

int getBatteryVersionHwinfo(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  unsigned int fwinfo, dfinfo;
  unsigned long long int hwinfo;

  if( kernmode ) { // helper plugins available
    ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
    if( ret != 0 )
      return ret;
  } else return -1111;

  *rawval = (unsigned int)hwinfo;
  sprintf(retstr, "0x%X", (unsigned int)hwinfo);

  return ret;
}

int getBatteryVersionFwinfo(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  unsigned int fwinfo, dfinfo;
  unsigned long long int hwinfo;

  if( kernmode ) { // helper plugins available
    ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  *rawval = (unsigned int)fwinfo;
  sprintf(retstr, "0x%X", (unsigned int)fwinfo);

  return ret;
}

int getBatteryVersionDfinfo(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  unsigned int fwinfo, dfinfo;
  unsigned long long int hwinfo;

  if( kernmode ) { // helper plugins available
    ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
    if( ret != 0 )
      return ret;
  } else return -1111;  

  *rawval = (unsigned int)dfinfo;
  sprintf(retstr, "0x%X", (unsigned int)dfinfo);

  return ret;
}

int getBatterySerial(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x100] == 0xFF && buf[0x101] == 0xFF && buf[0x102] == 0xFF ) // no string (eg pdel)
      return -1234; // "NO DATA"

    if( buf[0x100] == 0x00 && buf[0x101] == 0x00 && buf[0x102] == 0x00 ) // no string (eg pstv)
      return -1234; // "NO DATA"

    /** Examples
     * 
     * "SP65M" or "SP65X" (later models)
     * 65MCO1Z11AA0091J1WGF - PTEL100000010000 -> 20111211AJWGF
     * 65XCO2X05AA2705SYWAF - PCH01000ZA040000 ->           
     * 65MCO1X19AA0869AEWBF - PCH01004ZA010000 -> 
     * 65MCO1X20AA1310J1WAG - PCH01100AB020002 -> 
     * 65MCO2615DA1798J1WBF - PCH01100AB020002 -> 20120615DJWBF
     * 65MCO2304BA1958AEWGF - PCH01104ZA010003 -> 
     *      ^^^^^     ^ ^^^                          ^^^^^^^^^^
     * 
     * "SP86R"
     * 86RCO3407AA0954SYWLS - CPV02000ZA160000 -> 20130407ASWLS "SAMPLE"
     * 86RCO6620CC0729SYW1G - CPVB2000ZA260000 -> 20160620CSW1G
     * 86RCO4421AA0386SYW1F - PCH02004ZX110000 -> 
     * 86RCO4625AA0367SYW1G - PCH02000ZA130000 -> 
     *      ^^^^^     ^ ^^^                          ^^^^^^^^^^ 
     * 
     * ... so X = 10 Oct, Y = 11 Nov, Z = 12 Dec
     */

    snprintf(rawstr, 0x20+1, "%s", &buf[0x100]);
    snprintf(retstr, 0x20+1, "%s", &buf[0x100]);
  }
    
  return ret;
}

int getBatterySerialDate(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static unsigned char buf[0x200];
  
  unsigned char *month(char m) {
    switch(m) {
      case '1': return "Jan";
      case '2': return "Feb";
      case '3': return "Mar";
      case '4': return "Apr";
      case '5': return "May";
      case '6': return "Jun";
      case '7': return "Jul";
      case '8': return "Aug";
      case '9': return "Sep";
      case 'X': return "Oct";
      case 'Y': return "Nov";
      case 'Z': return "Dec";
    }
    return "error";
  }

  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {
  
    if( buf[0x100] == 0xFF && buf[0x101] == 0xFF && buf[0x102] == 0xFF ) // no string (eg pdel)
      return -1234; // "NO DATA"

    if( buf[0x100] == 0x00 && buf[0x101] == 0x00 && buf[0x102] == 0x00 ) // no string (eg pstv)
      return -1234; // "NO DATA"

    sprintf(rawstr, "%X%X%X%X", buf[0x105], buf[0x106], buf[0x107], buf[0x108]);
    sprintf(retstr, "201%c %s %c%c", buf[0x105], month(buf[0x106]), buf[0x107], buf[0x108]);
  }

  return ret;
}

int getBatterySerialModel(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x100] == 0xFF && buf[0x101] == 0xFF && buf[0x102] == 0xFF ) // no string (eg pdel)
      return -1234; // "NO DATA"

    if( buf[0x100] == 0x00 && buf[0x101] == 0x00 && buf[0x102] == 0x00 ) // no string (eg pstv)
      return -1234; // "NO DATA"

    sprintf(rawstr, "%X%X%X", buf[0x100], buf[0x101], buf[0x102]);
    sprintf(retstr, "SP%c%c%c", buf[0x100], buf[0x101], buf[0x102]);
  }
  return ret;
}

int getBatterySerialNumber(uint8_t *retstr, uint8_t *rawstr) { // like the actual serial part?!
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x100] == 0xFF && buf[0x101] == 0xFF && buf[0x102] == 0xFF ) // no string (eg pdel)
      return -1234; // "NO DATA"

    if( buf[0x100] == 0x00 && buf[0x101] == 0x00 && buf[0x102] == 0x00 ) // no string (eg pstv)
      return -1234; // "NO DATA"

    sprintf(rawstr, "%c%c%c%c%c%c%c%c%c%c%c", buf[0x109], buf[0x10A], buf[0x10B], buf[0x10C], buf[0x10D], buf[0x10E], buf[0x10F], buf[0x110], buf[0x111], buf[0x112], buf[0x113]);
    sprintf(retstr, "%c%c%c%c%c%c%c%c%c%c%c", buf[0x109], buf[0x10A], buf[0x10B], buf[0x10C], buf[0x10D], buf[0x10E], buf[0x10F], buf[0x110], buf[0x111], buf[0x112], buf[0x113]);
  }
  return ret;
}

int getBatteryCalibrationVoltage(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {
    *rawval =  *(uint16_t*)&buf[0xA8];
    sprintf(retstr, "0x%04X", *(uint16_t*)&buf[0xA8]);
  }

  return ret;
}

int getBatteryCalibrationCurrent(uint8_t *retstr, uint32_t *rawval) {
  int ret = -1;
  static char buf[0x200];
    
  ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {
    *rawval =  *(uint16_t*)&buf[0xAA];
    sprintf(retstr, "0x%04X", *(uint16_t*)&buf[0xAA]);
  }

  return ret;
}

int getBatteryCharging(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerIsBatteryCharging(); // returns SceBool 
  *rawval = ret;
  sprintf(retstr, ret == 1 ? "TRUE" : "FALSE");
  return ret;
}

int getBatteryPowerOnline(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerIsPowerOnline(); // returns SceBool 
  *rawval = ret;
  sprintf(retstr, ret == 1 ? "TRUE" : "FALSE");
  return ret;
}

int getBatteryCycleCount(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryCycleCount();
  *rawval = ret;
  sprintf(retstr, "%d", ret);
  return ret;
}

int getBatteryTempInCelsius(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryTemp();
  *rawval = ret;
  sprintf(retstr, "%0.0f", (float)scePowerGetBatteryTemp() / 100.0);
  return ret;
}

int getBatteryStateOfHealth(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatterySOH();
  *rawval = ret;
  sprintf(retstr,"%d", ret);
  return ret;
}

int getBatteryPercentage(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryLifePercent();
  *rawval = ret;
  sprintf(retstr,"%d", ret);
  return ret;
}

int getBatteryVoltage(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryVolt();
  *rawval = ret;
  sprintf(retstr, "%0.2f", (float)scePowerGetBatteryVolt() / 1000.0);
  return ret;
}

int getBatteryCapacity(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryFullCapacity();
  *rawval = ret;
  sprintf(retstr,"%i", ret);
  return ret;
}

int getBatteryRemCapacity(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryRemainCapacity();
  *rawval = ret;
  sprintf(retstr,"%i", ret);
  return ret;
}

int getBatteryLifeTime(uint8_t *retstr, uint32_t *rawval) {
  int ret = scePowerGetBatteryLifeTime();
  
  *rawval = ret;
  //sprintf(retstr, "%i minutes", ret);
    
  ret *= 60;
  int h = (ret/3600); 
  int m = (ret -(3600*h))/60;
  //int s = (ret -(3600*h)-(m*60));
  sprintf(retstr, "%02d hours, %02d minutes", h, m);

  return ret;
}

int getModeIDU(uint8_t *retstr, uint32_t *rawval) {
  int ret = vshSysconIsIduMode();
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeShow(uint8_t *retstr, uint32_t *rawval) {
  int ret = vshSysconIsShowMode();
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeDownloader(uint8_t *retstr, uint32_t *rawval) {
  int ret = vshSysconIsDownLoaderMode();
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeDevelopment(uint8_t *retstr, uint32_t *rawval) { // via vshSbl (works in usermode)
  int ret = vshSblSsIsDevelopmentMode();
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeDevelopmentViaDip(uint8_t *retstr, uint32_t *rawval) { // via DIP switch https://wiki.henkaku.xyz/vita/KBL_Param#SDK_(SCE)_flags
  int ret = GetDipsw(159);
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeManufacturing(uint8_t *retstr, uint32_t *rawval) {  
  int ret = -1;
  SceKblParam kblparam;

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam); // https://wiki.henkaku.xyz/vita/KBL_Param#Boot_type_indicator_1
    if( ret != 0 )
      return ret;
  } else return -1111;  

  *rawval = kblparam.boot_type_indicator_1 & 0x4;
  sprintf(retstr, "%s", ((kblparam.boot_type_indicator_1 & 0x4) != 0) ? "TRUE" : "FALSE"); // check 4th bit https://github.com/SKGleba/PSP2-batteryFixer/blob/master/kernel/main.c#L89
  
  return ret;
}

int getModePstvForDevkit(uint8_t *retstr, uint32_t *rawval) { // via DIP switch https://wiki.henkaku.xyz/vita/KBL_Param#SDK_(SCE)_flags
  int ret = GetDipsw(152);
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getModeMemSizeForDevkit(uint8_t *retstr, uint32_t *rawval) { // via DIP switch https://wiki.henkaku.xyz/vita/KBL_Param#SDK_(SCE)_flags
  int ret = GetDipsw(128);
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "DevTool" : "Console"); /// Bits 128-159 are used for DevKit Boot Parameters. 
  }
  return ret;
}

int getModeReleaseCheckConsoleForDevkit(uint8_t *retstr, uint32_t *rawval) { // via DIP switch https://wiki.henkaku.xyz/vita/KBL_Param#SDK_(SCE)_flags
  int ret = GetDipsw(129);
  if( ret >= 0 ) {
    *rawval = ret;
    sprintf(retstr, "%s", ret ? "TRUE" : "FALSE");
  }
  return ret;
}

int getFirmwareInternal(uint8_t *retstr, uint32_t *rawval) {  // current firmware!
  
  // check if interal QA token is active for now. (They can only be active on internal and there is only one known firmware that works without QAF requirement. So its no 100% solution but close enough for now)
  /*int ret = -1;
  SceKblParam kblparam;
  static unsigned char string[64];
  memset(string, 0, sizeof(string));

  if( kernmode ) { // helper plugins available
    ret = psvident_kblparam_GetBuffer(&kblparam);
    if( ret != 0 )
      return ret;
  } else return -1111;

  *rawval = 0; // False
  sprintf(retstr, "External");

  if( kblparam.qa_flags[0] != 0x00) { // only check for name now since that takes forever .. for whatever reason
    ret = sceSblQafManagerGetQafNameForUser(string, sizeof(string)); // return 0 on success
    if( ret >= 0 ) {
      if( string[strlen(string)-2] == 'I' ) {
        *rawval = 1; // True
        sprintf(retstr, "Internal");
      }
    }
  }
  
  return ret;*/


  // better:
  // all selfs in os0:sm/ should have 83 FF AD 6D 24 F3 39 64 A0 61 78 8D A0 68 3B 19 @ 0x3A0 if internal (eg: act_sm.self)
  uint8_t data[0x10];
  memset(data, 0, 0x10);

  /// open 
  SceUID fp = sceIoOpen("os0:sm/act_sm.self", SCE_O_RDONLY, 0);
    if( fp < 0 ) {
        return fp;
  }

  /// get and check filesize
  int length = sceIoLseek(fp, 0, SCE_SEEK_END);
  if( length < 0x20 ) 
    return -2468; // "UNEXPECTED DATA"
  
  /// read in data @ offset
  sceIoLseek(fp, 0x3A0, SCE_SEEK_SET);
  if( sceIoRead(fp, data, 0x10) != 0x10 )
    return -1234; // "NO DATA"
  
  sceIoClose(fp);

  // check
  if( data[0] == 0x83 &&  data[1] == 0xFF &&  data[2] == 0xAD &&  data[3] == 0x6D ) { // internal
    *rawval = 1; // TRUE
    sprintf(retstr, "Internal");
  } else {
    *rawval = 0; // FALSE
    sprintf(retstr, "External");
  }

  return 0;
}

int getRegistryWifiSSID(uint8_t *retstr, int profile) { 
  char buffer[64];
  static unsigned char string[64];
  sprintf(buffer, "/CONFIG/NET/%02d/WIFI/", profile);
  int ret = getRegistryString(string, buffer, "ssid");
  if( ret >= 0 ) {
    sprintf(retstr, string);
  }
  return ret;
}

int getRegistryWifiPassword(uint8_t *retstr, int profile) { 
  char buffer[64];
  static unsigned char string[64];
  sprintf(buffer, "/CONFIG/NET/%02d/WIFI/", profile);
  int ret, type = -1;
  
  ret = getRegistryInteger(&type, buffer, "wifi_security");
  if( ret != 0 )
    return ret;
  
  ret = getRegistryString(string, buffer, type == 1 ? "wep_key" : "wpa_key");
  if( ret >= 0 ) {
    sprintf(retstr, string);
  }
  return ret;
}

int getWifiRegion(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x110_-_WlanRegion
  int ret = -1;
  static unsigned char buf[0x200];

  ret = vshIdStorageReadLeaf(0x110, buf);
  if( ret >= 0) {
    *rawval = *(uint32_t*)&buf[0];
    sprintf(retstr, "%02X %02X %02X", buf[0], buf[1], buf[2]);
  }
  return ret;  
}

int getLocalIp(uint8_t *retstr, uint8_t *rawstr) { // via sceNet
  int ret = -1;
  int state;

  memset(retstr, 0, 256);
  memset(rawstr, 0, 256);

    ret = sceNetCtlInetGetState(&state);
  if (ret < 0 || state != SCE_NETCTL_STATE_CONNECTED) {
        return ret; // not connected
    }

  SceNetCtlInfo info;
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    if (ret >= 0) {
        sprintf(rawstr, "%s", info.ip_address);
        sprintf(retstr, "%s", info.ip_address);
    }

  return ret;
}

int getSsid(uint8_t *retstr, uint8_t *rawstr) { // via sceNet
  int ret = -1;
  int state;

  memset(retstr, 0, 256);
  memset(rawstr, 0, 256);


    ret = sceNetCtlInetGetState(&state); // 0 on success
  if (ret < 0 || state != SCE_NETCTL_STATE_CONNECTED) {
        return ret; // not connected
    }

  SceNetCtlInfo info;
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SSID, &info); // 0 on success
    if (ret >= 0) {
        sprintf(rawstr, "%s", info.ssid);
        sprintf(retstr, "%s", info.ssid);
    }

  return ret;
}

int getSimLocked(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x115_-_ProductTypeInfo
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x113, buf);
  if( ret >= 0 ) {

    if( buf[0x88] == 0xFF ) // nothing here
      return -1234; // "NO DATA"

    *rawval = buf[0x88];
    sprintf(retstr, buf[0x88] ? "TRUE" : "FALSE");

  }
  return ret;  
}

int getSimCarrier(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x115_-_ProductTypeInfo
  int ret = -1;
  static char buf[0x200];

  ret = vshIdStorageReadLeaf(0x115, buf);
  if( ret >= 0 ) {
    *rawval = buf[0xF];
    switch( buf[0xF] ) {
      case '0': sprintf(retstr, "NONE"); break;
      case '1': sprintf(retstr, "US Operator"); break;
      case '2': sprintf(retstr, "JP Operator"); break;
      case '3': sprintf(retstr, "EU Generic"); break;
      case '4': sprintf(retstr, "Asia Generic"); break;
      case '5': sprintf(retstr, "Canada Operator"); break;
      case '6': sprintf(retstr, "Mexico Generic"); break;
      default: sprintf(retstr, "Unknown");
    }    
  }
  return ret;
}

int getImeiViaIDStorage(uint8_t *retstr, uint8_t *rawstr) {
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x113, buf);
  if( ret >= 0 ) {
    
    if( buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF ) // no string
      return -1234; // "NO DATA"

    snprintf(rawstr, 0xF, "%s", &buf[0]);
    sprintf(retstr, "%c%c-%c%c%c%c%c%c-%c%c%c%c%c%c-%c", buf[0x00], buf[0x01], buf[0x02], buf[0x03], buf[0x04], buf[0x05], buf[0x06], buf[0x07], buf[0x08], buf[0x09], buf[0x0A], buf[0x0B], buf[0x0C], buf[0x0D], buf[0x0E]);
    
  }
  return ret;
}  

int getTestIccidViaIDStorage(uint8_t *retstr, uint8_t *rawstr) { // ICCID of the SIM used when Factory tested! 
  int ret = -1;
  static char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x102, buf);
  if( ret >= 0 ) {
  
    if( buf[0x148] == 0xFF && buf[0x149] == 0xFF && buf[0x14A] == 0xFF ) // no string
      return -1234; // "NO DATA"

    snprintf(rawstr, 20, "%s", &buf[0x148]);
    snprintf(retstr, 20, "%s", &buf[0x148]);
  }
  return ret;
}

int getAutoAvls(uint8_t *retstr, uint32_t *rawval) {
  static char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x118, buf); // n0ot every vita has this leaf
  if( ret >= 0 ) {  
    *rawval = (unsigned int)buf[0];
    sprintf(retstr, buf[0] ? "TRUE" : "FALSE");
  }
  return ret;
}

int getWaveColor(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x116_-_ColorVariation
  static char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x116, buf); // not every vita has this leaf
  if( ret >= 0 ) {  
    *rawval = buf[1]; // color_index - default Wave color for various colored models
    sprintf(retstr, "%d", buf[1]);
    //sprintf(retstr, buf[1] ? "TRUE" : "FALSE");
  }
  return ret;
}

int getPreinstWave(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x116_-_ColorVariation
  static char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x116, buf); // not every vita has this leaf
  if( ret >= 0 ) {  
    *rawval = buf[0]; // flags & 2 has wave background, located in pd0:wave/waveparam.bin
    sprintf(retstr, buf[0] & 2 ? "TRUE" : "FALSE");
  }
  return ret;
}

int getPreinstTheme(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x116_-_ColorVariation
  static char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x116, buf); // not every vita has this leaf
  if( ret >= 0 ) {  
    *rawval = buf[0]; // flags & 4 has pre-installed theme, located in pd0:theme
    sprintf(retstr, buf[0] & 4 ? "TRUE" : "FALSE");
  }
  return ret;
}

int getDiagnostics(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/IdStorage#0x104
  static char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x104, buf); 
  if( ret >= 0 ) {  
    *rawval = buf[0];
    sprintf(retstr, buf[0] != 0xFF ? "TRUE" : "FALSE");
  }
  return ret;
}

int getOledSerial1(uint8_t *retstr, uint8_t *rawstr) { // not on PDEL?
  static unsigned char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x184] == 0xFF && buf[0x185] == 0xFF && buf[0x186] == 0xFF ) // no string
      return -1234; // "NO DATA"

    snprintf(rawstr, 0x10+1, "%s", &buf[0x184]);
    snprintf(retstr, 0x10+1, "%s", &buf[0x184]);
  }
  return ret;
}

int getUnknown120(uint8_t *retstr, uint8_t *rawstr) {
  static unsigned char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x120] == 0xFF && buf[0x121] == 0xFF && buf[0x122] == 0xFF ) // no string
      return -1234; // "NO DATA"

    /* Examples (only present for 3g units?)
    PCH-1100 AB02: "D3200-STSUGNZ-1601  1  [Apr 20 2011 17:00:00] - 1601.00.04.07.000"
    PCH-1100 AB02: "D3200-STSUGNZ-1601  1  [Apr 20 2011 17:00:00] - 1601.00.04.07.000"
    PCH-1104 ZA01: "D3200-STSUGNZ-1601  1  [Apr 20 2011 17:00:00] - 1601.00.04.06.000"
    */ 

    snprintf(rawstr, 0x64, "%s", &buf[0x120]);
    snprintf(retstr, 0x24, "%s", &buf[0x120]);
    retstr[0x23] = ' ';
    retstr[0x24] = '.';
    retstr[0x25] = '.';
    retstr[0x26] = '\0';
  }
  return ret;
}

int getUnknown194(uint8_t *retstr, uint8_t *rawstr) {
  static unsigned char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x194] == 0xFF && buf[0x195] == 0xFF && buf[0x196] == 0xFF ) // no string
      return -1234; // "NO DATA"

    /* Examples (only present on OLED models)
    PCH-1000 ZA04: TKBA210194800000
    PCH-1104 ZA01: TDB3221558400000
    PCH-1100 AB02: CWB6060538100000
    PCH-1100 AB02: TWB6180404700000
    PDEL-1000:     TDAB181635400000
    */ 

    snprintf(rawstr, 0x10+1, "%s", &buf[0x194]);
    snprintf(retstr, 0x10+1, "%s", &buf[0x194]);
  }
  return ret;
}

int getLcdSerial(uint8_t *retstr, uint8_t *rawstr) {
  static unsigned char buf[0x200];
  int ret = vshIdStorageReadLeaf(0x103, buf);
  if( ret >= 0 ) {

    if( buf[0x1A8] == 0xFF && buf[0x1A9] == 0xFF ) // no string
      return -1234; // "NO DATA"

    snprintf(rawstr, 0x20+1, "%s", &buf[0x1A8]);
    snprintf(retstr, 0x20+1, "%s", &buf[0x1A8]);
  }
  return ret;
}

int getCodename(uint8_t *retstr, uint32_t *rawval) { /// via hardware info 
  unsigned char hwinfo[4];
  int ret = _vshSysconGetHardwareInfo(hwinfo);
  if( ret >= 0 ) {

    *rawval = (unsigned int)hwinfo[2];

    switch( hwinfo[2] ) {
      case 0x10: // early CEMs / NGP
      case 0x31: // DEM (Slideys)
      case 0x40: // PCH-1XXX & CEMs
      case 0x41: // PDEL & "new" DEMs
        sprintf(retstr, "Iris"); 
        break;

      case 0x60: // PCH-1XXX (new) & CPV-B1xxx
        sprintf(retstr, "Iris VA");  // not 100% confirmed though
        break;

      case 0x70: // VTE-10XX 
        sprintf(retstr, "Dolce"); 
        break;

      case 0x72: // VTE-10XX (new) 
        sprintf(retstr, "Dolce 1.2g");
        break;

      case 0x80: // PCH-20XX & CPV-2xxx
        sprintf(retstr, "Ushiwaka");
        break;

      case 0x82: // PCH-20XX (new) & CPV-B2xxx
        sprintf(retstr, "Ushiwaka VA"); 
        break;

    }
  }
  return ret;
}

int getUserApps(uint8_t *retstr, uint32_t *rawval) { // via ux0:iconlayout.ini (pretty simplistic approach but hey)
  char buf[256];
  int i, r, lines = 0;

  SceUID fp = sceIoOpen("ux0:iconlayout.ini", SCE_O_RDONLY, 0);
  if (fp < 0) 
    return fp;

  while((r = sceIoRead(fp, buf, sizeof(buf))) > 0) {
    for(i = 0; i < r; i++) {
      if (buf[i] == '\n')
        lines++;
    }
  } sceIoClose(fp);

  *rawval = lines;
  sprintf(retstr, "%d", lines);
  
  return 0;
}

int getUserThemes(uint8_t *retstr, uint32_t *rawval) { // better via app.db "tbl_theme" (does NOT include pd0:theme)
  SceUID d = sceIoDopen("ux0:theme");
  if( d < 0 )
    return d;

  int count = 0;
  SceIoDirent dir;
  memset(&dir, 0, sizeof(dir));

  while( sceIoDread(d, &dir) > 0 ) {
    if( dir.d_stat.st_mode & SCE_S_IFDIR ) {
      if( strcmp(dir.d_name, ".") != 0 && strcmp(dir.d_name, "..") != 0 )
        count++;
    } memset(&dir, 0, sizeof(dir));
  }

  sceIoDclose(d);

  *rawval = count;
  sprintf(retstr, "%d", count);
  
  return 0;
}

int getPlaylogEntry(uint8_t *retstr, uint32_t *rawval, int entry, int var) {

  int RECORD_SIZE = 0x38;
  memset(retstr, 0, 256);

    SceUID fd = sceIoOpen("ur0:user/00/shell/playlog/playlog.dat", SCE_O_RDONLY, 0); // seems to be 100% updated whenever ps button pressed and game/app exits and app never openend before. otherwise after 5 minutes when app opne / close
    if (fd < 0)
        return fd;

    sceIoLseek(fd, 0x20 + (int64_t)entry * RECORD_SIZE, SCE_SEEK_SET);

    uint8_t buf[RECORD_SIZE];
    int r = sceIoRead(fd, buf, RECORD_SIZE);
    sceIoClose(fd);

    if (r != RECORD_SIZE)
        return -1;

    switch( var ) {
      case 0: // TitleID
        sprintf(retstr, "%s", &buf[0]);
        break;

      case 1: //  timestamp
        SceRtcTick t;
        t.tick = *(SceUInt64*)(buf + 0x18); // eg: 0x00E319C9C4FEA192 = 2026-08-24 13:26:17.000Z ca
        //sprintf(retstr, "0x%llX", t.tick); 

        SceDateTime dt;
        sceRtcConvertTickToDateTime(&t, &dt);
        sprintf(retstr, "%04d-%02d-%02d %02d:%02d", sceRtcGetYear(&dt), sceRtcGetMonth(&dt), sceRtcGetDay(&dt), sceRtcGetHour(&dt), sceRtcGetMinute(&dt));  //, sceRtcGetSecond(&dt), sceRtcGetMicrosecond
        break;

      case 2: // ? only some apps have it? mostly 0. but one vita yes the other no?!
        sprintf(retstr, "%d", *(uint32_t*)(buf + 0x20));
        *rawval =  *(uint32_t*)(buf + 0x20);
        break;

      case 3: // some normalized activity float (0.1 to ?) seen 6.9 for example    the higher the more the app was used recently
        //sprintf(retstr, "%0.4f", *(float*)(buf + 0x28));
        sprintf(retstr, "%f", *(float*)(buf + 0x28));
        *rawval =  *(uint32_t*)(buf + 0x28);
        break;

      case 4: // open app counter?
        *rawval =  *(uint32_t*)(buf + 0x2C);
        break;

      case 5: // button presses
        *rawval =  *(uint32_t*)(buf + 0x30);
        break;

      case 6: // some normalized activity float (0.0 to 1.0)
        //sprintf(retstr, "%0.4f", *(float*)(buf + 0x34));
        sprintf(retstr, "%f", *(float*)(buf + 0x34));
        *rawval =  *(uint32_t*)(buf + 0x34);
        break;

      default:
        return -2;
    }

    return 0;
}

int getDeviceInfo(uint8_t *retstr, uint32_t *rawval, char* device, int option) {
  
  /*
  struct SceIoDevInfo {
    SceOff   max_size   
    SceOff   free_size   
    SceSize cluster_size   
    void *   unk
  }
  */

  memset(retstr, 0, 256);

  SceIoDevInfo info;
  int ret = sceIoDevctl(device, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
  if( ret >= 0 ) {
    switch(option){
      case 0: //sprintf(retstr, "%d", info.max_size);
          getSizeString(retstr, info.max_size);
          *rawval = (uint32_t)info.max_size;
                break;

      case 1: //sprintf(retstr, "%d", info.free_size);
          getSizeString(retstr, info.free_size);
          *rawval = (uint32_t)info.free_size;
                break;

      case 2: sprintf(retstr, "%u", info.cluster_size);
          *rawval = (uint32_t)info.cluster_size;
                break;

      case 3: uint32_t used_size = info.max_size - info.free_size;
          //sprintf(retstr, "%u", used_size);
          getSizeString(retstr, used_size);
          *rawval = (uint32_t)used_size;
                break;
    }
  }
  return ret;
}

int getPupVersion(uint8_t *retstr, uint32_t *rawval, char* pup) { // https://wiki.henkaku.xyz/vita/PUP

  /* https://www.psdevwiki.com/ps3/Playstation_Update_Package_(PUP)#Header_2
  typedef struct ScePupHeader_v2 { // size is 0x80-bytes
    SceUInt8 magic[7];
    SceUInt8 format_flag;
    SceUInt64 format_version;
    SceUInt32 version;
    SceUInt32 buildno;
    SceUInt64 segment_num;
    SceUInt64 file_offset;
    SceUInt64 file_size;
    SceUInt32 sign_algorithm;
    SceUInt32 sign_key_index;
    SceUInt8 attribute[4];
    SceUInt32 target;
    SceUInt32 sub_target;
    SceUInt32 support_list;
    SceUInt32 base_version;
    SceUInt32 base_buildno;
    SceUInt8 unk_0x50[0x30];
  } ScePupHeader_v2;
  */

  uint8_t string[16];
  uint8_t buf[0x80];
  memset(retstr, 0, 256);

    SceUID fd = sceIoOpen(pup, SCE_O_RDONLY, 0);
    if (fd < 0)
        return fd;
  
    int bytes_read = sceIoRead(fd, buf, 0x80);
    sceIoClose(fd);
  
  if( bytes_read != 0x80 )
    return -1234; // "NO DATA"
  
  
  if(buf[0] == 'S' && buf[1] == 'C' && buf[2] == 'E' && buf[3] == 'U' && buf[4] == 'F') { // magic check
    *rawval = *(uint32_t*)&buf[0x10];
    firmware_string(string, *rawval);
    sprintf(retstr, "%s", string);

    if( buf[0x3C] == 0x1 ) sprintf(retstr, "%s (TOOL) #%d", string, *(uint32_t*)&buf[0x14]);
    if( buf[0x3C] == 0x2 ) sprintf(retstr, "%s (CEX) #%d", string, *(uint32_t*)&buf[0x14]);
    if( buf[0x3C] == 0x4 ) sprintf(retstr, "%s (DEX) #%d", string, *(uint32_t*)&buf[0x14]);

    // 0x14 BuildNo  
    // 0x18 No of Files
    // 0x3C Target     - CEX / DEX / TOOL
    // 0x38 Requires   - QAF / MANU
  }

  return 0;
}

int getParamValue(uint8_t *retstr, uint32_t *rawval, char* paramsfo, char *key) { // eg: "gro0:app/PCSE00048/sce_sys/param.sfo" & "TITLE"
  char sfo[4096] = "";

  SceUID fd = sceIoOpen(paramsfo, SCE_O_RDONLY, 0);
  if (fd < 0) 
    return fd;

  int size = sceIoLseek(fd, 0, SCE_SEEK_END);
  if( size <= 0 )
    return -1234; // "NO DATA"

  sceIoLseek(fd, 0, SCE_SEEK_SET);
  sceIoRead(fd, sfo, size);
  sceIoClose(fd);

  memset(retstr, 0, 256);

  GetSfoValueByKey(sfo, key, rawval);
  GetSfoStringByKey(sfo, key, retstr, 256);

  return 0;
}

int getGamecardMounted(uint8_t *retstr) { // returns TITLEID (in gro0:app/) as retstr
  memset(retstr, 0, 256);

  SceUID d = sceIoDopen("gro0:app/");
  if(d < 0) 
    return d;

  SceIoDirent dir;
  memset(&dir, 0, sizeof(dir));

  while( sceIoDread(d, &dir) > 0 ) {
    if( dir.d_stat.st_mode & SCE_S_IFDIR ) {
      if( strcmp(dir.d_name, ".") != 0 && strcmp(dir.d_name, "..") != 0 ) { // first found real folder
        sprintf(retstr, "%s", dir.d_name);
        sceIoDclose(d);
        return 0; // success
      }
    }
    memset(&dir, 0, sizeof(dir));
  }

  sceIoDclose(d);
  return -1;
}

int getGamecardInfo(uint8_t *retstr, uint32_t *rawval, int val) { // 

  // Example: https://forum.gsmhosting.com/vbb/f798/how-remap-memory-teyes-cc2-answered-3020343/

  int ret;
  EmmcCardId cardid;
  uint8_t cid[0x10];
  uint8_t csd[0x10];
  uint8_t ext_csd[0x200];

  memset(cid, 0x00, sizeof(cid));
  memset(csd, 0x00, sizeof(csd));
  memset(ext_csd, 0x00, sizeof(ext_csd));

  if( kernmode ) { // helper plugins available
    ret = psvident_sdifgetcontextmmc(&cardid, cid, csd, ext_csd);
    if( ret != 0 )
      return ret;
  } else return -1111;

  memset(retstr, 0, 256);

  switch(val) {
    case 0: // CID 
      sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", cid[0], cid[1], cid[2], cid[3], cid[4], cid[5], cid[6], cid[7], cid[8], cid[9], cid[0xA], cid[0xB], cid[0xC], cid[0xD], cid[0xE], cid[0xF]);
      break;

    case 1: // CSD
      sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", csd[0], csd[1], csd[2], csd[3], csd[4], csd[5], csd[6], csd[7], csd[8], csd[9], csd[0xA], csd[0xB], csd[0xC], csd[0xD], csd[0xE], csd[0xF]);
      break;

    case 2: // EXT_CSD revision
      sprintf(retstr, "%X", ext_csd[0xC0]);
      *rawval = ext_csd[0xC0];
      break;

  

    case 10: // CRC7
      sprintf(retstr, "0x%02X", cardid.crc7);
      *rawval = cardid.crc7;
      break;

    case 11: // Manufacturer date 
      
      static const char *months[] = { "inv0",  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "inv1", "inv2", "inv3" };

      int revision = ext_csd[0xC0];

      int mdt_year = (cardid.mdt & 0xF) + 1997;
      int mdt_month = cardid.mdt >> 4;

      // https://github.com/torvalds/linux/blob/master/drivers/mmc/core/mmc.c
      if (revision >= 9) { // Adjust production date as per JEDEC JESD84-B51B
        if (mdt_year < 2023)
          mdt_year += 16;

      } else if (revision >= 5) { // Adjust production date as per JEDEC JESD84-B451
        if (mdt_year < 2010)
          mdt_year += 16;
      }

      sprintf(retstr, "%u %s", mdt_year, months[mdt_month]);
      //sprintf(retstr, "%02u/%u", mdt_month, mdt_year); // alternative
      //sprintf(retstr, "0x%02X", cardid.mdt);
      *rawval = cardid.mdt;
      break;

    case 12: // Serial Number
      sprintf(retstr, "0x%08X", cardid.psn);
      *rawval = cardid.psn;
      break;

    case 13: // Product revision
      int prv_min = cardid.prv & 0xF;
      int prv_maj = cardid.prv >> 4;

      sprintf(retstr, "%u.%u", prv_maj, prv_min);
      //sprintf(retstr, "0x%02X", cardid.prv);
      *rawval = cardid.prv;
      break;

    case 14: // Product name 
      snprintf(retstr, 7, "%s", cardid.pnm);
      break;

    case 15: // OEM/Application ID
      sprintf(retstr, "0x%01X", cardid.oid);
      *rawval = cardid.oid;
      break;

    case 16: // Device - https://kernel.googlesource.com/pub/scm/utils/mmc/mmc-utils/+/8f41ccbb40b887325f243bd73695a9e814610520/lsmmc.c#624
      sprintf(retstr, "%s", cardid.cbx == 0 ? "Card" : (cardid.cbx == 1 ? "BGA" : (cardid.cbx == 2 ? "PoP" : "error")));
      *rawval = cardid.cbx;
      break;

    case 17: // Manufacturer ID
      
      switch(cardid.mid) { // MMC DB via https://kernel.googlesource.com/pub/scm/utils/mmc/mmc-utils/+/8f41ccbb40b887325f243bd73695a9e814610520/lsmmc.c#172
        case 0x00: sprintf(retstr, "SanDisk"); break;

        case 0x02: sprintf(retstr, "Kingston/SanDisk"); break;

        case 0x03: 
        case 0x11: sprintf(retstr, "Toshiba"); break;

        case 0x13: 
        case 0xFE: sprintf(retstr, "Micron"); break;

        case 0x15: sprintf(retstr, "Samsung/SanDisk/LG"); break;

        case 0x37: sprintf(retstr, "KingMax"); break;

        case 0x44: sprintf(retstr, "ATP"); break;

        case 0x45: sprintf(retstr, "SanDisk"); break; // actually "SanDisk Corporation"

        case 0x2c: 
        case 0x70: sprintf(retstr, "Kingston"); break;

        default: sprintf(retstr, "Unknown");
      }
    
      //sprintf(retstr, "0x%02X", cardid.mid);
      *rawval = cardid.mid;
      break;


    default: sprintf(retstr, "error");
  } 

  
  
  return 0;
}

int getPspemuFirmware(uint8_t *retstr,  uint32_t *rawval) {
  
  int ret = sceCompatGetPspSystemSoftwareVersion();
  if( ret > 0 ) {
    sprintf(retstr, "%d.%02d", ret / 100, ret % 100);
    *rawval = ret;
    return 0;
  } 
  
  return ret;
}

int getUdcdState(uint8_t *retstr,  uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/SceUdcd#sceUdcdGetDeviceState
  
  int ret;

  /* typedef struct {
    int unk_00;
    int state;
    int cable;
    int connection;
    int use_usb_charging;
    int unk_14;
  } SceUdcdDeviceState; */
 
  SceUdcdDeviceState state;
  ret = sceUdcdGetDeviceState(&state);
  if( ret < 0 ) 
    return ret;

  *rawval = state.state + state.cable + state.connection + state.use_usb_charging;

/* typedef enum SceUdcdStatus {
    SCE_UDCD_STATUS_CONNECTION_NEW          = 0x0001,
    SCE_UDCD_STATUS_CONNECTION_ESTABLISHED  = 0x0002,
    SCE_UDCD_STATUS_CONNECTION_SUSPENDED    = 0x0004,
    SCE_UDCD_STATUS_CABLE_DISCONNECTED      = 0x0010,
    SCE_UDCD_STATUS_CABLE_CONNECTED         = 0x0020,
    SCE_UDCD_STATUS_DEACTIVATED             = 0x0100,
    SCE_UDCD_STATUS_ACTIVATED               = 0x0200,
    SCE_UDCD_STATUS_IS_CHARGING             = 0x0400,
    SCE_UDCD_STATUS_USE_USB_CHARGING        = 0x0800,
    SCE_UDCD_STATUS_UNKNOWN_1000            = 0x1000,
    SCE_UDCD_STATUS_UNKNOWN_2000            = 0x2000
  } SceUdcdStatus; */

  memset(retstr, 0, 256);
  if(state.connection & SCE_UDCD_STATUS_CONNECTION_NEW) {
    strcat(retstr, "NEW, "); // more like "waiting" or "idle"
  } 
  if(state.connection & SCE_UDCD_STATUS_CONNECTION_ESTABLISHED) {
    strcat(retstr, "ESTABLISHED, "); 
  } 
  if(state.connection & SCE_UDCD_STATUS_CONNECTION_SUSPENDED) {
    strcat(retstr, "SUSPENDED, "); 
  } 
  if(state.cable & SCE_UDCD_STATUS_CABLE_DISCONNECTED) {
    strcat(retstr, "CABLE_DISCONNECTED, "); 
  } 
  if(state.cable & SCE_UDCD_STATUS_CABLE_CONNECTED) {
    strcat(retstr, "CABLE_CONNECTED, "); 
  } 
  if(state.state & SCE_UDCD_STATUS_DEACTIVATED) {
    strcat(retstr, "DEACTIVATED, "); 
  } 
  if(state.state & SCE_UDCD_STATUS_ACTIVATED) {
    strcat(retstr, "ACTIVATED, "); 
  }
  if(state.state & SCE_UDCD_STATUS_IS_CHARGING) {
    strcat(retstr, "IS_CHARGING, ");
  } 
  //if(state.use_usb_charging & SCE_UDCD_STATUS_USE_USB_CHARGING) {
  //  strcat(retstr, "USE_USB_CHARGING "); 
  //} 

  retstr[strlen(retstr)-2] = 0; // remove last ','

  return 0;
}

int getUdcdDevInfo(uint8_t *retstr,  uint32_t *rawval) {
  
  int ret;

  /*
  typedef struct {
    uint8_t info[64];
  } SceUdcdDeviceInfo;
 */

  SceUdcdDeviceInfo devInfo;
  ret = sceUdcdGetDeviceInfo(&devInfo);
  if( ret >= 0 ) 
    sprintf(retstr, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X..", devInfo.info[0], devInfo.info[1], devInfo.info[2], devInfo.info[3], devInfo.info[4], devInfo.info[5], devInfo.info[6], devInfo.info[7], devInfo.info[8], devInfo.info[9], devInfo.info[10], devInfo.info[11]);
  
  return 0;
}

int getBluetoothRevisionViaIDStorage(uint8_t *retstr, uint32_t *rawval) { // Robin
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103
  if( ret >= 0 ) {
    sprintf(retstr, "0x%X", *(uint32_t*)&buf[0xE0]); 
    *rawval = *(uint32_t*)&buf[0xE0];
  }

  return ret;
}

int getSysconRevisionViaIDStorage(uint8_t *retstr, uint32_t *rawval) { // Ernie
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103 ErnieHwInfo
  if( ret >= 0 ) {
    sprintf(retstr, "0x%X", *(uint32_t*)&buf[0x00]); // https://wiki.henkaku.xyz/vita/Ernie#Hardware_Versions
    *rawval = *(uint32_t*)&buf[0x00];
  } // -> returns hardware info basially with last part excluded eg: 0x805000 with hwinfo 00 80 50 38

  return ret;
}

int getPowerManVersionViaIDStorage(uint8_t *retstr, uint32_t *rawval) { // Elmo
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103 ElmoFWVer
  if( ret >= 0 ) {
    sprintf(retstr, "0x%X", *(uint16_t*)&buf[0x40]); 
    *rawval = *(uint16_t*)&buf[0x40];
  }

  return ret;
}

int getUsbChargeManVersionViaIDStorage(uint8_t *retstr, uint32_t *rawval) { // Cookie
  int ret = -1;
  static unsigned char buf[0x200];
  
  ret = vshIdStorageReadLeaf(0x103, buf); // https://wiki.henkaku.xyz/vita/IdStorage#0x103 CookieFWVer
  if( ret >= 0 ) {
    sprintf(retstr, "0x%X", *(uint16_t*)&buf[0x60]); 
    *rawval = *(uint16_t*)&buf[0x60];
  }

  return ret;
}

int getClockArm(uint8_t *retstr, uint32_t *rawval) { // CPU clock frequency in Mhz 
  int ret = scePowerGetArmClockFrequency();
  sprintf(retstr, "%u MHz", ret); 
  *rawval = ret;
  return 0;
}

int getClockGpu(uint8_t *retstr, uint32_t *rawval) { // GPU clock frequency in Mhz 
  int ret = scePowerGetGpuClockFrequency();
  sprintf(retstr, "%u MHz", ret); 
  *rawval = ret;
  return 0;
}

int getClockGpuXbar(uint8_t *retstr, uint32_t *rawval) { // GPU crossbar clock frequency in Mhz 
  int ret = scePowerGetGpuXbarClockFrequency();
  sprintf(retstr, "%u MHz", ret); 
  *rawval = ret;
  return 0;
}

int getClockBus(uint8_t *retstr, uint32_t *rawval) { // BUS clock frequency in Mhz 
  int ret = scePowerGetBusClockFrequency();
  sprintf(retstr, "%u MHz", ret); 
  *rawval = ret;
  return 0;
}

int getClockTimebase(uint8_t *retstr, uint32_t *rawval) { // timebase frequency (ticks per second) (how many times the hardware counter increments every single second)
  int ticks = 0;
  int ret = sceSysmoduleLoadModule(SCE_PERF_ARM_PMON_MAIN_PIPE); // crashes
  if( ret > 0) {
    ticks = scePerfGetTimebaseFrequency();
  }
  sprintf(retstr, "%u ticks per sec", ticks); 
  *rawval = ticks;
  return ret;
}

int getDateLastFirmwareUpdated(uint8_t *retstr) { 
  if( doesFileExist("ud0:PSP2UPDATE/updatemode_result.dat") == 0 ) {
    sprintf(retstr, "%s", getFileDateModify("ud0:PSP2UPDATE/updatemode_result.dat")); 
    return 0;
  } else if( doesFileExist("ux0:data/updatemode_result.dat") == 0 ) { // modoru redirects to "ux0:data/"
    sprintf(retstr, "%s", getFileDateModify("ux0:data/updatemode_result.dat")); 
    return 0;
  } else return -1;
}

int getNvsQaf(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/Ernie#NVS   (dip switches 240 and 241 should be checked as well)
  int ret = -1;

  static uint8_t nvs[0x10];
  memset(nvs, 0, 0x10);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x480, nvs, 0x10);
    if( ret < 0 )
      return ret;
  } else return -1111;

  *rawval = nvs[0x0]; 

  switch(nvs[0x0]) {
    case 0x00: sprintf(retstr, "Enabled"); break;
    case 0x01: sprintf(retstr, "Disabled"); break; // uninstalled! see https://wiki.henkaku.xyz/vita/SceSblSsMgr#sceSblQafMgrDeleteQafToken2
    case 0xFF: sprintf(retstr, "Not set"); break; // no token installed!
    default: sprintf(retstr, "unknown");
  }
  
  return ret;
}

int getNvsExtraUart(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/Ernie#NVS
  int ret = -1;

  static uint8_t nvs[0x10];
  memset(nvs, 0, 0x10);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x480, nvs, 0x10);
    if( ret < 0 )
      return ret;
  } else return -1111;

  *rawval = nvs[0x1]; // 0x481

  switch(nvs[0x1]) {
    case 0x00: sprintf(retstr, "Enabled"); break;
    case 0x01: sprintf(retstr, "Enabled when JIG"); break;
    case 0xFF: sprintf(retstr, "Disabled"); break;
    default: sprintf(retstr, "unknown");
  }
  
  return ret;
}

int getNvsSavemode(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/Ernie#NVS
  int ret = -1;

  static uint8_t nvs[0x10];
  memset(nvs, 0, 0x10);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x480, nvs, 0x10);
    if( ret < 0 )
      return ret;
  } else return -1111;

  *rawval = nvs[0x3];  // 0x483
  sprintf(retstr, nvs[0x3] == 0xFF ? "FALSE" : "TRUE"); // 1, 3, 5, 9, 0x11 - safemode type
  
  return ret;
}

int getNvsUpdateMode(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/Ernie#NVS (sceSblUsGetUpdateModeForUser)
  int ret = -1;

  static uint8_t nvs[0x10];
  memset(nvs, 0, 0x10);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x4A0, nvs, 0x10);
    if( ret < 0 )
      return ret;
  } else return -1111;

  *rawval = nvs[0x0]; // 0x4A0
  sprintf(retstr, nvs[0x0] == 0xFF ? "FALSE" : "TRUE"); 
  
  return ret;
}

int getNvsLanguage(uint8_t *retstr, uint32_t *rawval) { // https://wiki.henkaku.xyz/vita/Ernie#NVS 
  int ret = -1;

  const char *lang[] = { // 0 - 18
		"Japanese",
		"English (US)",
		"French",
		"Spanish",
		"German",
		"Italian",
		"Dutch",
		"Portuguese",
		"Russian",
		"Korean",
		"Chinese (Traditional)",
		"Chinese (Simplified)",
		"Finnish",
		"Swedish",
		"Danish",
		"Norwegian",
		"Polish",
		"Portuguese (Brazilian)",
		"English (UK)"
	};

  static uint8_t nvs[0x10];
  memset(nvs, 0, 0x10);
  
  if( kernmode ) { // helper plugins available
    ret = psvident_nvs_Get(0x4A0, nvs, 0x10);
    if( ret < 0 )
      return ret;
  } else return -1111;

  *rawval = nvs[0x4]; // 0x4A4
  
  if( nvs[0x4] >= 0 && nvs[0x4] <= 18 )
    sprintf(retstr, "%s", lang[nvs[0x4]]);
  else 
    sprintf(retstr, "error");

  return ret;
}

int getModuleInfo(int mode, int no, int val, uint8_t *retstr, uint32_t *rawval) { // 0 = kernel / 1 = user, no/128
  int ret = -1;

  SceKernelModuleInfo info; // https://docs.vitasdk.org/group__SceModulemgrKernel.html#structSceKernelModuleInfo
  memset(&info, 0, sizeof(SceKernelModuleInfo));
  
  if( kernmode ) { // helper plugins available
    ret = psvident_detect_plugins(mode, no, &info);
    if( ret < 0 )
      return ret;
  } else return -1111;

  switch( val ) {
    case 0: // filesz   https://docs.vitasdk.org/group__SceModulemgrKernel.html#structSceKernelSegmentInfo
      getSizeString(retstr, info.segments[0].filesz);
      *rawval = info.segments[0].filesz;
      break; 
      
    case 1: // modid (SceUID)
      sprintf(retstr, " ");
      *rawval = info.modid;
      break;

    case 2: // modver (major.minor)
      sprintf(retstr, "%d.%d", info.modver[1], info.modver[0]);
      *rawval = info.modver[1] * 0x10 + info.modver[0]; 
      break;
      
    case 3: // module_name
      sprintf(retstr, "%s", info.module_name);
      *rawval = 0;
      break;

    case 4: // path
      sprintf(retstr, "%s", info.path);
      *rawval = 0;
      break;

    case 5: // state
      *rawval = info.state; //
      /*SceKernelModuleState { 
        SCE_KERNEL_MODULE_STATE_READY = 0x2, 
        SCE_KERNEL_MODULE_STATE_STARTED = 0x6, 
        SCE_KERNEL_MODULE_STATE_ENDED = 0x9 
      }*/
      switch(info.state) {
        case 0x2: sprintf(retstr, "READY"); break;
        case 0x6: sprintf(retstr, "STARTED"); break;
        case 0x9: sprintf(retstr, "ENDED"); break;
        default: sprintf(retstr, "ERROR");
      }
      break;

    default:
      return -2;
  }
  
  return ret;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void printCapabilities(char *file) {
  logPrintf(file, "sceKernelGetModelForCDialog(): 0x%X", sceKernelGetModelForCDialog());
  
  logPrintf(file, "vshKernelCheckModelCapability(): %d", vshKernelCheckModelCapability());
  
  logPrintf(file, "vshSblAimgrIsCEX(): %d", vshSblAimgrIsCEX());
  logPrintf(file, "vshSblAimgrIsDEX(): %d", vshSblAimgrIsDEX());
  logPrintf(file, "vshSblAimgrIsTest(): %d", vshSblAimgrIsTest());
  logPrintf(file, "vshSblAimgrIsTool(): %d", vshSblAimgrIsTool());
  logPrintf(file, "vshSblAimgrIsDolce(): %d", vshSblAimgrIsDolce());
  logPrintf(file, "vshSblAimgrIsGenuineDolce(): %d", vshSblAimgrIsGenuineDolce());
  logPrintf(file, "vshSblAimgrIsVITA(): %d", vshSblAimgrIsVITA());
  logPrintf(file, "vshSblAimgrIsGenuineVITA(): %d", vshSblAimgrIsGenuineVITA());

  logPrintf(file, "vshSysconIsIduMode(): %d", vshSysconIsIduMode());
  logPrintf(file, "vshSysconIsShowMode(): %d", vshSysconIsShowMode());
  
  logPrintf(file, "vshSysconHasWWAN(): %d", vshSysconHasWWAN());
  logPrintf(file, "vshSysconIsMCEmuCapable(): %d", vshSysconIsMCEmuCapable());
  
  logPrintf(file, "vshMemoryCardGetCardInsertState(): 0x%X", vshMemoryCardGetCardInsertState());
  logPrintf(file, "vshRemovableMemoryGetCardInsertState(): 0x%X", vshRemovableMemoryGetCardInsertState());
  
}

void printQaf(char *file) { 
  logPrintf(file, "sceSblQafMgrIsAllowAllDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowAllDebugMenuDisplay());
  logPrintf(file, "sceSblQafMgrIsAllowForceUpdate(): 0x%08X", sceSblQafMgrIsAllowForceUpdate());
  logPrintf(file, "sceSblQafMgrIsAllowLimitedDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowLimitedDebugMenuDisplay());
  logPrintf(file, "sceSblQafMgrIsAllowMinimumDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowMinimumDebugMenuDisplay());
  logPrintf(file, "sceSblQafMgrIsAllowNonQAPup(): 0x%08X", sceSblQafMgrIsAllowNonQAPup());
  logPrintf(file, "sceSblQafMgrIsAllowNpFullTest(): 0x%08X", sceSblQafMgrIsAllowNpFullTest());
  //logPrintf(file, "sceSblQafMgrIsAllowNpTest(): 0x%08X", sceSblQafMgrIsAllowNpTest());
  logPrintf(file, "sceSblQafMgrIsAllowNpFullTest(): 0x%08X", sceSblQafMgrIsAllowNpFullTest());
  logPrintf(file, "sceSblQafMgrIsAllowRemoteSysmoduleLoad(): 0x%08X", sceSblQafMgrIsAllowRemoteSysmoduleLoad());
  logPrintf(file, "sceSblQafMgrIsAllowScreenShotAlways(): 0x%08X", sceSblQafMgrIsAllowScreenShotAlways());
  logPrintf(file, "sceSblQafManagerIsAllowKernelDebugForUser(): 0x%08X", sceSblQafManagerIsAllowKernelDebugForUser());
  
  /*logPrintf(file, "ksceSblQafMgrIsAllowControlIduAutoUpdate(): 0x%08X", ksceSblQafMgrIsAllowControlIduAutoUpdate());
  logPrintf(file, "ksceSblQafMgrIsAllowDecryptedBootConfigLoad(): 0x%08X", ksceSblQafMgrIsAllowDecryptedBootConfigLoad());
  logPrintf(file, "ksceSblQafMgrIsAllowDtcpIpReset(): 0x%08X", ksceSblQafMgrIsAllowDtcpIpReset());
  logPrintf(file, "ksceSblQafMgrIsAllowHost0Access(): 0x%08X", ksceSblQafMgrIsAllowHost0Access());
  logPrintf(file, "ksceSblQafMgrIsAllowKeepCoreFile(): 0x%08X", ksceSblQafMgrIsAllowKeepCoreFile());
  logPrintf(file, "ksceSblQafMgrIsAllowLoadMagicGate(): 0x%08X", ksceSblQafMgrIsAllowLoadMagicGate());
  logPrintf(file, "ksceSblQafMgrIsAllowMarlinTest(): 0x%08X", ksceSblQafMgrIsAllowMarlinTest());
  logPrintf(file, "ksceSblQafMgrIsAllowNearTest(): 0x%08X", ksceSblQafMgrIsAllowNearTest());
  logPrintf(file, "ksceSblQafMgrIsAllowPSPEmuShowQAInfo(): 0x%08X", ksceSblQafMgrIsAllowPSPEmuShowQAInfo());
  logPrintf(file, "ksceSblQafMgrIsAllowRemotePlayDebug(): 0x%08X", ksceSblQafMgrIsAllowRemotePlayDebug());
  logPrintf(file, "ksceSblQafMgrIsAllowSystemAppDebug(): 0x%08X", ksceSblQafMgrIsAllowSystemAppDebug());*/
}

