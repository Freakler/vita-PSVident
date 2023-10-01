#include <psp2/kernel/processmgr.h>
#include <psp2/vshbridge.h> 
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/sysmem.h> 
#include <psp2/io/fcntl.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/rtc.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/apputil.h>
#include <psp2/appmgr.h>
#include <psp2/system_param.h>
#include <psp2/motion_dev.h> 

#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

#include "main.h"
#include "psvident.h"
#include "utils/aes.h"
#include "utils/utils.h"
#include "utils/registry.h"
#include "plugin/common.h"

extern int censored;
extern int nicemode;
extern int kernmode;


char *getEmmcSize() {
	int ret = -1;
	static char string[16];
	unsigned int size = -1;
	
	if( kernmode ) { // helper plugins available
		ret = psvident_mbr_GetEmmcSize(&size);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "0x%X bytes", size);

	if( nicemode ) 
		//sprintf(string, "%d bytes", size);
		getSizeString(string, size);
		
	return string;
}

char *getCPVersion() {
	int ret = -1;
	static char string[16];
	uint8_t buf[0x20];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_DIPSwitches(buf);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	unsigned short version = *(unsigned short *)(buf + 0x4); 

	sprintf(string, "0x%X", version);

	if( nicemode ) 
		sprintf(string, "%d", version);
	
	return string;
}

char *getCPBoardId() {
	int ret = -1;
	static char string[16];
	uint8_t buf[0x20];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_DIPSwitches(buf);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	unsigned short id = *(unsigned short *)(buf + 0x6); 

	sprintf(string, "0x%X", id);

	if( nicemode ) 
		sprintf(string, "%d", id);
	
	return string;
}

char *getCPTimestamp() {
	int ret = -1;
	static char string[32];
	uint8_t buf[0x20];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_DIPSwitches(buf);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	unsigned int timestamp = *(unsigned int *)(buf + 0x0); 
	
	sprintf(string, "0x%X", timestamp);

	if( nicemode ) {
		SceDateTime time;
		sceRtcConvertTime_tToDateTime(timestamp, &time);
		sprintf(string, "%04d-%02d-%02d %02d:%02d", sceRtcGetYear(&time), sceRtcGetMonth(&time), sceRtcGetDay(&time), sceRtcGetHour(&time), sceRtcGetMinute(&time)); 
	}
	return string;
}

char *getSoCRevision() {
	int ret = -1;
	static char string[32];
	unsigned int soc = -1;
	
	if( kernmode ) { // helper plugins available
		ret = psvident_pervasive_GetSoCRevision(&soc);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "0x%08X", soc);
	
	if( nicemode ) 
		sprintf(string, "%X.%01X", (soc << 0xf) >> 0x13, soc & 0xf);
	
	return string;
}

char *getMotherboard() {
	int ret = -1;
	static char string[32];
	unsigned char hwinfo[4];
	
	ret = _vshSysconGetHardwareInfo(hwinfo);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	sprintf(string, "%02X %02X %02X", hwinfo[2], hwinfo[1], hwinfo[0]);
	
	if( nicemode ) {
		switch( hwinfo[2] ) {
			case 0x10: sprintf(string, "IRS-001"); break; // early CEMs
			case 0x31: sprintf(string, "IRT-001"); break; // DEM G/H
			case 0x40: sprintf(string, "IRS-002"); break; // PCH-1XXX
			case 0x41: sprintf(string, "IRT-002"); break; // DEM & PDEL
			case 0x60: sprintf(string, "IRS-1001"); break; // PCH-1XXX (new)
			case 0x70: sprintf(string, "DOL-1001"); break; // VTE-10XX 
			case 0x72: sprintf(string, "DOL-1002"); break; // VTE-10XX (new)
			case 0x80: sprintf(string, "USS-1001"); break; // PCH-20XX 
			case 0x82: sprintf(string, "USS-1002"); break; // PCH-20XX (new)
			default: sprintf(string, "unknown"); break;
		}
		
		if( (hwinfo[2] &= ~0xF0) != 0x01 ) { // additional capabilities (only non-devkits)
			//if( (hwinfo[0] &= ~0xF0) == 0x0 ) // check wifi
				//strcat(string, " (Wifi)");
			
			if( (hwinfo[0] &= ~0xF0) == 0x2 ) // check for 0xX2 where the 2 is for 3g
				strcat(string, " (3g)");
		}
	}
	
	return string;
}

char *getKibanId() {
	int ret = -1;
	static char string[64];
	uint8_t str[0x21];
	
	memset(str, 0, 0x21);
	
	if( kernmode ) { // helper plugins available
		ret = psvident_nvs_GetKibanId(str);
		if( ret < 0 )
			return error(ret, "ERROR");
	} else return "";
	
	if( censored )
		str[0x13] = str[0x14] = str[0x15] = 'X';

	if( nicemode ) 
		sprintf(string, "%c%c%c-%c%c%c%c%c%c%c-%c%c%c%c%c-%c%c%c%c%c%c%c", str[0x00], str[0x01], str[0x02], str[0x03], str[0x04], str[0x05], str[0x06], str[0x07], str[0x08], str[0x09], str[0x0A], str[0x0B], str[0x0C], str[0x0D], str[0x0E], str[0x0F], str[0x10], str[0x11], str[0x12], str[0x13], str[0x14], str[0x15]);
	else 
		sprintf(string, "%s", str);
	
	return string;
}	
	
char *getModelMore() {
	static char string[64];
	memset(string, 0, sizeof(string));
	
	unsigned char hwinfo[4];
	int ret = _vshSysconGetHardwareInfo(hwinfo);
	if( ret != 0 ) return "";
	//sprintf(string, "0x%02X%02X%02X%02X", hwinfo[3], hwinfo[2], hwinfo[1], hwinfo[0]);
	
		
	
	/// scene-name
	/*if( vshSblAimgrIsGenuineVITA() ) {
		if( vshSysconIsMCEmuCapable() )
			strcat(string, "Vita Slim,");
		else
			strcat(string, "Vita Fat,");	
	}
	if( vshSblAimgrIsGenuineDolce() ) {
		strcat(string, "PlayStation TV,");
	}
		
	
	/// capability
	if( !vshSysconHasWWAN() ) {
		strcat(string, "3G,");
	}*/


	/// color
	// via model name (alternative?)

	
	/// true IDU ?
	if( hwinfo[0] >> 4 == 0x8 )
		strcat(string, "In-store Demonstration Unit");
	
	
	
	//string[strlen(string)-1] = 0; // remove last ','
	
	return string; 
}

char *getModelName() {
	int ret = -1;
	static char string[64];
	static char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x115, buf); // protos are missing this leaf!
	if( ret != 0 ) {
		//if( nicemode )
		//	sprintf(string, "%s", getModelNameCustom()); // custom
		//else
			sprintf(string, "%s", getModelCode()); // eg: TG16201H
		return string;
	}
	
	snprintf(string, 0x11, "%s", buf); // write only x chars to string
	
	if( nicemode )
		model_string(string, string);

	return string;
}

char *getModelNameCustom() { // prototypes don't have modelname sometimes OR final name.. so my own version TODO
	int ret = -1;
	char CID[32];
	static char string[64];
	
	sprintf(string, "Unknown");
	
	ret = _vshSblAimgrGetConsoleId(CID);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	int subcode = CID[6] * 0x100 + CID[7];
	if( subcode <= 0xF ) { // its a prototype		
		
		/// todo here.. return CEM / DEM / TEFV via TG codes
		
		sprintf(string, "Prototype: %s", getModelCode()); // eg: TG16201H
		
	}
		
	return string; 
}

char *getModelCode() {
	int ret = -1;
	static char string[16];
	static char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x102, buf);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	snprintf(string, 9, "%s", &buf[0x68]); // eg: TG135F0H
	
	return string;
}

char *getSerial() {
	int ret = -1;
	static char string[0x40];
	char temp[0x40];
	char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x112, buf);
	if( ret != 0 ) { // either error reading leaf or its prototype that has no modelstring
		return error(ret, "ERROR");
	}			
	// Examples
	// 0000000000003TG135F0H60070050420 // CPV-2000KD1	03-TG.. 6007005
	// 00000000000000000274599200100771 // PDEL-1000	00-27.. 0100771
	// 00000000000000000TG9B000K8200250 // DEM-3000L	00-TG.. 8200250 
	// 00000000000032745212505300760C61 // PTEL-2002 	03-27.. 0530076
	// 00000000000000003TG16201H8120014 // CEM-3000NP1	03-TG.. 8120014 

	// we want it look like this: "03-27447091-2068012"
	
	memset(string, 0, sizeof(string));
	memset(temp, 0, sizeof(temp));
	
	int i = 1, j;	
	while( buf[i] == 0x30 )
		i+=2;

	for( j = 0; i < 0x40; i+=2, j++ ) {
		temp[j] = buf[i];
	} memset(buf, 0, sizeof(buf));
	
	// 32744709120680120BD0
	
	if( !nicemode ) { // return as is
		snprintf(string, 0x20, "%s", temp);
		if( censored ) 
			string[13] = string[14] = string[15] = 'X';
		return string;
	}
	
	if( strlen(temp) > 16 ) { // there are unwanted trailing numbers (ALWAYS 4!?)
		temp[strlen(temp)-4] = 0;
	} 
	
	// 3274470912068012
	
	for( i = 19, j = strlen(temp); i >= 0; i-- ) {
		if( i == 11 ) string[i] = '-'; // between serial and code
		else if( i == 2 ) string[i] = '-'; // "03-27.."
		else {
			if( j >= 0 ) { 
				string[i] = temp[j--];
			} else string[i] = '0'; // fill the rest in front
		}
	}
	
	// 03-27447091-2068012
	
	if( censored ) 
		string[16] = string[17] = string[18] = 'X';
	
	return string;
}

char *getConsoleID() {
	int i, ret = -1;
	char CID[32];
	static char string[64];
	static char helper[8];
	memset(string, 0, 64);
	
	ret = _vshSblAimgrGetConsoleId(CID);
	if( ret != 0 )
		return error(ret, "ERROR");
	

	for( i = 0; i < 16; i++ ) {
		if( i >= 10 && censored ) 
			sprintf(helper, "XX");
		else 
			sprintf(helper, "%02X", (unsigned char)CID[i]);
		
		if( i % 2 && nicemode ) 
			strcat(helper, " ");
		
		strcat(string, helper);
	}
	
	return string;
}

char *getHardwareInfo() {
	int ret = -1;
	static char string[16];
	unsigned char hwinfo[4];
	
	ret = _vshSysconGetHardwareInfo(hwinfo);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	sprintf(string, "0x%02X%02X%02X%02X", hwinfo[3], hwinfo[2], hwinfo[1], hwinfo[0]);
	
	if( nicemode ) 
		sprintf(string, "%02X %02X %02X %02X", hwinfo[3], hwinfo[2], hwinfo[1], hwinfo[0]);
	
	return string;
}

char *getMacAddressWifi() { // via sceNet
	int ret = -1;
	static char string[32];
	SceNetEtherAddr mac;
		
	ret = sceNetGetMacAddress(&mac, 0);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( censored )
		sprintf(string, "%02X:%02X:%02X:%02X:XX:XX", mac.data[0], mac.data[1], mac.data[2], mac.data[3]);
	else
		sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);

	return string;
}

char *getMacAddressLan() { // via IDStorage
	int ret = -1;
	static char string[64];
	static unsigned char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x119, buf); 
	if( ret != 0 && ret != 0x80230005) 
		return error(ret, "ERROR");
	if( ret == 0x80230005) // no leaf
		return "";
	if( buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF ) // on slims leaf exists but FFed
		return "";
	
	if( censored )
		sprintf(string, "%02X:%02X:%02X:%02X:XX:XX", buf[0], buf[1], buf[2], buf[3]);
	else 
		sprintf(string, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

	return string;
}

char *getMinFirmware() { // the lowest firmware that can be installed (via idstorage SMI header)
	int ret = -1;
	static char string[32];
	unsigned int version = -1;
		
	ret = _vshSblAimgrGetSMI(&version); // SMI = Service/System Manufacturing Information
	if( ret != 0 ) {
		/// try manually
		static char buf[0x200];
		ret = vshIdStorageReadLeaf(0x80, buf);
		if( ret != 0 )
			return error(ret, "ERROR");
		version = *(unsigned int *)(buf + 0x8);
	}
	sprintf(string, "0x%08X", version); // 0x0XYZ0000
	
	if( nicemode ) 
		firmware_string(string, version); // X.YZ
	
	return string;
}

char *getFactoryFirmware() { // the first firmware installed in the factory (though SMI string is NOT always correct :/)
	int ret = -1;
	static char string[128];
	static char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x100, buf);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	// 0001,0197,0096,0001,0001,3,1,1,1,03.150_20140430_01_E/cex,0.3.8.0_20131209_01_E,1	
	// 0001,0246,0108,0001,0001,3,1,1,1,03.60_20160404_01_E/cex,0.4.4.1_20150330_01_E,1
	
	if( buf[0x23] != '.' ) // until now all SMI Strings match this
		return warning(-1, "ERROR");
	
	sprintf(string, "%c.%c%c", buf[0x22], buf[0x24], buf[0x25]);
	
	if( !nicemode ) 
		sprintf(string, "%s", getSmiString1(9));
	
	return string;
}

char *getTarget() {
	int ret = -1;
	static char string[32];
	

	/// via ConsoleID
	char CID[32];
	ret = _vshSblAimgrGetConsoleId(CID);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	sprintf(string, "0x%03X", CID[4] * 0x100 + CID[5]);
	
	/// via vshSbl
	if( nicemode ) {
		if( vshSblAimgrIsCEX() )
			sprintf(string, "CEX (Consumer)"); // "Consumer Experience"
		else if( vshSblAimgrIsDEX() )
			sprintf(string, "DEX (Testing)"); // "Developer Experience"
		else if( vshSblAimgrIsTool() )
			sprintf(string, "TOOL (Development)");
		else if( vshSblAimgrIsTest() )
			sprintf(string, "TEST (Internal)");
		else 
			sprintf(string, "ERROR");
	}
	
	return string;
}

char *getRegionForDev() { // from Model String #ugly
	int ret = -1;
	static char buf[0x200];
	static char string[32];
	
	ret = vshIdStorageReadLeaf(0x115, buf); // protos are missing this leaf!
	if( ret != 0 ) {
		return error(ret, "ERROR");
		//return getRegionForDevProto(); // todo
	}
	char model = buf[1]; // D or T
	char version = buf[4]; // 1 or 2
	char code = buf[7]; // 0, 1, 2 ..
	
	if( model != 'D' && model != 'T' )
		return "ERROR";
	
	if( version != '1' && version != '2' )
		return "ERROR";
	
	if( model == 'D' ) { // Devkit
		switch( code ) {
			case '0': sprintf(string, "J1 (Japan)"); break;
			case '1': sprintf(string, "(EU & US)"); break;
			case '2': sprintf(string, "(Asia)"); break;
			default: sprintf(string, "ERROR");
		}
	}
	
	if( model == 'T' ) { // Testkit
		if( version == '1' ){ // Fat
			switch( code ) { 
				case '0': sprintf(string, "J1 (Japan)"); break; // HJ..
				case '1': sprintf(string, "UC2 (North America)"); break; // HU..
				case '2': sprintf(string, "Korea"); break; // HE..
				case '3': sprintf(string, "CEL (Europe)"); break; // HC..
				default: sprintf(string, "ERROR");
			}
		}
		if( version == '2' ){ // Slim
			switch( code ) {
				case '0': sprintf(string, "J1 (Japan)"); break;
				case '2': sprintf(string, "Korea"); break;
				case '4': sprintf(string, "US, Canada, EU, Turkey, UK"); break;
				default: sprintf(string, "ERROR");
			}
		}
	}
	
	return string;
}

char *getRegion() {
	int ret = -1;
	char CID[32];
	static char string[32];
		
	/// via ConsoleID
	ret = _vshSblAimgrGetConsoleId(CID);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	sprintf(string, "0x%03X", CID[4] * 0x100 + CID[5]);
	
	if( nicemode ) {
		switch( CID[4] * 0x100 + CID[5] ) {
			case 0x100:
				sprintf(string, "-");
				break;
				
			case 0x101: case 0x102: // Devkit / Testkit
				sprintf(string, "%s", getRegionForDev());
				break;
			
			case 0x103: sprintf(string, "J1 (Japan)"); break; // 0
			case 0x104: sprintf(string, "UC2 (North America)"); break;  // 1 	// UCS later for slim?!
			case 0x105: sprintf(string, "CEL (Europe)"); break; // 4
			case 0x106: sprintf(string, "KR2 (South Korea)"); break; // 5
			case 0x107: sprintf(string, "CEK (United Kingdom)"); break; // 3
			case 0x108: sprintf(string, "MX2 (Mexico)"); break;
			case 0x109: sprintf(string, "AU3 (Australia/New Zealand)"); break;
			case 0x10A: sprintf(string, "E12 (Asia)"); break; // 6
			case 0x10B: sprintf(string, "TW1 (Taiwan)"); break; //7
			case 0x10C: sprintf(string, "RU3 (Russia)"); break; //8
			case 0x10D: sprintf(string, "CN9 (China)"); break;
			case 0x10E: sprintf(string, "HK5 (Hong Kong)"); break;
			case 0x10F: sprintf(string, "RSV1 (reserved)"); break;
			case 0x110: sprintf(string, "RSV2 (reserved)"); break;
			case 0x111: sprintf(string, "RSV3 (reserved)"); break;
			default: sprintf(string, "Unknown?!");
		}	
	}
	
	return string;
}

char *getQATokenName() { 
	int ret = -1;
	static char string[64];
	
	memset(string, 0, sizeof(string));
	
	/// https://wiki.henkaku.xyz/vita/SceSblSsMgr#SceSblQafMgr
	ret = sceSblQafManagerGetQafNameForUser(string, sizeof(string)); // return 0 on success
	//if( ret != 0 )
		//return error(ret, "ERROR");
	
	//sprintf(string, "0x%08X - %s", ret, string);
	return string;
}

char *getDipSwitches(int print) {
	int ret = -1;
	static char string[64];
	uint8_t ds[0x20];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetDipSwitches(ds);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	
	if( print == 0 ) // 0x40 to 0x4F (CPInfo 0-95, ASLR? 96-127)
		sprintf(string, "%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X", ds[0x00], ds[0x01], ds[0x02], ds[0x03], ds[0x04], ds[0x05], ds[0x06], ds[0x07], ds[0x08], ds[0x09], ds[0x0A], ds[0x0B], ds[0x0C], ds[0x0D], ds[0x0E], ds[0x0F]);
	if( print == 1 ) // 0x50 to 0x5F (SDK 128-159, SHELL 160-191, DEBUG 192-223, SYSTEM 224-255)
		sprintf(string, "%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X", ds[0x10], ds[0x11], ds[0x12], ds[0x13], ds[0x14], ds[0x15], ds[0x16], ds[0x17], ds[0x18], ds[0x19], ds[0x1A], ds[0x1B], ds[0x1C], ds[0x1D], ds[0x1E], ds[0x1F]);
	
	return string;
}

char *getQAFlags() { 
	int ret = -1;
	static char string[64];
	uint8_t qaflags[0x10];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetQAFlags(qaflags);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", qaflags[0], qaflags[1], qaflags[2], qaflags[3], qaflags[4], qaflags[5], qaflags[6], qaflags[7], qaflags[8], qaflags[9], qaflags[0xA], qaflags[0xB], qaflags[0xC], qaflags[0xD], qaflags[0xE], qaflags[0xF]);
	
	return string;
}

char *getBootFlags() { 
	int ret = -1;
	static char string[64];
	uint8_t flags[0x10];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetBootFlags(flags);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", flags[0], flags[1], flags[2], flags[3], flags[4], flags[5], flags[6], flags[7], flags[8], flags[9], flags[0xA], flags[0xB], flags[0xC], flags[0xD], flags[0xE], flags[0xF]);
	
	return string;
}

char *getHardwareFlags() { 
	int ret = -1;
	static char string[64];
	uint8_t flags[0x10];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetHardwareFlags(flags);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", flags[0], flags[1], flags[2], flags[3], flags[4], flags[5], flags[6], flags[7], flags[8], flags[9], flags[0xA], flags[0xB], flags[0xC], flags[0xD], flags[0xE], flags[0xF]);
	
	return string;
}

char *getBootloaderVersion() { 
	int ret = -1;
	unsigned int bootldr = -1;
	static char string[32];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetBootloaderRevision(&bootldr);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "0x%X", bootldr);
	
	//if( nicemode ) 
		//sprintf(string, "%d", bootldr);
	
	return string;
}

char *getBaryonVersion() { 
	int ret = -1, baryonver = -1;
	static char string[32];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_syscon_GetBaryonVersion(&baryonver);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";

	sprintf(string, "0x%08X", baryonver);
	
	if( nicemode ) 
		baryon_string(string, baryonver);
	
	return string;
}

char *getBaryonTimestamp() { //eg: 32 30 31 32 31 31 30 38 31 37 30 34 | 201211081704 
	int ret = -1;
	unsigned long long baryonver = -1;
	static char string[32];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_syscon_GetBaryonTimestamp(&baryonver);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "0x%llX", baryonver);  // 0x2ED91D73E8 ???

	return string;
}

char *getCurrentFirmware() { // if mode print hex 
	int ret = -1;
	static char string[32];
	
	/// from index.dat
	// todo..
	// return string;
	
	
	/// from inactive os0
	SceUID fd = sceIoOpen("sdstor0:int-lp-act-os", SCE_O_RDONLY, 0777);
	if( fd >= 0 ) {
		char buf[0x100];
		unsigned int previous_fw = -1;
		sceIoLseek(fd, 0xB000, SCE_SEEK_SET); //0xB000 is first file (always?)
		sceIoRead(fd, buf, sizeof(buf));
		sceIoClose(fd);
		previous_fw = *(unsigned int *)(buf + 0x92); //at 0x92 in file is firmware
		sprintf(string, "0x%08X", previous_fw); 
		if( nicemode ) 
			firmware_string(string, previous_fw);
		return string;
	}
	
	/// from ModuleMgr (which might be spoofed, so last resort)
	SceKernelSystemSwVersion sw_ver_param;
	sw_ver_param.size = sizeof(SceKernelSystemSwVersion);
	ret = sceKernelGetSystemSwVersion(&sw_ver_param);
	if( ret != 0 ) {
		sprintf(string, "0x%08X", sw_ver_param.version);  // 0x0XYZ0000
		if( nicemode ) 
			sprintf(string, "%s", sw_ver_param.versionString); // X.YZ
		return string;
	}
	
	return "ERROR";
}

char *getPreviousFirmware() { // from inactive os0
	SceUID fd;
	static char string[16];
	char buf[0x100];
	unsigned int previous_fw = -1;
	
	fd = sceIoOpen("sdstor0:int-lp-ina-os", SCE_O_RDONLY, 0777);
	if( fd < 0 ) {
		return error(fd, "ERROR");
	}
	
	sceIoLseek(fd, 0xB000, SCE_SEEK_SET); //0xB000 is first file (always?)
	sceIoRead(fd, buf, sizeof(buf));
	sceIoClose(fd);
	
	previous_fw = *(unsigned int *)(buf + 0x92); //at 0x92 in file is firmware
	sprintf(string, "0x%08X", previous_fw); 
	if( nicemode ) 
		firmware_string(string, previous_fw);
	
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
	//  0     1    2    3   ...
	
	if( part < 0 ) // print full string
		snprintf(string, 128, "%s", buf);
	
	if( part >= 0 ) { // print part only
		int init_size = strlen(buf);
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
	//  0     1    2    3   ...
	
	if( part < 0 ) // print full string
		snprintf(string, 128, "%s", &buf[0x100]);
	
	if( part >= 0 ) { // print part only
		int init_size = strlen(&buf[0x100]);
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
	
	static char string[2048];
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
	}
	
	return string;
}

char *getBatteryVersionHwinfo() {
	static char string[8];
	unsigned int fwinfo, dfinfo;
	unsigned long long int hwinfo;
	if( kernmode ) { // helper plugins available
		int ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	sprintf(string, "0x%X", (unsigned int)hwinfo);
	return string;
}

char *getBatteryVersionFwinfo() {
	static char string[8];
	unsigned int fwinfo, dfinfo;
	unsigned long long int hwinfo;
	if( kernmode ) { // helper plugins available
		int ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";	
	sprintf(string, "0x%X", (unsigned int)fwinfo);
	return string;
}

char *getBatteryVersionDfinfo() {
	static char string[8];
	unsigned int fwinfo, dfinfo;
	unsigned long long int hwinfo;
	if( kernmode ) { // helper plugins available
		int ret = psvident_sysroot_GetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";	
	sprintf(string, "0x%X", (unsigned int)dfinfo);
	return string;
}

char *getBatteryStatus() {
    if( !scePowerIsBatteryCharging() ) 
		return "In use";
    else return "Charging";
}

char *getBatteryCycleCount() {
	static char string[8];
	sprintf(string, "%d", scePowerGetBatteryCycleCount());
	return string;
}

char *getBatteryTempInCelsius() {
	static char string[16];
	sprintf(string,"%0.0f",(float)scePowerGetBatteryTemp() / 100.0);
	return string;
}

char *getBatteryStateOfHealth() {
	static char string[8];
	sprintf(string,"%d", scePowerGetBatterySOH());
	return string;
}

char *getBatteryPercentage() {
	static char string[8];
	sprintf(string, "%d", scePowerGetBatteryLifePercent());
	return string;
}

char *getBatteryVoltage() {
	static char string[8];
	sprintf(string,"%0.2f",(float)scePowerGetBatteryVolt() / 1000.0);
	return string;
}

char *getBatteryCapacity() {
	static char string[10];
	sprintf(string,"%i", scePowerGetBatteryFullCapacity());
	return string;
}

char *getBatteryRemCapacity() {
	static char string[10];
	sprintf(string,"%i", scePowerGetBatteryRemainCapacity());
	return string;
}

char *getBatteryLifeTime() {
	static char string[64];
	sprintf(string, "%i minutes", scePowerGetBatteryLifeTime());
	if( nicemode ) {
		int ret = scePowerGetBatteryLifeTime();
		ret *= 60;
		int h = (ret/3600); 
		int m = (ret -(3600*h))/60;
		int s = (ret -(3600*h)-(m*60));
		sprintf(string, "%02d hours, %02d minutes", h, m);
	}
	return string;
}

char *getModeIDU() {
	static char string[8];
	sprintf(string, "%s", vshSysconIsIduMode() ? "TRUE" : "FALSE");
	return string;
}

char *getModeShow() {
	static char string[8];
	sprintf(string, "%s", vshSysconIsShowMode() ? "TRUE" : "FALSE");
	return string;
}

char *getModeDownloader() {
	static char string[8];
	sprintf(string, "%s", vshSysconIsDownLoaderMode() ? "TRUE" : "FALSE");
	return string;
}

char *getModeDevelopment() {
	static char string[8];
	sprintf(string, "%s", vshSblSsIsDevelopmentMode() ? "TRUE" : "FALSE");
	return string;
}

char *getModeManufacturing() { 
	int ret = -1;
	static char string[8];
	unsigned int manumode = -1;
	
	if( kernmode ) { // helper plugins available
		ret = psvident_sysroot_GetBootTypeIndicator(&manumode); // https://wiki.henkaku.xyz/vita/KBL_Param#Boot_type_indicator_1
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	sprintf(string, "%s", ((manumode & 0x4) != 0) ? "TRUE" : "FALSE"); // check 4th bit https://github.com/SKGleba/PSP2-batteryFixer/blob/master/kernel/main.c#L89
	return string;
}

char *getEnterButton() {	
	int enter_button = 0;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
	return enter_button ? "CROSS" : "CIRCLE";
}

char *getSystemUsername() {
	static SceChar8 username[SCE_SYSTEM_PARAM_USERNAME_MAXSIZE];
	sceAppUtilSystemParamGetString(SCE_SYSTEM_PARAM_ID_USERNAME, username, SCE_SYSTEM_PARAM_USERNAME_MAXSIZE);
	return (char *)username;
}

char *getRegistryNP() {
	int result = -1;
	static char string[64];
	int ret = getRegistryInteger(&result, "/CONFIG/NP", "enable_np");
	if( ret != 0 )
		return error(ret, "ERROR");
	sprintf(string,"%s", result ? "TRUE" : "FALSE");
	return string;
}

char *getRegistryAccountId() {
	static unsigned char str[16];
	static char string[64];
	int ret = getRegistryBinary(str, "/CONFIG/NP", "account_id");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( censored ) 
		sprintf(string,"XX XX XX XX XX XX XX XX");
	else
		convert2hex(str, string, 8);
	
	return string;
}

char *getRegistryLoginId() {
	static char string[64];
	int ret = getRegistryString(string, "/CONFIG/NP", "login_id");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( censored ) {
		for( ret = 0; ret < strlen(string); ret++ ) {
			if( string[ret] == '@' ) 
				break;
			string[ret] = 'X';
		}
	}
	return string;
}

char *getRegistryPassword() {
	static char string[64];
	int ret = getRegistryString(string, "/CONFIG/NP", "password");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( censored )
		for( ret = 0; ret < strlen(string); ret++ )
			string[ret] = 'X';
			
	return string;
}

char *getRegistryCountry() {
	static char string[64];
	int ret = getRegistryString(string, "/CONFIG/NP", "country");
	if( ret != 0 )
		return error(ret, "ERROR");
	return string;
}

char *getRegistryLanguage() {
	static char string[64];
	int ret = getRegistryString(string, "/CONFIG/NP", "lang");
	if( ret != 0 )
		return error(ret, "ERROR");
	return string;
}

char *getRegistryDateOfBirth() {
	int ret = -1;
	static char string[64];
	int yob = -1;
	int mob = -1;
	int dob = -1;
	
	ret = getRegistryInteger(&yob, "/CONFIG/NP", "yob");
	if( ret != 0 )
		return error(ret, "ERROR");
	ret = getRegistryInteger(&mob, "/CONFIG/NP", "mob");
	if( ret != 0 )
		return error(ret, "ERROR");
	ret = getRegistryInteger(&dob, "/CONFIG/NP", "dob");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( censored )
		sprintf(string, "XXXX-XX-XX");
	else
		sprintf(string, "%04d-%02d-%02d", yob, mob, dob);
	
	return string;
}

char *getActivationPeriod() { // from act.dat
	SceUID fd;
	static char string[64];
	char buf[0x100];
	unsigned int start_date = -1, end_date = -1;
	
	fd = sceIoOpen("tm0:activate/act.dat", SCE_O_RDONLY, 0777);
	if( fd < 0 )
		return error(fd, "ERROR");
	
	sceIoRead(fd, buf, sizeof(buf));
	sceIoClose(fd);
	
	start_date = *(unsigned int *)(buf + 0xC);
	end_date = *(unsigned int *)(buf + 0x10);
	
	sprintf(string, "0x%08X to 0x%08X", start_date, end_date);
	
	if( nicemode ) {
		SceDateTime start, end;
		sceRtcConvertTime_tToDateTime(start_date, &start);
		sceRtcConvertTime_tToDateTime(end_date, &end);
		sprintf(string, "%04d-%02d-%02d to %04d-%02d-%02d", sceRtcGetYear(&start), sceRtcGetMonth(&start), sceRtcGetDay(&start), sceRtcGetYear(&end), sceRtcGetMonth(&end), sceRtcGetDay(&end)); 
	}
	return string;
}

char *getActivationCount() { // from act.dat
	SceUID fd;
	static char string[16];
	char buf[0x100];
	unsigned int value = -1;
	
	fd = sceIoOpen("tm0:activate/act.dat", SCE_O_RDONLY, 0777);
	if( fd < 0 )
		return error(fd, "ERROR");
	
	sceIoRead(fd, buf, sizeof(buf));
	sceIoClose(fd);
	
	value = *(unsigned int *)(buf + 0x8); //at 0x8 in file
	
	sprintf(string, "%d", value); 
	return string;
}

char *getTouchpanelInfo(int option) { 
	//SceKernelTouchpanelDeviceInfo info; // 0.931-3.60
	SceKernelTouchpanelDeviceInfo2 info; // 0.990-3.60
	int ret = -1;
	static char string[16];
	
	if( kernmode ) { // helper plugins available
		ret = psvident_syscon_GetTouchpanelDeviceInfo2(&info);
		if( ret != 0 )
			return error(ret, "ERROR");
	} else return "";
	
	switch(option) {
		case 0: sprintf(string, "%04X", info.FrontVendorID); break;
		case 1: sprintf(string, "%04X", info.FrontFwVersion); break;
		case 2: sprintf(string, "%04X", info.FrontConfRev); break;
		case 3: sprintf(string, "%04X", info.BackVendorID); break;
		case 4: sprintf(string, "%04X", info.BackFwVersion); break;
		case 5: sprintf(string, "%04X", info.BackConfRev); break;
		default: sprintf(string, "ERROR");
	}		
	
	return string;
}

// https://wiki.henkaku.xyz/vita/SceMsif#sceMsifGetMsInfoForDriver
// https://docs.vitasdk.org/group__SceMsifKernel.html#ga1728cee8e5f0284024952d155182a4f1

/**
typedef struct SceMsId { // size is 8 bytes
    SceUInt8 unk_0x0; // maybe manufacture code. ex: 0x20.
    SceUInt16 manuf_year;
    SceUInt8 manuf_month;
    SceUInt8 manuf_day;
    SceUInt8 manuf_hour;
    SceUInt8 manuf_minute;
    SceUInt8 manuf_second;
} SceMsId;

typedef struct SceMsInfo { // size is 0x24 on FW 0.990-3.01, 0x40 on FW 3.10-3.60
    int ms_type; // Always 5 on seen PS Vita memory cards. ex (in software): 0, 1, 2, 3, 4, 5. Other values than 5 might be older revisions (Memory Stick, M2).
    SceBool is_read_only; // Set to 0 to allow RW. Anything else than 0 seems to disable write.
    SceUInt64 nbytes; // Total size in bytes.
    SceUInt64 nbytes2; // Same value as nbytes so for what purpose?
    SceUInt32 sector_size_low; // ex: 0x200 (default for PS Vita memory cards)
    SceUInt32 sector_size_hi; // Always set to 0 (hardcoded). Indeed PS Vita does not support big allocation sizes.
    SceUInt32 fs_offset;
    SceUInt32 unk_0x24; // ex: various
    SceUInt32 unk_0x28; // ex: 0
    SceUInt32 unk_0x2C; // ex: 1
    SceMsId id; // Stored in coredumps.
    void *SceMsif_subctx_addr; // Should not be accessible to usermode as it points to SceMsif kernel module data segment (offset 0x840 on FW 3.60)...
    SceUInt32 reserved; // Totally unused. This field has been forgotten in some code so there exists vulnerabilities
} SceMsInfo;
**/

char *getMemCardDate() { // Samples: https://pastebin.com/raw/VZ1MSzs1 thx cat
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	// 20 07 DC 01 14 .... -> 07DC = 2012
	sprintf(string, "%04d-%02d-%02d %02d:%02d:%02d", info.id[1] * 0x100 + info.id[2], info.id[3], info.id[4], info.id[5], info.id[6], info.id[7]); 
		
	return string;
}

char *getMemCardSize() { 
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	sprintf(string, "%llu bytes", info.nbytes); 
	
	if( nicemode )
		getSizeString(string, info.nbytes);
	
	return string;
}

char *getMemCardType() { 
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	sprintf(string, "%d", info.unk_0x00); 
	return string;
}

char *getMemCardFsoffset() { 
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	sprintf(string, "0x%X", info.fs_offset); 
	return string;
}

char *getMemCardSectorSize() { 
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	sprintf(string, "0x%X", info.sector_size); 
	
	if( nicemode )
		sprintf(string, "%d", info.sector_size); 
	
	return string;
}

char *getMemCardReadonly() { 
	SceMsInfo info;
	int ret = -1;
	static char string[32];

	ret = vshMsifGetMsInfo(&info);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	sprintf(string, "%d", info.unk_0x04); 
	
	if( nicemode )
		sprintf(string, "%s", info.unk_0x04 ? "TRUE" : "FALSE"); 
	
	return string;
}

char *getWifiSSID(int profile) { 
	char buffer[64];
	static char string[64];
	sprintf(buffer, "/CONFIG/NET/%02d/WIFI/", profile);
	
	int ret = getRegistryString(string, buffer, "ssid");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	return string;
}

char *getWifiPassword(int profile) { 
	char buffer[64];
	static char string[64];
	sprintf(buffer, "/CONFIG/NET/%02d/WIFI/", profile);
	int ret, type = -1;
	
	ret = getRegistryInteger(&type, buffer, "wifi_security");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	
	ret = getRegistryString(string, buffer, type == 1 ? "wep_key" : "wpa_key");
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( string[0] != 0 && censored ) 
		return "-";
	
	return string;
}

char *getAutoAvls() {
	int ret = -1;
	static char string[16];
	static char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x118, buf);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	if( buf[0] == 0x01 )
		return "TRUE";
	
	if( buf[0] == 0x00 )
		return "FALSE";
	
	return warning(-1, "ERROR");
}

char *getRefurbished() {
	if( getSerial()[1] == '8' ) {
		return "TRUE";
	}
	return "FALSE";
}

char *getTrueIdu() {
	unsigned char hwinfo[4];
	int ret = _vshSysconGetHardwareInfo(hwinfo);
	if( ret != 0 ) 
		return error(ret, "ERROR");
	
	if( hwinfo[0] >> 4 == 0x8 ) {
		return "TRUE";
	}
	return "FALSE";
}

char *getTest() {
	int ret = -1;
	static char string[64];
	static unsigned char buf[0x200];
	
	ret = vshIdStorageReadLeaf(0x119, buf); 
	if( ret != 0 ) {
		return error(ret, "ERROR");
	}
	
	if( nicemode )
		sprintf(string, "%02X %02X %02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]); // print HEX
	else 
		snprintf(string, 0x11, "%s", buf); // print String

	return string;
}


/*typedef struct SceMotionDevDeviceInfo { // TODO
    SceUInt8 a;
   // SceUInt32 b;
    //SceUInt32 c;
} SceMotionDevDeviceInfo;

char *getMotionInfo() { 
	int ret = -1;
	static char string[32];
	SceMotionDevDeviceInfo info;
		
	ret = sceMotionDevGetDeviceInfo(&info);
	if( ret != 0 )
		return error(ret, "ERROR");
	
	//sprintf(string, "%X", info.a);
	
	return string;
}*/

void printCapabilities(char *file) {
	logPrintf(file, "sceKernelGetModelForCDialog(): 0x%08X", sceKernelGetModelForCDialog());
	
	logPrintf(file, "vshKernelCheckModelCapability(): %d", vshKernelCheckModelCapability());
	
	logPrintf(file, "vshSblAimgrIsCEX(): %d", vshSblAimgrIsCEX());
	logPrintf(file, "vshSblAimgrIsDEX(): %d", vshSblAimgrIsDEX());
	logPrintf(file, "vshSblAimgrIsTest(): %d", vshSblAimgrIsTest());
	logPrintf(file, "vshSblAimgrIsTool(): %d", vshSblAimgrIsTool());
	logPrintf(file, "vshSblAimgrIsDolce(): %d", vshSblAimgrIsDolce());
	logPrintf(file, "vshSblAimgrIsGenuineDolce(): %d", vshSblAimgrIsGenuineDolce());
	logPrintf(file, "vshSblAimgrIsVITA(): %d", vshSblAimgrIsVITA());
	logPrintf(file, "vshSblAimgrIsGenuineVITA(): %d", vshSblAimgrIsGenuineVITA());
	
	logPrintf(file, "vshSysconHasWWAN(): %d", vshSysconHasWWAN());
	logPrintf(file, "vshSysconIsMCEmuCapable(): %d", vshSysconIsMCEmuCapable());
	
	logPrintf(file, "vshMemoryCardGetCardInsertState(): %d", vshMemoryCardGetCardInsertState());
	logPrintf(file, "vshRemovableMemoryGetCardInsertState(): %d", vshRemovableMemoryGetCardInsertState());
	
}
void printQaf(char *file) {
	logPrintf(file, "sceSblQafMgrIsAllowAllDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowAllDebugMenuDisplay());
	logPrintf(file, "sceSblQafMgrIsAllowForceUpdate(): 0x%08X", sceSblQafMgrIsAllowForceUpdate());
	logPrintf(file, "sceSblQafMgrIsAllowLimitedDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowLimitedDebugMenuDisplay());
	logPrintf(file, "sceSblQafMgrIsAllowMinimumDebugMenuDisplay(): 0x%08X", sceSblQafMgrIsAllowMinimumDebugMenuDisplay());
	logPrintf(file, "sceSblQafMgrIsAllowNonQAPup(): 0x%08X", sceSblQafMgrIsAllowNonQAPup());
	logPrintf(file, "sceSblQafMgrIsAllowNpFullTest(): 0x%08X", sceSblQafMgrIsAllowNpFullTest());
	//logPrintf(file, "sceSblQafMgrIsAllowNpTest(): 0x%08X", sceSblQafMgrIsAllowNpTest());
	logPrintf(file, "sceSblQafMgrIsAllowRemoteSysmoduleLoad(): 0x%08X", sceSblQafMgrIsAllowRemoteSysmoduleLoad());
	logPrintf(file, "sceSblQafMgrIsAllowScreenShotAlways(): 0x%08X", sceSblQafMgrIsAllowScreenShotAlways());
	
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