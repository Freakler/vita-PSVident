#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h> 
#include <psp2/vshbridge.h> 
#include <psp2/io/fcntl.h>
#include <psp2/ctrl.h>
#include <psp2/sysmodule.h> 

#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

#include <taihen.h>

#include "main.h"
#include "psvident.h"
#include "utils/utils.h"
#include "print/pspdebug.h"


#define VERSION "v0.40"
#define APP_PATH "ux0:app/PSVIDENT0/"


int censored = 0;
int nicemode = 1;
int kernmode = -1;

void hardware() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Hardware");
			
		
	/// Model
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Model:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getModelName());
	//y += 1;
	//psvDebugScreenSetXY(x2, y);
	//psvDebugScreenPrintf("%s", getModelMore()); // "Vita Fat/Slim/TV" 3g/Wifi color .. todo
	y += 2;
	
	
	/// Region
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Region:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegion());
	y += 2;
	
	/// Target
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Target:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTarget());
	y += 2;
	
	/// Motherboard
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Motherboard:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMotherboard());
	y += 2;
	
	/// DEV ///////////////////////////////////////////	
	
	if( vshSblAimgrIsTool() ) { // Devkit
	
		// Neighborhood outputs like this:
		// cp info.   : bid.4 ver.1301
		// (Board ID + version)
		
		/// CP Board ID
		psvDebugScreenSetXY(x1, y);
		psvDebugScreenSetTextColor(YELLOW);
		psvDebugScreenPrintf("CP Board:");
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("bid. %s ver. %s", getCPBoardId(), getCPVersion());
		y += 2;
	}
	y += 1;
	
	
	/// Code + Serial
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Serial:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getSerial());
	y += 2;
	
	/// MAC Address
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("MAC Address:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMacAddress());
	y += 2;
	
	/// Console ID
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("ConsoleID:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getConsoleID());
	y += 2;
	
	/// Hardware Info
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Hardware Info:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getHardwareInfo());
	y += 2;
		

	/// SoC rev: https://wiki.henkaku.xyz/vita/Pervasive#.22SoC_revision.22
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("SoC revision:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getSoCRevision());
	y += 2;
	
	
	/// eMMC size
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("eMMC size:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getEmmcSize());
	y += 3;
	
	
	
	/// Hardware Flags
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Hardware Flags:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getHardwareFlags());
	y += 3;




	
		
}
	
void software() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Software & Flags");
	
	
	/// Mode - IDU
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("IDU Mode:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getModeIDU());
	y += 2;	
	
	/// Mode - Show
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Show Mode:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getModeShow());
	y += 2;	
	
	/// Mode - Download
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Downloader Mode:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getModeDownloader());
	y += 2;	
	
	/// Mode - Manu
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Manufacturing Mode:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getModeManufacturing());
	y += 2;	
	
	if( vshSblAimgrIsTool() ) { // Devkit
	
		/// Mode - Development
		psvDebugScreenSetXY(x1, y);
		psvDebugScreenSetTextColor(YELLOW);
		psvDebugScreenPrintf("Development Mode:");
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("%s", getModeDevelopment());
		y += 2;
				
	}
	y += 1;	

	
	/// Bootloader rev:
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Bootloader rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBootloaderVersion());
	y += 3;
	
	

	/// Baryon - GetBaryonVersion https://wiki.henkaku.xyz/vita/Ernie#CMD_0x0001_-_GetBaryonVersion current installed Ernie firmware (ex: 0x0100060B -> 1.0.6.11) 
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Baryon Version:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBaryonVersion());
	//psvDebugScreenPrintf(" (%s)", getBaryonTimestamp()); // todo
	y += 3;	



	
	
	/// QA Flags
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("QA Flags:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getQAFlags());
	y += 1;	
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getQATokenName()); // only printed if found
	y += 2;	
	
	
	

	/// Boot Flags
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Boot Flags:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBootFlags());
	y += 3;	



	/// Switches
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("DIP Switches:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getDipSwitches(0));
	y += 1;	
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getDipSwitches(1));
	y += 3;	



	/// NVS
	/*psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("NVS:");
	psvDebugScreenSetXY(x2, y);
	//psvDebugScreenPrintf("%s", getBootFlags());
	y += 3;	*/


}

void firmware() {
	int i, x1, x2, x3, y;

	x1 = 3, x2 = 24, x3 = 31, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Firmware");
	
	
	/// Current Firmware
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Current Firmware:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getCurrentFirmware());
	y += 2;
	
	/// Previous Firmware
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Previous Firmware:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getPreviousFirmware());
	y += 2;
	
	/// Factory Firmware
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Factory Firmware:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getFactoryFirmware());
	y += 2;
	
	/// Min. Firmware
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Minimum Firmware:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMinFirmware());
	y += 3;
		
	
		
	/// index.dat
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	char buf[2048];
	memset(&buf[0], 0, sizeof(buf));
	sprintf(buf, "%s", getVersionTxtString());
	while( strcspn(buf, "\r\n") != strlen(buf) ) // In case there is no match, the end of string (position of null character ) is returned.
		buf[strcspn(buf, "\r\n")] = ' '; // replace
	int counter = 0;
	int init_size = strlen(buf);
	char delim[] = " ";
	char *ptr = strtok(buf, delim);
	while( ptr != NULL ) {
		if( counter % 2 == 0 ) {
			psvDebugScreenSetXY(x1, y);
		} else {
			psvDebugScreenSetXY(x3, y);
			y += 1;	
		}			
		psvDebugScreenPrintf("%s", ptr);
		ptr = strtok(NULL, delim);
		counter++;
	}
	y += 3;
	
		
	/// SMI String 1
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("SMI String:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getSmiString1(nicemode ? 9 : -1)); // 9 is factory firmware version, date & target
 	y += 2;
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", nicemode ? getSmiString1(10) : ""); // 10 is some other version
 	y += 3;
	
	/// SMI String 2
	/*psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("SMI String 2:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getSmiString2(-1)); 
 	y += 2;	*/
		
}

void battery() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Battery");
			
		
	/// Status
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Percentage:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s%%", getBatteryPercentage());
	y += 2;
		
	/// Capacity
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Capacity:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s of %s mAh", getBatteryRemCapacity(), getBatteryCapacity());
	y += 2;
	
	/// Status
	/*psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Status:");
	psvDebugScreenSetXY(x2, y);
	//psvDebugScreenPrintf("%s", getBatteryStatus());
	y += 2;*/
	
	/// Lifetime
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Lifetime:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBatteryLifeTime());
	y += 2;
	
	/// Temperature
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Temperature:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s Celsius", getBatteryTempInCelsius());
	y += 2;
	
	/// Voltage
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Voltage:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s mV", getBatteryVoltage());
	y += 2;
	
	/// Cycle Count
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Cycle Count:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBatteryCycleCount());
	y += 2;
	
	/// State of Health
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("State of Health:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s%%", getBatteryStateOfHealth());
	y += 2;
	
	
	y += 2;
		
	/// HWinfo
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("HWinfo:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBatteryVersionHwinfo());
	y += 2;
	
	/// FWinfo
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("FWinfo:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBatteryVersionFwinfo());
	y += 2;
	
	/// DFinfo
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("DFinfo:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getBatteryVersionDfinfo());
	y += 2;
		
	
}

void partitions() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Partitions");
			
		
	/// Percentage
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("ux0:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("xyz MB");
	//psvDebugScreenPrintf("%s", getModelName());
	y += 2;
	
	
	//https://github.com/SKGleba/PSP2Info/blob/master/main.c#L189
	//https://wiki.henkaku.xyz/vita/SceIofilemgr#Mount_Points
	
	
}

void miscellaneous() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Miscellaneous");
			
		
	/// Enso
	/* psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Enso:");
	psvDebugScreenSetXY(x2, y);
	//psvDebugScreenPrintf("%s", getSys());
	y += 2;	*/
	
	/// Enter Button
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Enter Button:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getEnterButton());
	y += 2;	
	
	/// Auto AVLS
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Auto AVLS:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getAutoAvls());
	y += 2;	
	
	
}

void memorycard() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> MemoryCard");
			
	//if( isMemoryCard ) {	
	
	/// MemCard - Type
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Type:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardType());
	y += 2;
	
	/// MemCard - Size
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Size:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardSize());
	y += 2;
	
	/// MemCard - Date
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Manufacturing Date:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardDate());
	y += 3;


	/// MemCard - Readonly
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Read Only:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardReadonly());
	y += 2;

	/// MemCard - Sector Size
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Sector Size:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardSectorSize());
	y += 2;

	/// MemCard - fs_offset
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("fs_offset:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMemCardFsoffset());
	y += 2;



}

void touchpanel() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Touchpads");
			
		
	/// Touchpanel - Front /Touch Vendor ID
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Front Vendor ID:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(0));
	y += 2;
	
	/// Touchpanel - Front /Touch Firmware Revision
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Front Firmware Rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(1));
	y += 2;
	
	/// Touchpanel - Front /Touch Configuration Revision
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Front Config Rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(2));
	y += 2;
	
	
	y += 1;
	
	
	/// Touchpanel - Rear /Touch Vendor ID
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Rear VendorID:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(3));
	y += 2;
	
	/// Touchpanel - Rear /Touch Firmware Revision
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Rear Firmware Rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(4));
	y += 2;
	
	/// Touchpanel - Rear /Touch Configuration Revision
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Rear Config Rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getTouchpanelInfo(5));
	y += 2;
	
	
	y += 1;
	
	
	/// Motion
	/*psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Motion Sensor FW Rev:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getMotionInfo());
	y += 2;*/
	
}

void activation() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> PSN & Activation");
			

	/// Activated
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("NP Activated:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryNP());
	y += 3;	
	
	/// Username
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Username:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getSystemUsername());
	y += 3;	
	
	

	/// Account ID
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Account ID:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryAccountId());
	y += 2;	
	
	/// Login ID
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Login ID:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryLoginId());
	y += 2;	
	
	/// Password
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Password:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryPassword());
	y += 2;	
		
	
	/// Date of Birth
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Date of Birth:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryDateOfBirth());
	y += 2;	
	
	/// Country Code
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Country Code:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryCountry());
	y += 2;	
	
	/// Language
	psvDebugScreenSetXY(x1, y);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("Language:");
	psvDebugScreenSetXY(x2, y);
	psvDebugScreenPrintf("%s", getRegistryLanguage());
	y += 2;	
	
	
	
	
	
	
	/// DEV ///////////////////////////////////////////	
	
	if( vshSblAimgrIsDEX() || vshSblAimgrIsTool() ) { // Development Hardware
		y += 1;	
	
		/// Activation Period
		psvDebugScreenSetXY(x1, y);
		psvDebugScreenSetTextColor(YELLOW);
		psvDebugScreenPrintf("Activation Period:");
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("%s", getActivationPeriod());
		y += 2;
		
		/// Activation Count
		psvDebugScreenSetXY(x1, y);
		psvDebugScreenSetTextColor(YELLOW);
		psvDebugScreenPrintf("Activation Count:");
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("%s", getActivationCount());
		y += 2;
		
		
		if( vshSblAimgrIsTool() ) { // DEVKIT
		
			/// CP Timestamp
			psvDebugScreenSetXY(x1, y);
			psvDebugScreenSetTextColor(YELLOW);
			psvDebugScreenPrintf("CP Timestamp:");
			psvDebugScreenSetXY(x2, y);
			psvDebugScreenPrintf("%s", getCPTimestamp());
			y += 2;
			
		}
		
	}
	
}

void wifiprofiles() {
	int i, x1, x2, y;

	x1 = 3, x2 = 24, y = 6;
	
	psvDebugScreenSetXY(2, 3);
	psvDebugScreenSetTextColor(WHITE);
	psvDebugScreenPrintf("> Wifi Profiles");
	
	
	for( i = 1; i < 10; i++) {		
		psvDebugScreenSetXY(x1, y);
		psvDebugScreenSetTextColor(WHITE);
		//psvDebugScreenPrintf("Profile %d", i);
		psvDebugScreenPrintf("%d", i);
		//y += 1;	
		psvDebugScreenSetXY(x1+2, y);
		psvDebugScreenPrintf("SSID:");
		//psvDebugScreenPrintf("SSID #%d:", i);
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("%s", getWifiSSID(i));
		y += 1;	
		psvDebugScreenSetXY(x1+2, y);
		psvDebugScreenPrintf("Password:");
		psvDebugScreenSetXY(x2, y);
		psvDebugScreenPrintf("%s", getWifiPassword(i));
		y += 2;	
	}	
	
	/// no space for more than 6 (7)
	
}


////////////////////	////////////////////////////	/////////////////////////	///////////////////////	////////////////////	////////////////////////////

char *warning(int code, char *msg) {
	psvDebugScreenSetTextColor(YELLOW);
	
	static char string[16];
	if( !nicemode ) {
		sprintf(string, "0x%08X", code);
		return string;
	}
	
	return msg;
}

char *error(int code, char *msg) {
	psvDebugScreenSetTextColor(RED);
	
	static char string[16];
	if( !nicemode ) {
		sprintf(string, "0x%08X", code);
		return string;
	}
	
	switch(code) { // only a small selection of error codes
		case 0x80010002: return "SCE_ERROR_ERRNO_ENOENT";
		case 0x8002D013: return "SCE_KERNEL_ERROR_MODULEMGR_OLD_LIB";
		case 0x8002D003: return "SCE_KERNEL_ERROR_MODULEMGR_NO_LIB";
		case 0x80024501: return "SCE_KERNEL_ERROR_INVALID_UID";
		case 0x80024302: return "SCE_KERNEL_ERROR_NO_FREE_PHYSICAL_PAGE";
		case 0x80230005: return "NO LEAF"; // C4-3085-6
		default: return msg;
	}
}

void savereport(char *file) {
	
	int i;
	sceIoRemove(file);
	
	logPrintf(file, "PSVident %s %s %s", VERSION, censored ? "-censored" : "", !nicemode ? "-raw" : "");
	logPrintf(file, "--------------");
	logPrintf(file, "");
	
	
	/// Hardware //////////////////////////////
	logPrintf(file, "> Hardware");
	logPrintf(file, "Model: %s", getModelName());
	logPrintf(file, "Region: %s", getRegion());
	logPrintf(file, "Target: %s", getTarget());
	logPrintf(file, "Motherboard: %s", getMotherboard());
	if( vshSblAimgrIsTool() ) { // Devkit
	logPrintf(file, "CP Board: bid. %s ver. %s", getCPBoardId(), getCPVersion());
	}
	logPrintf(file, "Serial: %s", getSerial());
	logPrintf(file, "MAC Address: %s", getMacAddress());
	logPrintf(file, "ConsoleID: %s", getConsoleID());
	logPrintf(file, "Hardware Info: %s", getHardwareInfo());
	logPrintf(file, "SoC revision: %s", getSoCRevision());
	logPrintf(file, "eMMC size: %s", getEmmcSize());
	logPrintf(file, "Hardware Flags: %s", getHardwareFlags());
	logPrintf(file, "\n");
	
	
	
	/// Firmware //////////////////////////////
	logPrintf(file, "> Firmware");
	logPrintf(file, "Current Firmware: %s", getCurrentFirmware());
	logPrintf(file, "Previous Firmware: %s", getPreviousFirmware());
	logPrintf(file, "Factory Firmware: %s", getFactoryFirmware());
	logPrintf(file, "Minimum Firmware: %s", getMinFirmware());
	logPrintf(file, "%s", getVersionTxtString());
	logPrintf(file, "SMI String 1: %s", getSmiString1(-1)); // print full string
	logPrintf(file, "SMI String 2: %s", getSmiString2(-1)); // print full string
	logPrintf(file, "\n");



	/// Software //////////////////////////////
	logPrintf(file, "> Software & Flags");
	logPrintf(file, "IDU Mode: %s", getModeIDU());
	logPrintf(file, "Show Mode: %s", getModeShow());
	logPrintf(file, "Downloader Mode: %s", getModeDownloader());
	logPrintf(file, "Manufacturing Mode: %s", getModeManufacturing());
	if( vshSblAimgrIsTool() ) { // Devkit
	logPrintf(file, "Development Mode: %s", getModeDevelopment());
	}
	logPrintf(file, "Bootloader rev: %s", getBootloaderVersion());
	logPrintf(file, "Baryon Version: %s", getBaryonVersion());
	logPrintf(file, "QA Flags: %s", getQAFlags());
	logPrintf(file, "QA Token Name: %s", getQATokenName());
	logPrintf(file, "Boot Flags: %s", getBootFlags());
	logPrintf(file, "DIP Switches: %s %s", getDipSwitches(0), getDipSwitches(1));
	logPrintf(file, "\n");
	
	
	
	/// Battery //////////////////////////////
	logPrintf(file, "> Battery");
	logPrintf(file, "Percentage: %s%%", getBatteryPercentage());
	logPrintf(file, "Capacity: %s of %s mAh", getBatteryRemCapacity(), getBatteryCapacity());
	logPrintf(file, "Lifetime: %s", getBatteryLifeTime());
	logPrintf(file, "Temperature: %s Celsius", getBatteryTempInCelsius());
	logPrintf(file, "Voltage: %s mV", getBatteryVoltage());
	logPrintf(file, "Cycle Count: %s", getBatteryCycleCount());
	logPrintf(file, "State of Health: %s%%", getBatteryStateOfHealth());
	logPrintf(file, "HWinfo: %s", getBatteryVersionHwinfo());
	logPrintf(file, "FWinfo: %s", getBatteryVersionFwinfo());
	logPrintf(file, "DFinfo: %s", getBatteryVersionDfinfo());
	logPrintf(file, "\n");
	
	
	
	/// Miscellaneous //////////////////////////////
	logPrintf(file, "> Miscellaneous");
	logPrintf(file, "Enter Button: %s", getEnterButton());
	logPrintf(file, "Auto AVLS: %s", getAutoAvls());
	logPrintf(file, "\n");
	
	
	
	/// PSN & Activation //////////////////////////////
	logPrintf(file, "> PSN & Activation");
	logPrintf(file, "NP Activated: %s", getRegistryNP());
	logPrintf(file, "Username: %s", getSystemUsername());
	logPrintf(file, "Account ID: %s", getRegistryAccountId());
	logPrintf(file, "Login ID: %s", getRegistryLoginId());
	logPrintf(file, "Password: %s", getRegistryPassword());
	logPrintf(file, "Date of Birth: %s", getRegistryDateOfBirth());
	logPrintf(file, "Country Code: %s", getRegistryCountry());
	logPrintf(file, "Language: %s", getRegistryLanguage());
	if( vshSblAimgrIsDEX() || vshSblAimgrIsTool() ) { // Development Hardware
	logPrintf(file, "Activation Period: %s", getActivationPeriod());
	logPrintf(file, "Activation Count: %s", getActivationCount());
	}
	if( vshSblAimgrIsTool() ) { // DEVKIT
	logPrintf(file, "CP Timestamp: %s", getCPTimestamp());
	}
	logPrintf(file, "\n");
	
	
	
	/// Memory Card //////////////////////////////
	logPrintf(file, "> MemoryCard");
	logPrintf(file, "Type: %s", getMemCardType());
	logPrintf(file, "Size: %s", getMemCardSize());
	logPrintf(file, "Date: %s", getMemCardDate());
	logPrintf(file, "Read Only: %s", getMemCardReadonly());
	logPrintf(file, "Sector Size: %s", getMemCardSectorSize());
	logPrintf(file, "fs_offset: %s", getMemCardFsoffset());
	logPrintf(file, "\n");
	
	
	
	/// Touchpads //////////////////////////////
	logPrintf(file, "> Touchpads");
	logPrintf(file, "Front Vendor ID: %s", getTouchpanelInfo(0));
	logPrintf(file, "Front Firmware Rev: %s", getTouchpanelInfo(1));
	logPrintf(file, "Front Config Rev: %s", getTouchpanelInfo(2));
	logPrintf(file, "Rear VendorID: %s", getTouchpanelInfo(3));
	logPrintf(file, "Rear Firmware Rev: %s", getTouchpanelInfo(4));
	logPrintf(file, "Rear Config Rev: %s", getTouchpanelInfo(5));
	logPrintf(file, "\n");
	
	
	
	/// Wifi Profiles //////////////////////////////
	logPrintf(file, "> Wifi Profiles");
	for( i = 1; i < 0x1F; i++) {
		logPrintf(file, "Profile %d", i);
		logPrintf(file, "SSID: %s", getWifiSSID(i));
		logPrintf(file, "Password: %s", getWifiPassword(i));	
	} logPrintf(file, "\n");
	
	
}

void printMenu(int menu) {

	psvDebugScreenClear();
	psvDebugScreenSetXY(1, 1);
	psvDebugScreenSetTextColor(GREEN);
	psvDebugScreenPrintf("PSVident %s", VERSION);	
	
	psvDebugScreenSetTextColor(GREY);
	if( censored ) psvDebugScreenPrintf(" -censored");	
	if( !nicemode ) psvDebugScreenPrintf(" -raw");
	
	////////////////////////////////////////////////
	
	switch( menu ) {
		case 0: hardware(); break;
		case 1: firmware(); break;
		case 2: software(); break;
		case 3: battery(); break;
		case 4: memorycard(); break;
		case 5: touchpanel(); break;
		case 6: activation(); break;
		case 7: wifiprofiles(); break;
		case 8: miscellaneous(); break;
		
		//case 4: partitions(); break;
		
		//case 3: consoleid(); break;
		//case 4: hardwareinfo(); break;
		//case 5: versiontxt(); break;
		
		default: hardware();
	}
}


int main(int argc, char *argv[]) {
	SceCtrlData pad, oldpad;
	oldpad.buttons = 0;
	int menu = 0;
	static SceUID kernel_id = -1, user_id = -1;
	
	initNet();
	initSceAppUtil();
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL); 
	
	psvDebugScreenInit();
	

	psvDebugScreenPrintf("loading kernel plugin.. ");
	kernel_id = taiLoadStartKernelModule(APP_PATH "PSVident_kernel.skprx", 0, NULL, 0);
	if(kernel_id < 0) {
		kernmode = 0;
		switch(kernel_id) {
			case 0x80010002: psvDebugScreenPrintf("SCE_ERROR_ERRNO_ENOENT\n\n"); break;
			case 0x8002D013: psvDebugScreenPrintf("SCE_KERNEL_ERROR_MODULEMGR_OLD_LIB\n\n"); kernmode = 1; break;
			case 0x8002D003: psvDebugScreenPrintf("SCE_KERNEL_ERROR_MODULEMGR_NO_LIB\n\n"); break;
			case 0x80024501: psvDebugScreenPrintf("SCE_KERNEL_ERROR_INVALID_UID\n\n"); break;
			case 0x80024302: psvDebugScreenPrintf("SCE_KERNEL_ERROR_NO_FREE_PHYSICAL_PAGE\n\n"); break;
			default: psvDebugScreenPrintf("0x%08X\n\n", kernel_id);
		}
	} else { psvDebugScreenPrintf("Ok\n\n"); kernmode = 1; }
	
	psvDebugScreenPrintf("loading user plugin.. ");
	user_id = sceKernelLoadStartModule(APP_PATH "PSVident_user.suprx", 0, NULL, 0, NULL, NULL);
	if(user_id < 0) {
		kernmode = 0;
		switch(user_id) {
			case 0x80010002: psvDebugScreenPrintf("SCE_ERROR_ERRNO_ENOENT\n\n"); break;
			case 0x8002D013: psvDebugScreenPrintf("SCE_KERNEL_ERROR_MODULEMGR_OLD_LIB\n\n"); break;
			case 0x8002D003: psvDebugScreenPrintf("SCE_KERNEL_ERROR_MODULEMGR_NO_LIB\n\n"); break;
			case 0x80024501: psvDebugScreenPrintf("SCE_KERNEL_ERROR_INVALID_UID\n\n"); break;
			default: psvDebugScreenPrintf("0x%08X\n\n", user_id);
		}
	} else { psvDebugScreenPrintf("Ok\n\n"); kernmode = 1; }
	
	//sceKernelDelayThread(1 * 1000 * 1000); //wait 1 seconds

	if( !kernmode ) 
		sendNotification("Failed to load helper plugin!");
	
	printMenu(0);
	
	while(1) {
		memset(&pad, 0, sizeof(pad));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		
		if( pad.buttons != oldpad.buttons ) {
			
			/// update values
			if( pad.buttons & SCE_CTRL_CROSS ) {
				printMenu(menu);
			}
			
			/// censored mode
			if( pad.buttons & SCE_CTRL_SQUARE ) {
				censored = 1 - censored;
				printMenu(menu); // print again to take effect
			}
			
			/// advanced mode
			if( pad.buttons & SCE_CTRL_TRIANGLE ) {
				nicemode = 1 - nicemode;
				printMenu(menu); // print again to take effect
			}
			
			/// reset
			if( pad.buttons & SCE_CTRL_CIRCLE ) {
				menu = 0;
				nicemode = 1;
				censored = 0;
				printMenu(menu);
			}
			
			/// menu next
			if( pad.buttons & SCE_CTRL_RIGHT ) {
				if( menu < 8 ) 
					printMenu(++menu);
			}
			
			/// menu previous
			if( pad.buttons & SCE_CTRL_LEFT ) {
				if( menu > 0 ) 
					printMenu(--menu);
			}
			
			/// save report
			if( pad.buttons & SCE_CTRL_UP ) {
				//uploadreport(); ?!
			}
			
			/// save report
			if( pad.buttons & SCE_CTRL_DOWN ) {
				savereport("ux0:data/psvident.txt");
				sendNotification("Report has been saved to ux0:data/psvident.txt");
			}
			
		}
		
		/// exit combo
		if( (pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT) )
			break;
		
		oldpad = pad;
	}
	
	
	/// unload user plugin 
	if(user_id >= 0) 
		sceKernelStopUnloadModule(user_id, 0, NULL, 0, NULL, NULL);
	
	/// unload kernel plugin 
		// possible?
		
	sceKernelExitProcess(0);
	return 0;
}
