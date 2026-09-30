#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h> 
#include <psp2/vshbridge.h> 
#include <psp2/io/fcntl.h>
#include <psp2/ctrl.h>
#include <psp2/sysmodule.h> 
#include <psp2/rtc.h> 

#include <string.h>
#include <stdio.h> 
#include <stdlib.h>

#include <taihen.h>

#include "main.h"
#include "psvident.h"
#include "utils/utils.h"
#include "print/pspdebug.h"


#define VERSION "v1.01"
#define APP_PATH "ux0:app/PSVIDENT0/" // "PSP2IDENT"


int COL_TITLE = SCEGREEN;
int COL_CATEG = SCEMAGENTA; // replaced with device specific color later
int COL_ALERT = SCEMAGENTA;
int COL_TEXTS = WHITE;
int COL_WARNG = YELLOW;
int COL_ERROR = RED;


int RAW = 0;      // display raw values
int CENSORED = 0; // hide crucial info (not in raw-mode though)

int kernmode = -1; // helper plugin load status


int ret = 0, ret2 = 0;
uint32_t rawval = -1, rawval2 = -1;
uint8_t retstr[256], rawstr[256], retstr2[256], rawstr2[256];

////////////////////  ////////////////////////////  /////////////////////////  ///////////////////////  ////////////////////  ////////////////////////////

void overview() {
  int x1, x2, y;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Overview");

  /***************************************************************************** */
    x1 = 68, y = 1;
    psvDebugScreenSetXY(50, y);
    psvDebugScreenSetTextColor(COL_ALERT);

    /// Prototype
    ret = getSerial(retstr, rawstr); 
    if( ret >= 0 ) {
      if( (rawstr[0] == 'T' && rawstr[1] == 'G') || (rawstr[1] == 'T' && rawstr[2] == 'G') ) { // either "00-TG.." or "03-TG.."
        psvDebugScreenSetXY(x1-10, y);
        psvDebugScreenPrintf("Prototype!");
        y+=2;
      }
    }

    /// Refurbished
    ret = getRefurbished(retstr, &rawval); 
    if( ret >= 0 ) {
      if( rawval == TRUE ) {
        psvDebugScreenSetXY(x1-12, y);
        psvDebugScreenPrintf("Refurbished!");
        y+=2;
      }
    }

    /// IDU
    ret = getModeIDU(retstr, &rawval); 
    if( ret >= 0 ) {
      if( rawval == TRUE ) {
        psvDebugScreenSetXY(x1-9, y);
        psvDebugScreenPrintf("IDU Mode!");
        y+=2;
      }
    }

    /// Internal Firmware 
    ret = getFirmwareInternal(retstr, &rawval); 
    if( ret >= 0 ) {
      if( rawval == TRUE ) {
        psvDebugScreenSetXY(x1-16, y);
        psvDebugScreenPrintf("Internal Keyset!");
        y+=2;
      }
    }

    /// QA Flagged
    ret = getNvsQaf(retstr, &rawval);
    if( ret >= 0 && rawval == 0x00 ) {
      psvDebugScreenSetXY(x1-11, y);
      psvDebugScreenPrintf("QA-Flagged!");
      y+=2;
    }



  /***************************************************************************** */
  
  x1 = 3, x2 = 24, y = 6;
    
  /// Model
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Model:");
  psvDebugScreenSetXY(x2, y);
  ret = getModelName(retstr, rawstr);
  if( ret >= 0 ) { // success
    if( RAW ) {
      psvDebugScreenPrintf("%s", rawstr);
    } else {
      ret2 = getModelColor(retstr2, &rawval2);
      if( ret2 >= 0 && ret2 != 999 ) { /// eg 999 for PDEL/PTEL as they are always black anyways
        psvDebugScreenPrintf("%s (%s)", retstr, retstr2);
      } else psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  

  /// Region
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Region:");
  psvDebugScreenSetXY(x2, y);
  //ret = getRegion(0, retstr, &rawval); // via SblAimgr           -> on for example CEX2DEX modified consoles "getRegionForDev()" doesn't work so this fails
  ret2 = getRegion(1, retstr2, &rawval2); // via IDStorage
  if( /*ret >= 0 &&*/ ret2 >= 0 ) { // success
    //if( rawval == rawval2 ) { // same same
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
      else psvDebugScreenPrintf("%s", retstr2);

    /* } else { // anomaly
      /// original
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
      else psvDebugScreenPrintf("%s", retstr2);

      /// current (in yellow)
      psvDebugScreenSetXY(x2 + 22, y);
      psvDebugScreenSetTextColor(COL_WARNG);
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
      else psvDebugScreenPrintf("%s", retstr);
    } */
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  
  /// Target
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Target:");
  psvDebugScreenSetXY(x2, y);
  //ret = getTarget(0, retstr, &rawval); // via SblAimgr
  ret2 = getTarget(1, retstr2, &rawval2); // via IDStorage
  if( /*ret >= 0 &&*/ ret2 >= 0 ) { // success
    //if( rawval == rawval2 ) { // same same
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
      else psvDebugScreenPrintf("%s", retstr2);

    /*} else { // anomaly
      /// original
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
      else psvDebugScreenPrintf("%s", retstr2);

      /// current (in yellow)
      psvDebugScreenSetXY(x2 + 22, y);
      psvDebugScreenSetTextColor(COL_WARNG);
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
      else psvDebugScreenPrintf("%s", retstr);
    }*/
    
  } else { // backup
    ret = getTarget(2, retstr, &rawval); // works without helper plugins
    if( ret >= 0 ) {
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
  }
  y += 2;
  
  
  /// Motherboard
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Motherboard:");
  psvDebugScreenSetXY(x2, y);
  ret = getMotherboard(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
    if( vshSblAimgrIsTool() == 1 ) {
      ret = getCpboard(retstr, rawstr);
      if( ret >= 0 ) { // success
        psvDebugScreenPrintf(" + %s", RAW ? rawstr : retstr);
      }
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;




  /// Current Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getCurrentFirmware(1, retstr, &rawval); // os0
  ret2 = getCurrentFirmware(2,retstr2, &rawval2); // index.dat
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      if( ret2 >= 0 ) // reading index.dat worked
        psvDebugScreenPrintf("0x%08X", rawval2); // with minor version number
      else // via os0
        psvDebugScreenPrintf("0x%08X", rawval);

    } else { // print nice
      if( ret2 >= 0 ) { // reading index.dat worked
        if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) // Developer firmware - print full
          psvDebugScreenPrintf("%s (%s)", retstr2, getVersionTxtStringValueForKey("build")); // X.YYY.ZZZ

        else  // normal CEX firmware (display via os0)
          psvDebugScreenPrintf("%s (%s)", retstr, getVersionTxtStringValueForKey("build")); // X.YY

      } else { // via os0 (reading dat failed - no build)
        psvDebugScreenPrintf("%s", retstr); // X.YZ
      }


      ret = getFirmwareInternal(retstr, &rawval);
      if( ret >= 0 && rawval == 1 ) { // success & internal
        strcat(retstr, " [internal]");
      }
    }
  } else { // fallback if helper function not loaded
    psvDebugScreenSetTextColor(COL_TEXTS);
    ret = getCurrentFirmware(0, retstr, &rawval);  // works in usermode but may be spoofed! 
    if( ret >= 0 ){
      if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
  }
  y += 2;


  /// Min Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Min. Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getMinFirmware(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;

  
  

  /// Model Code + Serial "XX-YYYYYYY-ZZZZZZZ"
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Serial:");
  psvDebugScreenSetXY(x2, y);
  ret = getSerial(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 12, y);
      psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Console ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("ConsoleID:");
  psvDebugScreenSetXY(x2, y);
  ret = getConsoleID(0, retstr, rawstr); // current
  ret2 = getConsoleID(1, retstr2, rawstr2); // idstorage copy
  if( ret >= 0 && ret2 >= 0 ) { // success
    if( strcmp(rawstr, rawstr2) == 0 ) { // same same
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }

    } else { // anomaly detected - print both
      psvDebugScreenPrintf("%s", RAW ? rawstr2 : retstr2); // original in white
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }
      
      psvDebugScreenSetXY(x2, ++y);
      psvDebugScreenSetTextColor(COL_WARNG);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr); // current in-use one in yellow
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y+=2;



  /// MAC Address
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("MAC Address:");
  psvDebugScreenSetXY(x2, y);
  ret = getMacAddressWifi(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 12, y);
      psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
    }
    
    ret = getMacAddressLanViaIDStorage(retstr2, rawstr2);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("  %s", RAW ? rawstr2 : retstr2);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 16 + 3 + 12, y);
        psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
      }
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;



  /// SoC rev: https://wiki.henkaku.xyz/vita/Pervasive#.22SoC_revision.22
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SoC Revision:");
  psvDebugScreenSetXY(x2, y);
  ret = getSoCRevision(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Hardware Info
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Hardware Info:");
  psvDebugScreenSetXY(x2, y);
  ret = getHardwareInfo(retstr, &rawval); // getHardwareInfoViaIDStorage() - like it was at factory
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
    
  

  /*
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("getTest:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTest());
  */


  
}

void hardware() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Hardware");
      
    
  
  /// Motherboard
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Motherboard:");
  psvDebugScreenSetXY(x2, y);
  ret = getMotherboard(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Motherboard Code + Serial
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Board Serial:");
  psvDebugScreenSetXY(x2, y);
  ret = getKibanId(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 18, y);
      psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  /// Hardware Info
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Hardware Info:");
  psvDebugScreenSetXY(x2, y);
  ret = getHardwareInfo(retstr, &rawval); // getHardwareInfoViaIDStorage() - like it was at factory
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  y += 1;


  if( vshSblAimgrIsTool() == 1 ) { // Devkit

    /// Motherboard
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 CP-Board:");
    psvDebugScreenSetXY(x2, y);
    ret = getCpboard(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// CP-Board ID (original married CP serial from idstorage!)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 CP-Board Serial:");
    psvDebugScreenSetXY(x2, y);
    ret = getCpKibanId(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 18, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// CP-Board Info
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 CP-Board Info:");
    psvDebugScreenSetXY(x2, y);
    ret = getCpInfo(retstr, rawstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 3;
  } 
  
  
    

  /// SoC (Kermit): https://wiki.henkaku.xyz/vita/Pervasive#revision0
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SoC Model:");
  psvDebugScreenSetXY(x2, y);
  ret = getSoC(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  /// SoC (Kermit): https://wiki.henkaku.xyz/vita/Pervasive#revision0
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SoC DRAM:");
  psvDebugScreenSetXY(x2, y);
  ret = getSoCDram(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  
  /// SoC (Kermit) rev: https://wiki.henkaku.xyz/vita/Pervasive#revision0
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SoC Revision:");
  psvDebugScreenSetXY(x2, y);
  ret = getSoCRevision(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;



  /// Syscon (Ernie) 
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SysCon Revision:");
  psvDebugScreenSetXY(x2, y);
  ret = getSysconRevisionViaIDStorage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;*/

  /// Syscon (Ernie) - https://wiki.henkaku.xyz/vita/Ernie#CMD_0x0001_-_GetBaryonVersion current installed Ernie firmware (ex: 0x0100060B -> 1.0.6.11) 
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SysCon Version:");
  psvDebugScreenSetXY(x2, y);
  ret = getBaryonVersion(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  // getBaryonTimestamp()); // todo
  y += 2;  

  
  /// Bootloader rev:
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Bootloader rev:");
  psvDebugScreenSetXY(x2, y);
  ret = getBootloaderVersion(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("0x%X", rawval);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;

  
  /// eMMC size
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("eMMC size:");
  psvDebugScreenSetXY(x2, y);
  ret = getEmmcSize(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval); // " bytes"
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;
  
  
  



  /*
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("getTest:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTest());
  */
}
  
void software() {
  int x1, x2, x3, x4, y, y2;

  x1 = 3, x2 = 24, x3 = 37, x4 = 58, y = 6, y2 = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Modes & Flags");
  
  
  /// Mode - Flight
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Flight Mode:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/SYSTEM", "flight_mode"));
  y += 2;  

  /// Mode - IDU
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Demo Mode:");
  psvDebugScreenSetXY(x2, y);
  ret = getModeIDU(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Mode - Download
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Downloader Mode:");
  psvDebugScreenSetXY(x2, y);
  ret = getModeDownloader(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  
  
  /// Mode - Manu
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Manufacturing Mode:");
  psvDebugScreenSetXY(x2, y);
  ret = getModeManufacturing(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// NVS - Savemode Flag (0x483)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Save Mode:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsSavemode(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%02X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } y += 2;

  /// NVS - Update Mode (0x4A0)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Update Mode:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsUpdateMode(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%02X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } y += 2;


  y += 2;  
  

  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  


  if( vshSblAimgrIsDEX()== 1 ) { // Testkit
      
    /// Mode - Show
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Show Mode:");
    psvDebugScreenSetXY(x4, y2);
    ret = getModeShow(retstr, &rawval);
    if( ret >= 0 ) { // success    
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;  

  }

  if( vshSblAimgrIsTool() == 1 ) { // Devkit

    /// Mode - Development (aka not Release Check Mode)        bootparam:/development_mode
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Development Mode:");
    psvDebugScreenSetXY(x4, y2);
    ret = getModeDevelopment(retstr, &rawval);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;

    /// Release Check Mode Console                                  bootparam:/release_check_mode_console
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Release Mode TTY:");
    psvDebugScreenSetXY(x4, y2);
    ret = getModeReleaseCheckConsoleForDevkit(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;
    
    /// Mode - PSTV emulation                    bootparam:/platform_emulation_dolce
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 PSTV Emulation:");
    psvDebugScreenSetXY(x4, y2);
    ret = getModePstvForDevkit(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;
    
    /// Mode - Memory Size                      bootparam:/memory_size_switch
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Memory Size:");
    psvDebugScreenSetXY(x4, y2);
    ret = getModeMemSizeForDevkit(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;
        
  } y2 += 1;  



  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  


  
  /// QA Flags
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("QA Flags:");
  psvDebugScreenSetXY(x2, y);
  ret = getQAFlags(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", rawstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 1;  
  ret = getNvsQaf(retstr, &rawval);
  if( ret >= 0 && rawval == 0x00 ) { // theres an active token
    psvDebugScreenSetXY(x2, y);
    ret = getQATokenName(retstr); // only printed if found because reading from nvs is slow
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else { // sceSblQafMgr failed
      ret2 = getQATokenNameViaNvs(retstr2); 
      if( ret2 >= 0 ) { // but there is one in NVS
        psvDebugScreenSetTextColor(COL_WARNG);
        psvDebugScreenPrintf("%s (inactive)", retstr2);
      } 
    } 
  }
  y += 2;
  
  

  /// Boot Flags
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Boot Flags:");
  psvDebugScreenSetXY(x2, y);
  ret = getBootFlags(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  


  /// Hardware Flags
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Hardware Flags:");
  psvDebugScreenSetXY(x2, y);
  ret = getHardwareFlags(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;


  /// Switches
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("DIP Switches:");
  psvDebugScreenSetXY(x2, y);
  ret = getDipSwitches(retstr, &rawval, 0);
  if( ret >= 0 ) { // success

    ret = getDipSwitches(retstr, &rawval, 0); // cp_timestamp_1
    psvDebugScreenPrintf("%s", retstr);
    ret = getDipSwitches(retstr, &rawval, 1); // cp_version
    psvDebugScreenPrintf(" %s", retstr);
    ret = getDipSwitches(retstr, &rawval, 2); // cp_build_id
    psvDebugScreenPrintf("%s", retstr);
    ret = getDipSwitches(retstr, &rawval, 3); // cp_timestamp_2
    psvDebugScreenPrintf(" %s", retstr);
    ret = getDipSwitches(retstr, &rawval, 4); // aslr_seed
    psvDebugScreenPrintf(" %s", retstr);
    y += 1;  

    psvDebugScreenSetXY(x2, y);
    ret = getDipSwitches(retstr, &rawval, 5); // sce_sdk_flags
    psvDebugScreenPrintf("%s", retstr);
    ret = getDipSwitches(retstr, &rawval, 6); // shell_flags
    psvDebugScreenPrintf(" %s", retstr);
    ret = getDipSwitches(retstr, &rawval, 7); // debug_control_flags
    psvDebugScreenPrintf(" %s", retstr);
    ret = getDipSwitches(retstr, &rawval, 8); // system_control_flags
    psvDebugScreenPrintf(" %s", retstr);

  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;



  /// NVS
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("NVS:");
  psvDebugScreenSetXY(x2, y);
  //psvDebugScreenPrintf("%s", getBootFlags());
  y += 3;  */


}  

void consoleid() {
  int x1, x2, y;
  static int FAKECID = 0; // anomaly flag

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Console Identifiers");
  
  
  /// Console ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("ConsoleID:");
  psvDebugScreenSetXY(x2, y);
  ret = getConsoleID(0, retstr, rawstr); // current
  ret2 = getConsoleID(1, retstr2, rawstr2); // idstorage copy
  if( ret >= 0 && ret2 >= 0 ) { // success
    if( strcmp(rawstr, rawstr2) == 0 ) { // same same
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }

    } else { // anomaly detected - print both
      FAKECID = 1;
      psvDebugScreenPrintf("%s", RAW ? rawstr2 : retstr2); // original in white
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }

      psvDebugScreenSetXY(x2, ++y);
      psvDebugScreenSetTextColor(COL_WARNG);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr); // current in-use one in yellow
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 25, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB \xDB\xDB\xDB\xDB");
      }
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y+=2;



    /// Console ID - Company
    /*psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Company Code:");
    psvDebugScreenSetXY(x2, y);
    ret = getConsoleIdCompanycode(0, retstr, &rawval); // current
    ret2 = getConsoleIdCompanycode(1, retstr2, &rawval2); // original
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( rawval == rawval2 ) { // same same
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);

      } else { // anomaly detected - print both
        /// original
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
        else psvDebugScreenPrintf("0x%X -> %s", rawval2, retstr2);
        
        /// current (in yellow)
        psvDebugScreenSetXY(x2 + 22, y);
        psvDebugScreenSetTextColor(COL_WARNG);
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;*/
    
    /// Console ID - Product
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Product Code:");
    psvDebugScreenSetXY(x2, y);
    ret = getConsoleIdProductcode(0, retstr, &rawval); // current
    ret2 = getConsoleIdProductcode(1, retstr2, &rawval2); // original
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( rawval == rawval2 ) { // same same
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);

      } else { // anomaly detected - print both
        /// original
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
        else psvDebugScreenPrintf("0x%X -> %s", rawval2, retstr2);
        
        /// current (in yellow)
        psvDebugScreenSetXY(x2 + 22, y);
        psvDebugScreenSetTextColor(COL_WARNG);
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Console ID - Subcode
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Product SubCode:");
    psvDebugScreenSetXY(x2, y);
    ret = getConsoleIdSubcode(0, retstr, &rawval); // current
    ret2 = getConsoleIdSubcode(1, retstr2, &rawval2); // original
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( rawval == rawval2 ) { // same same
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);

      } else { // anomaly detected - print both
        /// original
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
        else psvDebugScreenPrintf("0x%X -> %s", rawval2, retstr2);
        
        /// current (in yellow)
        psvDebugScreenSetXY(x2 + 22, y);
        psvDebugScreenSetTextColor(COL_WARNG);
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Console ID - Factory
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Factory Code:");
    psvDebugScreenSetXY(x2, y);
    ret = getConsoleIdFactoryCode(0, retstr, &rawval); // current
    ret2 = getConsoleIdFactoryCode(1, retstr2, &rawval2); // original
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( rawval == rawval2 ) { // same same
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);

      } else { // anomaly detected - print both
        /// original
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
        else psvDebugScreenPrintf("0x%X -> %s", rawval2, retstr2);
        
        /// current (in yellow)
        psvDebugScreenSetXY(x2 + 22, y);
        psvDebugScreenSetTextColor(COL_WARNG);
        if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
        else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Console ID - Serial
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Serial Number:");
    psvDebugScreenSetXY(x2, y);
    if( !CENSORED ) {
      ret = getConsoleIdSerial(0, retstr, &rawval); // current
      ret2 = getConsoleIdSerial(1, retstr2, &rawval2); // original
      if( ret >= 0 && ret2 >= 0 ) { // success
        if( rawval == rawval2 ) { // same same
          if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
          else psvDebugScreenPrintf("0x%X -> No. %s", rawval, retstr);

        } else { // anomaly detected - print both
          /// original
          if( RAW ) psvDebugScreenPrintf("0x%X", rawval2);
          else psvDebugScreenPrintf("0x%X -> %s", rawval2, retstr2);

          /// current (in yellow)
          psvDebugScreenSetXY(x2 + 22, y);
          psvDebugScreenSetTextColor(COL_WARNG);
          if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
          else psvDebugScreenPrintf("0x%X -> %s", rawval, retstr);
        }
      } else psvDebugScreenPrintf(error(ret, "ERROR"));
    } else psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
    y += 2;
    
  y += 1;
  y += 1;

  /// pscode
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("PSCode:");
  psvDebugScreenSetXY(x2, y);
  if( FAKECID ) psvDebugScreenSetTextColor(COL_WARNG);
  ret = getPscode(retstr, rawstr); 
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

    /// Factory Code
    /*psvDebugScreenSetXY(x1 + 2, y);
    psvDebugScreenSetTextColor(COL_TEXTS);getMacAddressWifiViaIDStorage
    psvDebugScreenPrintf("Factory Code:");
    psvDebugScreenSetXY(x2, y);
    ret = getPscodeFactory(retstr, &rawval);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("0x%X", rawval);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;*/

  y += 1;  

  /// OpenPSID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("OpenPSID:");
  psvDebugScreenSetXY(x2, y);
  ret = getOpenPsid(retstr, rawstr); // current
  ret2 = getOpenPsidViaIDStorage(retstr2, rawstr2); // idstorage copy
  if( ret >= 0 && ret2 >= 0 ) { // success
    if( strcmp(rawstr, rawstr2) == 0 ) { // same same
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 24, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }

    } else { // anomaly detected - print both
      psvDebugScreenPrintf("%s", RAW ? rawstr2 : retstr2); // original
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 24, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }

      psvDebugScreenSetXY(x2, ++y);
      psvDebugScreenSetTextColor(COL_WARNG);
      psvDebugScreenPrintf("%s (org)", RAW ? rawstr : retstr); // current
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 24, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }

    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;

    
  /// USB/MTP Serial    https://wiki.henkaku.xyz/vita/IdStorage#0x112_-_MtpSerial
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("USB/MTP Serial:");
  psvDebugScreenSetXY(x2, y);
  ret = getMtpSerial(retstr, rawstr); 
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 24, y);
      psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;
  
  /// CMA ID (AccountID / AID)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Content Manager ID:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
  else 
    psvDebugScreenPrintf("%s", getRegistryBinaryNow("/CONFIG/NP", "account_id", 8));
  y += 3;  


  /* if( vshSblAimgrIsTool() == 1 ) { // Devkit

    /// Neighborhood ID (comes from CP-Board. Can be replaced by nickname in NBH)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Neighborhood ID:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", warning(-1, "TODO")); // somewhere on CP-board. Is it even accessible?
    y += 3;
  } */


}

void firmware() {
  int x1, x2, x3, x4, x5, y;

  x1 = 3, x2 = 24, x3 = 31, x4 = 37, x5 = 56, y = 6; // x5 = 58
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Firmware");
  
  
  // LEFT 

  /// Current Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Current Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getCurrentFirmware(1, retstr, &rawval); // os0
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else {
    ret = getCurrentFirmware(0, retstr, &rawval);  // works in usermode
    if( ret >= 0 ){
      if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
  }
  y += 2;
  
  /// Previous Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Previous Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getPreviousFirmware(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  

  /// Factory Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Factory Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getFactoryFirmware(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  

  /// Min. Firmware
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Minimum Firmware:");
  psvDebugScreenSetXY(x2, y);
  ret = getMinFirmware(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%08X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;
  

  // RIGHT
  y = 6;

  /// Interal Firmware
  psvDebugScreenSetXY(x4, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Build Keyset:");
  psvDebugScreenSetXY(x5, y);
  ret = getFirmwareInternal(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  /// Last Firmware udpdate date
  psvDebugScreenSetXY(x4, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Last updated:");
  psvDebugScreenSetXY(x5, y);
  ret = getDateLastFirmwareUpdated(retstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", retstr);
  } //else psvDebugScreenPrintf("NO DATA"); 
  y += 2;  

  y += 2; 


  /// ePSP Firmware
  psvDebugScreenSetXY(x4, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("ePSP Firmware:"); 
  psvDebugScreenSetXY(x5, y);
  ret = getPspemuFirmware(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  

  // DETAILS
  y = 15;


  /// index.dat
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  char buf[2048];
  memset(&buf[0], 0, sizeof(buf));
  sprintf(buf, "%s", getVersionTxtString());
  while( strcspn(buf, "\r\n") != strlen(buf) ) // In case there is no match, the end of string (position of null character ) is returned.
    buf[strcspn(buf, "\r\n")] = ' '; // replace
  int counter = 0;
  //int init_size = strlen(buf);
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
  y += 2;
  
    
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_WARNG);
  psvDebugScreenPrintf("release:");
  psvDebugScreenSetXY(x1, ++y);
  psvDebugScreenPrintf("build:");
  psvDebugScreenSetXY(x1, ++y);
  psvDebugScreenPrintf("system:");
  psvDebugScreenSetXY(x1, ++y);
  psvDebugScreenPrintf("vsh:");
  psvDebugScreenSetXY(x1, ++y);
  psvDebugScreenPrintf("target");*/

  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  const char *p = getPspemuVersionTxt();
  while (*p) {
    const char *start = p;

    while (*p && *p != '\n' && *p != '\r')
      p++;
    
    psvDebugScreenSetXY(x1, y++);
    psvDebugScreenPrintf("%.*s\n", (int)(p - start), start);

    if (*p == '\r' && p[1] == '\n') p += 2;
    else if (*p) p++;
  }
  
}

void battery() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Battery");
      
    
  /// Status
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("State of Charge:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryPercentage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s%%", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
    
  /// Capacity
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Capacity:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryRemCapacity(retstr, &rawval);
  ret2 = getBatteryCapacity(retstr2, &rawval2);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d / %d", rawval, rawval2);
    } else { // print nice
      psvDebugScreenPrintf("%s of %s mAh", retstr, retstr2);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  
  /// Lifetime
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Lifetime:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryLifeTime(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  /// Temperature
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Temperature:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryTempInCelsius(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s Celsius", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  /// Voltage
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Voltage:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryVoltage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s mV", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  /// Cycle Count
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Cycle Count:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryCycleCount(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  /// State of Health
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("State of Health:");
  psvDebugScreenSetXY(x2, y);
  ret = getBatteryStateOfHealth(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s%%", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  
  
  y += 2;
  


  if( vshSblAimgrIsDolce() == 0 &&  vshSblAimgrIsTool() == 0 ) { // ignore PSTVs und devkits

     
    /// Calibration
    /*psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Calibration Data:");
    psvDebugScreenSetXY(x2, y);
    ret = getBatteryCalibrationVoltage(retstr, &rawval);
    ret2 = getBatteryCalibrationCurrent(retstr2, &rawval2);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X Voltage, 0x%X Current", rawval, rawval2);
      } else { // print nice
        psvDebugScreenPrintf("%s Voltage, %s Current", retstr, retstr2);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 3; */

    /// Calibration
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Calibration Data:");
    psvDebugScreenSetXY(x2, y);
    ret = getBatteryCalibrationVoltage(retstr, &rawval);
    ret2 = getBatteryCalibrationCurrent(retstr2, &rawval2);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("0x%X Voltage", rawval);
      y += 2;
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("0x%X Current", rawval2);
      
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 4;


    /// Factory 
    ret = getBatterySerial(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("Factory Info:");
      psvDebugScreenSetXY(x2, y);
      if( RAW ) { // raw
        psvDebugScreenPrintf("%s", retstr);

      } else {
        getBatterySerialModel(retstr, rawstr);
        psvDebugScreenPrintf("\"%s\"", retstr);
        
        getBatterySerialDate(retstr, rawstr);
        psvDebugScreenPrintf(" (%s)", retstr);
        
        if( CENSORED ) {
          psvDebugScreenPrintf(" \xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
        } else {
          getBatterySerialNumber(retstr, rawstr);
          psvDebugScreenPrintf(" %s", retstr);
        }
      }    
      y += 2;
    }  
    
    /*ret = getBatterySerial(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("Factory Info");
  
      if( RAW ) { // raw
        psvDebugScreenPrintf(":");
        psvDebugScreenSetXY(x2, y);
        psvDebugScreenPrintf("%s", retstr);

      } else {
        y+=2;
        psvDebugScreenSetXY(x1 + 2, y);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("Model:");
        getBatterySerialModel(retstr, rawstr);
        psvDebugScreenSetXY(x2, y);
        psvDebugScreenPrintf("\"%s\"", retstr);
        y+=2;
        psvDebugScreenSetXY(x1 + 2, y);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("Manufactured:");
        getBatterySerialDate(retstr, rawstr);
        psvDebugScreenSetXY(x2, y);
        psvDebugScreenPrintf("%s", retstr);
        y+=2;
        psvDebugScreenSetXY(x1 + 2, y);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("Serial:");
        getBatterySerialNumber(retstr, rawstr);
        psvDebugScreenSetXY(x2, y);
        psvDebugScreenPrintf("%s", retstr);
      }    
      y += 2;
    } */
    
    
  }
  
}

void power() {
  int x1, x2, x3, x4, y, y2;

  x1 = 3, x2 = 24, x3 = 37, x4 = 58, y = 6, y2 = 6;

  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Charging & USB");

  ///// ///// ///// ///// ///// ///// ///// ///// ///// 

  /// AC connected
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("AC Connected:");
  psvDebugScreenSetXY(x4, y2);
  ret = getBatteryPowerOnline(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y2 += 2;


  /// Battery charging
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Battery Charging:");
  psvDebugScreenSetXY(x4, y2);
  ret = getBatteryCharging(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y2 += 2;


  ///// ///// ///// ///// ///// ///// ///// ///// ///// 


  /// USB Cable connected
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("USB Connected:");
  psvDebugScreenSetXY(x2, y);
  ret = getUdcdState(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf(rawval & 0x20 ? "TRUE" : "FALSE"); // SCE_UDCD_STATUS_CABLE_CONNECTED
  }
  y += 2;

  /// USB connection established
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("USB Established:");
  psvDebugScreenSetXY(x2, y);
  ret = getUdcdState(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf(rawval & 0x2 ? "TRUE" : "FALSE"); // SCE_UDCD_STATUS_CONNECTION_ESTABLISHED
  }
  y += 2;

  /// USB charging
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("USB-Charge enabled:");
  psvDebugScreenSetXY(x2, y);
  ret = getUdcdState(retstr, &rawval);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf(rawval & 0x0800 ? "TRUE" : "FALSE"); // SCE_UDCD_STATUS_USE_USB_CHARGING
  }
  y += 2;

  y += 1;
  



  /// UDCD
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("UDCD State:");
  psvDebugScreenSetXY(x2, y);
  ret = getUdcdState(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  


  y += 3;


  //// //// //// //// //// //// //// //// //// //// //// //// //// //// 


  /// Power Management IC (Elmo)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("PowerMan Version:");
  psvDebugScreenSetXY(x2, y);
  ret = getPowerManVersionViaIDStorage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  


 if( vshSblAimgrIsDolce() == 0 && vshSblAimgrIsTool() == 0 ) { // ignore PSTVs und devkits

    /// USB Charge Management IC (Cookie)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("USB ChargeMan Ver:");
    psvDebugScreenSetXY(x2, y);
    ret = getUsbChargeManVersionViaIDStorage(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 3;  

    /// HWinfo (Abby)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("FuelGauge Revision:");
    psvDebugScreenSetXY(x2, y);
    ret = getBatteryVersionHwinfo(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// FWinfo (Abby)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("FuelGauge Version:");
    psvDebugScreenSetXY(x2, y);
    ret = getBatteryVersionFwinfo(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// DFinfo (Abby)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("FuelGauge Param:"); // Design Factor / Device Flags ?
    psvDebugScreenSetXY(x2, y);
    ret = getBatteryVersionDfinfo(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    y += 1;
    
  }
 

}

void identifiers() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Factory Information");
      
  /// Model Serial
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Device Serial:");
  psvDebugScreenSetXY(x2, y);
  ret = getSerial(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 12, y);
      psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Console Id
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Device Identifier:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getConsoleID());
  y += 2;*/

  y += 1;

  
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Component-Serials");
  y += 2;

    /// Motherboard
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x7 Mainboard:");
    psvDebugScreenSetXY(x2, y);
    ret = getKibanId(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 18, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    if( vshSblAimgrIsTool() == 1 ) { // Devkit
    
      /// CP-Board ID (only devkit)
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 \x80 CP-Board:");
      psvDebugScreenSetXY(x2, y);
      ret = getCpKibanId(retstr, rawstr);
      if( ret >= 0 ) { // success
        psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
        if( CENSORED ) {
          psvDebugScreenSetXY(x2 + 18, y);
          psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
        }
      } else psvDebugScreenPrintf(error(ret, "ERROR"));
      y += 2;
    }
    
    /// OLED (only Fat model)
    ret = getOledSerial1(retstr, rawstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1 , y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 OLED Screen:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      y += 2;
    }


    /// Unknown (OLED model only?)
    ret = getUnknown194(retstr, rawstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1 , y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Unknown 194:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      y += 2;
    }
    

    /// LCD (only Slim model)
    ret = getLcdSerial(retstr, rawstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 LCD Screen:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      y += 2;
    }

    /// Battery (all none PSTV)
    ret = getBatterySerial(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Battery:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 9, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
      }
      y += 2;
    }

    /// Unknown (3g unit only?)
    ret = getUnknown120(retstr, rawstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1 , y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Unknown 120:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      y += 2;
    }

    /// Touchpads (all none PSTV)
    if( vshSblAimgrIsVITA() == 1 ) { // ignore PSTVs

      psvDebugScreenSetXY(x1 + 1 , y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Touchpads:");
      psvDebugScreenSetXY(x2, y);  
      ret = getTouchpanelInfoViaIDStorage(retstr, &rawval, 6);
      if( ret >= 0 ) { // success
        psvDebugScreenPrintf("%s", retstr);
        ret = getTouchpanelInfoViaIDStorage(retstr, &rawval, 7);
        if( ret >= 0 ) { // success
          psvDebugScreenPrintf(" + %s", retstr);
        } 
      } else psvDebugScreenPrintf(error(ret, "ERROR"));
      y += 2;
    }

  y += 1;


  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("MAC Addresses");
  y += 2;

    /// WLAN 
    ret = getMacAddressWifiViaIDStorage(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Wifi:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 12, y);
        psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
      }
      y += 2;
    } 


    /// Bluetooh
    ret = getMacAddressBluetoothViaIDStorage(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Bluetooth:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 12, y);
        psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
      }
      y += 2;
    } 
    
    /// Ethernet
    ret = getMacAddressLanViaIDStorage(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 LAN:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 12, y);
        psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
      }
      y += 2;
    } 

  y += 1;
  

  /// SMI
  if( !RAW ) {
    /// SMI String 1
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("SMI String:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getSmiString1(9)); // 9 is factory firmware version, date & target
    y += 2;
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getSmiString1(10)); // 10 is some other version
    y += 3;
  
  } else { // raw -> show both full strings

    /// SMI String 1
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("%s", getSmiString1(-1)); 
    y += 3;  

    /// SMI String 2
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("%s", getSmiString2(-1)); 
    y += 2;  
  }
    
}

void threegee() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Network, 3G & SIM");
      

  /// SSID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("SSID:");
  psvDebugScreenSetXY(x2, y);
  ret = getSsid(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// IP
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("IP:");
  psvDebugScreenSetXY(x2, y);
  ret = getLocalIp(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  

  
  /// WLAN 
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Wifi Mac:");
  psvDebugScreenSetXY(x2, y);
  ret = getMacAddressWifi(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 12, y);
      psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Mac OUI 
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Wifi Mac OUI:");
  psvDebugScreenSetXY(x2, y);
  ret = getMacAddressWifiOui(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;


  /// Wifi Region 
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Wifi Region:");
  psvDebugScreenSetXY(x2, y);
  ret = getWifiRegion(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  


  /// Bluetooh
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Bluetooth Mac:");
  psvDebugScreenSetXY(x2, y);
  ret = getMacAddressBluetoothViaIDStorage(retstr, rawstr); // TODO ACTUAL not idstorage
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    if( CENSORED ) {
      psvDebugScreenSetXY(x2 + 12, y);
      psvDebugScreenPrintf("\xDB\xDB:\xDB\xDB");
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;

  /// Bluetooth HW Revision
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Bluetooth Revision:"); // HARDWARE
  psvDebugScreenSetXY(x2, y);
  ret = getBluetoothRevisionViaIDStorage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  



  /// LAN Adapter
  /* if( !vshSblAimgrIsCEX() || !vshSblAimgrIsVITA() || vshSysconIsIduMode() ) { // only then module loads (see https://wiki.henkaku.xyz/vita/SceUsbEtherRtl)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("LAN Adapter MAC:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", warning(-1, "TODO"));
    y += 3;  
  } */



  if( vshSysconHasWWAN() ) {
    /// IMEI
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("IMEI:");
    psvDebugScreenSetXY(x2, y);
    ret = getImeiViaIDStorage(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
      if( CENSORED ) {
        psvDebugScreenSetXY(x2 + 10, y);
        psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB");
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Carrier 
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("SIM Carrier:");
    psvDebugScreenSetXY(x2, y);
    ret = getSimCarrier(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;  

    /// Locked?
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("SIM Locked:");
    psvDebugScreenSetXY(x2, y);
    ret = getSimLocked(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;  
  }
  
  y += 1;



  /// Phone Number
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Phone Number:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", warning(-1, "TODO"));
  y += 2;


  /// ICCID — Integrated Circuit Card Identifier (aka SIM card serial)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("ICCID:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", warning(-1, "TODO"));
  y += 2;


  /// IMSI — International Mobile Subscriber Identity
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("IMSI:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", warning(-1, "TODO"));
  y += 2;*/

}

void memorycard() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> MemoryCard");
      
  if( vshRemovableMemoryGetCardInsertState() == 1 ) { // a removeable memcard is inserted!  (NO internal / NO SD2VITA)
  
    /// MemCard - Type
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Type:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardType(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// MemCard - Size
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Size:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardSize(retstr, rawstr);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// MemCard - Date
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Manufacturing Date:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardDate(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// MemCard - Readonly
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Read Only:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardReadonly(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// MemCard - Sector Size
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Sector Size:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardSectorSize(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// MemCard - fs_offset
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("fs_offset:");
    psvDebugScreenSetXY(x2, y);
    ret = getMemCardFsoffset(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;


  } else {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS); // COL_WARNG
    psvDebugScreenPrintf("No physical memory card inserted!");
    y += 2;
  }
  y += 1;




  /// id.dat https://wiki.henkaku.xyz/vita/Id.dat
  if( !CENSORED ) {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("ux0:id.dat");
    //psvDebugScreenPrintf("%s", getIddatString());
    const char *p = getIddatString();
    while (*p) {
      const char *start = p;

      while (*p && *p != '\n' && *p != '\r')
        p++;
      
      psvDebugScreenSetXY(x1+2, y+=2);
      //psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("%.*s\n", (int)(p - start), start);

      if (*p == '\r' && p[1] == '\n') p += 2;
      else if (*p) p++;
    }
    y += 2;
  }

}

void touchpanel() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Touch & Motion");

  if( vshSblAimgrIsVITA() == 1 ) { // ignore PSTVs (vshSblAimgrIsVITA does not work in henkaku savemode?)

    int i, j, k = 0;
    const char *list1[] = { "Front Touch Panel", "Rear Touch Pad" };
    const char *list2[] = { "Vendor ID", "Firmware Rev", "Config Rev" };
    for(i = 0; i < 2; i++) {
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("%s", list1[i]);
      y += 2;

      for(j = 0; j < 3; j++, k++) {
        psvDebugScreenSetXY(x1 + 1, y);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("\x7 %s:", list2[j]);
        psvDebugScreenSetXY(x2, y);

        ret = getTouchpanelInfoViaSyscon(retstr, &rawval, k);
        ret2 = getTouchpanelInfoViaIDStorage(retstr2, &rawval2, k);

        if( ret >= 0 && ret2 >= 0 && strcmp(retstr, retstr2) == 0 ) { // same same
          psvDebugScreenPrintf("%s", retstr);

        } else {  // anomaly detected
          if( ret >= 0 ) {
            psvDebugScreenSetTextColor(COL_WARNG); 
            psvDebugScreenPrintf("%s", retstr);  
            psvDebugScreenSetTextColor(COL_TEXTS);
            psvDebugScreenPrintf(" (current)");  
          } else psvDebugScreenPrintf(error(ret, "ERROR"));

          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenPrintf("  vs  ");  

          if( ret2 >= 0 ) {
            //psvDebugScreenSetTextColor(COL_WARNG); 
            psvDebugScreenPrintf("%s", retstr2);
            psvDebugScreenSetTextColor(COL_TEXTS);
            psvDebugScreenPrintf(" (factory)");  
          } else psvDebugScreenPrintf(error(ret, "ERROR"));
        }
        y += 2;
      } 
      y += 1;
    }

    
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Motion Sensor");
    y += 2;

      /// Motion (Barkley) Firmware Revision
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Firmware Rev:");
      psvDebugScreenSetXY(x2, y);
      ret = getMotionSensorInfoViaIDStorage(retstr, &rawval, 0);
      if( ret >= 0 ) { // success
        psvDebugScreenPrintf("%s", retstr);
      } else psvDebugScreenPrintf(error(ret, "ERROR"));
      y += 2;
      
      /// Motion (Barkley) Sensor Hardware Information
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("\x7 Hardware Info:");
      psvDebugScreenSetXY(x2, y);
      ret = getMotionSensorInfoViaIDStorage(retstr, &rawval, 1);
      if( ret >= 0 ) { // success
        psvDebugScreenPrintf("%s", retstr);
      } else psvDebugScreenPrintf(error(ret, "ERROR"));
      y += 2;

      /// Motion (Barkley) Firmware Loader Information
      /*psvDebugScreenSetXY(x1 + 2, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("Firmware Loader:");
      psvDebugScreenSetXY(x2, y);
      psvDebugScreenPrintf("%s", warning(-1, "TODO"));*/

      y += 2;

    y += 1;

  } else { // PSTV
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS); // COL_WARNG
    psvDebugScreenPrintf("Not supported on this model!");
    y += 2;
  }


/*  
  /// Touchpanel - Front /Touch Vendor ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Front Vendor ID:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(0));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(0));
  y += 2;
  
  /// Touchpanel - Front /Touch Firmware Revision
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Front Firmware Rev:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(1));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(1));
  y += 2;
  
  /// Touchpanel - Front /Touch Configuration Revision
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Front Config Rev:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(2));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(2));
  y += 2;


  /// Touchpanel - Front /TouchpanelLotInfo
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Front Lot Info:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaIDStorage(6));
  y += 2;
  
  
  y += 1;
  
  
  /// Touchpanel - Rear /Touch Vendor ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Rear VendorID:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(3));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(3));
  y += 2;
  
  /// Touchpanel - Rear /Touch Firmware Revision
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Rear Firmware Rev:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(4));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(4));
  y += 2;
  
  /// Touchpanel - Rear /Touch Configuration Revision
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Rear Config Rev:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaSyscon(5));
  psvDebugScreenPrintf(" vs %s", getTouchpanelInfoViaIDStorage(5));
  y += 2;
  
  /// Touchpanel - Rear /TouchpanelLotInfo
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Rear Lot Info:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getTouchpanelInfoViaIDStorage(7));
  y += 2;
*/

  y += 1;
  
  
  /// Motion
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Motion Sensor FW Rev:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getMotionInfo());
  y += 2;*/
  
}

void activation() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> PSN & Activation");
      

  /// Activated
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("NP Activated:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/NP", "enable_np"));
  y += 3;  
  
  /// Username
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Username:");
  psvDebugScreenSetXY(x2, y);
  ret = getSystemUsername(retstr, rawstr); 
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", RAW ? rawstr : retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  
  
  
  /// Account ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Account ID:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED ) 
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
  else 
    psvDebugScreenPrintf("%s", getRegistryBinaryNow("/CONFIG/NP", "account_id", 8));
  y += 2;  
  
  /// Login ID
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Login ID:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
  else
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/NP", "login_id"));
  y += 2;  
  
  /// Password
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Password:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
  else
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/NP", "password"));
  y += 2;  
    
  
  /// Date of Birth
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Date of Birth:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB-\xDB\xDB-\xDB\xDB");
  else {
    if( getRegistryIntegerNow_("/CONFIG/NP", "yob") > 0 ) { // error check 
      psvDebugScreenPrintf("%04d-%02d-%02d", getRegistryIntegerNow_("/CONFIG/NP", "yob"), getRegistryIntegerNow_("/CONFIG/NP", "mob"), getRegistryIntegerNow_("/CONFIG/NP", "dob"));
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
  }
  y += 2;  
  
  /// Country Code
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Country Code:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/NP", "country"));
  y += 2;  
  
  /// Language
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Language:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/NP", "lang"));
  y += 2;  
  
  
  
  
  
  
  /// DEV ///////////////////////////////////////////  
  
  if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // Development Hardware
    y += 2;  
  
    /// Activation Status
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Activation Status:");
    psvDebugScreenSetXY(x2, y);
    ret = getActivationStatus(retstr, &rawval);
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// Activation Period
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Activation Period:");
    psvDebugScreenSetXY(x2, y);
    ret = getActivationPeriodNvs(retstr, &rawval); 
    ret2 = getActivationPeriodDat(retstr2, &rawval2); 
    if( ret >= 0 ) { // success
      if( ret2 < 0) // error tm0 files
        psvDebugScreenSetTextColor(COL_WARNG);

      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
    
    /// Activation Count (Issue No)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Activation Count:");
    psvDebugScreenSetXY(x2, y);
    ret = getActivationCountNvs(retstr, &rawval);
    ret2 = getActivationCountDat(retstr2, &rawval2); 
    if( ret >= 0 ) { // success
      if( ret2 < 0) // error tm0 files
        psvDebugScreenSetTextColor(COL_WARNG);
        
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Activation Files
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 Activation Files:");
    psvDebugScreenSetXY(x2, y);
    if( doesFileExist("tm0:activate/act.dat") == 0 && doesFileExist("tm0:activate/actsig.dat") == 0 ) {
      psvDebugScreenPrintf("Present");
    } else psvDebugScreenPrintf("Not Present");
    y += 2;

    
  }
  
}

void wifiprofiles() {
  int i, j, x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Wifi Profiles");
  

  x1 = 3, x2 = 35, y = 6;
  psvDebugScreenSetTextColor(COL_TEXTS);
  /*psvDebugScreenSetXY(x1+4, y);
  psvDebugScreenPrintf("SSID:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("Password:");
  y += 2;  */
  for( i = 1; i < 14; i++) {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);

    psvDebugScreenSetXY(x1, y);
    ret = getRegistryWifiSSID(retstr, i);
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%02d: %s", i, retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));

    psvDebugScreenSetXY(x2, y);
    ret = getRegistryWifiPassword(retstr, i);
    if( ret >= 0 ) { // success
      if( CENSORED && strlen(retstr) > 0) {
        for( j = 0; j < strlen(retstr); j++) // looks cooler ig
          psvDebugScreenPrintf("\xDB");
      } else {
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;  

  }   
  psvDebugScreenSetXY(33, 32);
  psvDebugScreenSetTextColor(GREY);
  psvDebugScreenPrintf("...");

}

void gamecard() {
  int x1, x2, x3, x4, y, y2;

  x1 = 3, x2 = 24, x3 = 37, x4 = 58, y = 6, y2 = 0;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Gamecard");
  

  if( getGamecardMounted(retstr) == 0 ) { // game card inserted

    uint8_t paramsfo[512] = "";
    sprintf(paramsfo, "gro0:app/%s/sce_sys/param.sfo", retstr); // id via getGamecardMounted()

    
    /// SFO "TITLE"
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Title:");
    psvDebugScreenSetXY(x2, y);
    ret = getParamValue(retstr, &rawval, paramsfo, "TITLE");
    if( ret >= 0 ) { // success
      removeChar(retstr, 0xC2); // (R)
      removeChar(retstr, 0xAE);

      removeChar(retstr, 0xE2); // TM
      removeChar(retstr, 0x84);
      removeChar(retstr, 0xA2);

      psvDebugScreenPrintf("\"%s\"", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// SFO "TITLE_ID"
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Title ID:");
    psvDebugScreenSetXY(x2, y);
    ret = getParamValue(retstr, &rawval, paramsfo, "TITLE_ID"); // 
    ret2 = getParamValue(retstr2, &rawval2, paramsfo, "VERSION"); // 
    if( ret >= 0 && ret2 >= 0 ) { // success
      psvDebugScreenPrintf("%s (v%s)", retstr, retstr2[0] == '0' ? &retstr2[1] : retstr2); // "01.00" -> "1.00"
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;


    /// SFO "CONTENT_ID"
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Content ID:");
    psvDebugScreenSetXY(x2, y);
    ret = getParamValue(retstr, &rawval, paramsfo, "CONTENT_ID"); // 
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;


    /// gro0
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("gro0:");
    psvDebugScreenSetXY(x2, y);
    ret = getDeviceInfo(retstr, &rawval, "gro0:", 0); // max_size
    ret2 = getDeviceInfo(retstr2, &rawval2, "gro0:", 3); // used
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%u / %u", rawval2, rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s of %s used", retstr2, retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// grw0 (not every card has it)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("grw0:");
    psvDebugScreenSetXY(x2, y);
    ret = getDeviceInfo(retstr, &rawval, "grw0:", 0); // max_size
    ret2 = getDeviceInfo(retstr2, &rawval2, "grw0:", 3); // used
    if( ret >= 0 && ret2 >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("%u / %u", rawval2, rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s of %s used", retstr2, retstr);
      }
    } else psvDebugScreenPrintf("/"); // psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// update on card
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Update PUP:");
    psvDebugScreenSetXY(x2, y);
    ret = getPupVersion(retstr, &rawval, "gro0:psp2/update/psp2updat.pup"); 
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf("/");  //psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;




    y += 2;






    /// CID
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("CID:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 0); // full CID
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// CSD
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("CSD:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 1); // full CSD
    ret2 = getGamecardInfo(retstr2, &rawval2, 2); // ext CSD
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s (rev %s)", retstr, retstr2);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// EXT_CSD
    /*psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("EXT_CSD:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 2); // full 
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;*/

    y += 1;






    y2 = y;

    /// crc7
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("CRC7:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 10); // crc7
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// manufacture_date
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Manufacture Date:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 11); // mdt
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// serial_number
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Serial Number:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 12); // psn
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        if( CENSORED ) {
          psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB");
        } else {
          psvDebugScreenPrintf("%s", retstr);
        }
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;

    /// Product revision
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Product Revision:");
    psvDebugScreenSetXY(x2, y);
    ret = getGamecardInfo(retstr, &rawval, 13); // prv
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;




    /// Product name 
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Product Name:");
    psvDebugScreenSetXY(x4, y2);
    ret = getGamecardInfo(retstr, &rawval, 14); // pnm
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;

    /// OEM/Application ID
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("OEM / AppID:");
    psvDebugScreenSetXY(x4, y2);
    ret = getGamecardInfo(retstr, &rawval, 15); // oid
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;

    /// Device
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Device Type:");
    psvDebugScreenSetXY(x4, y2);
    ret = getGamecardInfo(retstr, &rawval, 16); // cbx
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;

    /// Manufacturer ID
    psvDebugScreenSetXY(x3, y2);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("Manufacturer ID:");
    psvDebugScreenSetXY(x4, y2);
    ret = getGamecardInfo(retstr, &rawval, 17); // mid
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%02X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y2 += 2;

  } else {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS); // COL_WARNG
    psvDebugScreenPrintf("No physical game card inserted!");
    y += 2;
  }

}

void partitions() {
  int i, x1, y;

  x1 = 3, y = 6;
    
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> MBR Partitions");


  /// 
  for(i = 0; i < 0x10; i++) {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);

    ret = getMbrPartitionEntry(retstr, &rawval, i, 0);
    if( ret < 0 || rawval == 0 )  // error or no more entries
      break; 

    psvDebugScreenPrintf("%02d:", i);

    psvDebugScreenSetXY(x1 + 4, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 2); // part_id
    if( RAW ) psvDebugScreenPrintf("0x%02X", rawval);
    else psvDebugScreenPrintf("%s", retstr);

    psvDebugScreenSetXY(x1 + 13, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 1); // n_sectors
    if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
    else psvDebugScreenPrintf("%s", retstr);
    
    psvDebugScreenSetXY(x1 + 26, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 0); // start_lba
    if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
    else psvDebugScreenPrintf("%s", retstr);

    psvDebugScreenSetXY(x1 + 37, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 3); // part_type
    if( RAW ) psvDebugScreenPrintf("0x%02X", rawval);
    else psvDebugScreenPrintf("%s", retstr);

    psvDebugScreenSetXY(x1 + 46, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 4); // part_flag
    if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
    else psvDebugScreenPrintf("%s", retstr);

    psvDebugScreenSetXY(x1 + 57, y);
    ret = getMbrPartitionEntry(retstr, &rawval, i, 5); // acl
    if( RAW ) psvDebugScreenPrintf("0x%04X", rawval);
    else psvDebugScreenPrintf("%s", retstr);
    y += 2;

  } y += 1;


  /// 
  /*psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("test:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", warning(-1, "TODO"));
  y += 2;*/

  
  
  //https://github.com/SKGleba/PSP2Info/blob/master/main.c#L189
  //https://wiki.henkaku.xyz/vita/SceIofilemgr#Mount_Points
  
  
}

void livearea() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
    
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Livearea & Settings");


  /// installed Apps Count
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Installed Apps:");
  psvDebugScreenSetXY(x2, y);
  ret = getUserApps(retstr, &rawval); 
  if( ret >= 0 )
    psvDebugScreenPrintf("%d", rawval);
  else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// installed Themes Count
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Installed Themes:");
  psvDebugScreenSetXY(x2, y);
  ret = getUserThemes(retstr, &rawval); 
  if( ret >= 0 )
    psvDebugScreenPrintf("%d", rawval);
  else psvDebugScreenPrintf("0");  
  y += 3;  
  
  /// liveareapages, plugins, photo / screenshots, videos, music .. trophies




  /// Wallpaper
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Wallpaper:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/THEME", "wallpaper"));
  y += 2;  
  
  /// Current Theme
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Current Theme:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/THEME/", "current_theme"));
  y += 2;  


  /// Default Wave Color
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Default Wave Color:");
  psvDebugScreenSetXY(x2, y);
  ret = getWaveColor(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Preinstalled Wave
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Preinstalled Wave:");
  psvDebugScreenSetXY(x2, y);
  ret = getPreinstWave(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Preinstalled Theme
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Preinstalled Theme:");
  psvDebugScreenSetXY(x2, y);
  ret = getPreinstTheme(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;




  /// Enter Button
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Enter Button:");
  psvDebugScreenSetXY(x2, y);
  ret = getEnterButton(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  


  /// NVS - Language (0x4A4)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Language:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsLanguage(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%u", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } y += 2;


  /// Brightness
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Brightness Level:");
  psvDebugScreenSetXY(x2, y);
  if( getRegistryIntegerNow_("/CONFIG/DISPLAY", "brightness") > 0 ) { // error check 
    if( !RAW ) {
      psvDebugScreenPrintf("%d%%", getRegistryIntegerNow_("/CONFIG/DISPLAY", "brightness") * 100 / 65536);
    } else psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/DISPLAY", "brightness"));
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Volume
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Volume Level:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s/30",  getRegistryIntegerNow("/CONFIG/SOUND", "main_volume"));
  y += 2;  

  /// AVLS
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("AVLS:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s",  getRegistryBooleanNow("/CONFIG/SOUND", "avls")); // 
  y += 2;  

  /// Auto AVLS
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Auto AVLS:");
  psvDebugScreenSetXY(x2, y);
  ret = getAutoAvls(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  
  
}      

void registry() {
  int x1, x2, y;

  x1 = 3, x2 = 44, y = 6;
    
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Registry");


  

  if( vshSblAimgrIsTool() == 1 ) { // Devkit

    /// Devkit - Name (Target name in Neighborhood. Can be empty though!)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 /DEVENV/HOST/devkitname:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryStringNow("/DEVENV/HOST", "devkitname"));
    y += 2;  
  }

  if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // Test or Devkit  
    /// Devkit - Core Dump Level (0 = Mini Dump, 1 = Full Dump)
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 /CONFIG/COREDUMP/dump_level:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/COREDUMP", "dump_level"));
    y += 2;  
    
    /// Fake 3G Interface
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 /CONFIG/NET/fake_3g_if:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/NET", "fake_3g_if"));
    y += 3;  

  }

  
  /// Language
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/SYSTEM/language:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/SYSTEM/", "language"));
  y += 2;  

   /// Debug 3G (disables 3g features if on!)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/TEL/use_debug_settings:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/TEL/", "use_debug_settings"));
  y += 2;  
  
  /// USB Charge
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/USB/usb_charge_enable:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryBooleanNow("/CONFIG/USB", "usb_charge_enable"));
  y += 2;  

  /// Last Location (default is sony hq in Tokyo :D) 35.63127 139.74331
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/LOCATION/last_latitude:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED ) { 
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB"); // XXX.XXXXX
  } else {
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/LOCATION/", "last_latitude"));
  } y += 2;  
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/LOCATION/last_longitude:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED ) { 
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB"); // XXX.XXXXX
  } else {
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/LOCATION/", "last_longitude"));
  } y += 2;  

  /// PS4 Link Counter
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/PS4LINK/counter:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/PS4LINK/", "counter"));
  y += 2;  

  /// Parental Passcode
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/SECURITY/PARENTAL/passcode:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB");
  else 
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/SECURITY/PARENTAL/", "passcode"));
  y += 2;  

  /// Lockscreen Passcode
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/SECURITY/SCREEN_LOCK/passcode:");
  psvDebugScreenSetXY(x2, y);
  if( CENSORED )
    psvDebugScreenPrintf("\xDB\xDB\xDB\xDB");
  else 
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/SECURITY/SCREEN_LOCK/", "passcode"));
  y += 2;  

  if( vshSblAimgrIsTool() == 0 ) { // not Devkit

    /// psp_first_boot
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/PSPEMU/psp_first_boot:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/PSPEMU/", "psp_first_boot"));
    y += 2;  

    /// ps1_first_boot
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/PSPEMU/ps1_first_boot:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/PSPEMU/", "ps1_first_boot"));
    y += 2;  

    /// pocketstation_first_boot
    /* psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/PSPEMU/pocketstation_first_boot:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/PSPEMU/", "pocketstation_first_boot"));
    y += 2;  */

  }

  /// suspend interval
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/POWER_SAVING/suspend_interval:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s sec", getRegistryIntegerNow("/CONFIG/POWER_SAVING/", "suspend_interval"));
  y += 2;

  /// controller_off_interval
  if( vshSblAimgrIsDolce() == 1 ) {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/POWER_SAVING/contr._off_int.:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s sec", getRegistryIntegerNow("/CONFIG/POWER_SAVING/", "controller_off_interval"));
    y += 2;  
  }

  /// Dimming interval
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("/CONFIG/DISPLAY/dimming_interval:");
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenPrintf("%s sec", getRegistryIntegerNow("/CONFIG/DISPLAY/", "dimming_interval"));
  y += 2;  


  if( vshSysconIsIduMode() == 1 ) {
    y += 1;

    /// idu_video_latency
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/SPECIFIC/idu_video_latency:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryIntegerNow("/CONFIG/SPECIFIC/", "idu_video_latency"));
    y += 2;

    /// contents_dl_url
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("/CONFIG/SPECIFIC/contents_dl_url:");
    psvDebugScreenSetXY(x2, y);
    psvDebugScreenPrintf("%s", getRegistryStringNow("/CONFIG/SPECIFIC/", "contents_dl_url"));
    y += 2;
   
  }

}

void playlog() {
  int x1, y;

  x1 = 3, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Playlog");
      
  if( doesFileExist("ur0:user/00/shell/playlog/playlog.dat") == 0 ) { 
  
    int i = 0;
    do {  
      ret = getPlaylogEntry(retstr, &rawval, i, 0);
      if( ret >= 0 ) { // success
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("%02d:", i);

        /// title_id
        psvDebugScreenSetXY(x1 + 4, y);
        psvDebugScreenPrintf("%s", retstr);

        /// timestamp
        ret = getPlaylogEntry(retstr, &rawval, i, 1);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 15, y);
          psvDebugScreenPrintf("%s", retstr);
        }

        /// ?
        /*ret = getPlaylogEntry(retstr, &rawval, i, 2);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 33, y);
          //if( RAW ) { // raw
            psvDebugScreenPrintf("0x%X", rawval);
          //} else { // print nice
          //  psvDebugScreenPrintf("%s", retstr);
          //}
        }*/

        /// some normalized activity float (0.1 to ?) seen 6.9 for example   the higher the more the app was used recently
        ret = getPlaylogEntry(retstr, &rawval, i, 3);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 33, y);
          if( RAW ) { // raw
            psvDebugScreenPrintf("0x%X", rawval);
          } else { // print nice
            psvDebugScreenPrintf("%s", retstr);
          }
        }

        /// open app counter?
        ret = getPlaylogEntry(retstr, &rawval, i, 4);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 43, y);
          psvDebugScreenPrintf("%d", rawval);
        }

        /// button presses
        ret = getPlaylogEntry(retstr, &rawval, i, 5);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 48, y);
          psvDebugScreenPrintf("%d", rawval);
        }

        /// some float (0.0 to 1.0)
        ret = getPlaylogEntry(retstr, &rawval, i, 6);
        if( ret >= 0 ) { // success
          psvDebugScreenSetTextColor(COL_TEXTS);
          psvDebugScreenSetXY(x1 + 54, y);
          if( RAW ) { // raw
            psvDebugScreenPrintf("0x%X", rawval);
          } else { // print nice
            psvDebugScreenPrintf("%s", retstr);
          }
        }

        y += 2; // <-- make it 1 for 24 lines

      } i++;
  
    } while( ret >= 0 && y <= 31 ); // 12 items max on screen with two rows
    
    if( y >= 31 ) {
      psvDebugScreenSetXY(33, 32); 
      psvDebugScreenSetTextColor(GREY);
      psvDebugScreenPrintf("...");
    }

  } else {
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS); // COL_WARNG
    psvDebugScreenPrintf("No playlog file found!");
    y += 2;
  }

}
 
void clocks() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Clocks");
  

  /// ARM
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("CPU frequency:");
  psvDebugScreenSetXY(x2, y);
  getClockArm(retstr, &rawval);
  if( RAW ) { // raw
    psvDebugScreenPrintf("%X", rawval);
  } else { // print nice
    psvDebugScreenPrintf("%s", retstr);
  }
  y += 3;  


  /// Bus
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Bus frequency:");
  psvDebugScreenSetXY(x2, y);
  getClockBus(retstr, &rawval);
  if( RAW ) { // raw
    psvDebugScreenPrintf("%X", rawval);
  } else { // print nice
    psvDebugScreenPrintf("%s", retstr);
  }
  y += 3;  


  /// GPU
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("GPU frequency:");
  psvDebugScreenSetXY(x2, y);
  getClockGpu(retstr, &rawval);
  if( RAW ) { // raw
    psvDebugScreenPrintf("%X", rawval);
  } else { // print nice
    psvDebugScreenPrintf("%s", retstr);
  }
  y += 3;  

  /// GPU xbar
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("GPU xbar frequency:"); // Crossbar
  psvDebugScreenSetXY(x2, y);
  getClockGpuXbar(retstr, &rawval);
  if( RAW ) { // raw
    psvDebugScreenPrintf("%X", rawval);
  } else { // print nice
    psvDebugScreenPrintf("%s", retstr);
  }
  y += 3;  


  /// timebase
  /* psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Timebase frequency:");
  psvDebugScreenSetXY(x2, y);
  getClockTimebase(retstr, &rawval);
  if( RAW ) { // raw
    psvDebugScreenPrintf("%X", rawval);
  } else { // print nice
    psvDebugScreenPrintf("%s", retstr);
  }
  y += 3;  */


  /// more?
  
}

void dipswitches() { 
  int x1, x2, x3, x4, y, y2;

  x1 = 3, x2 = 30, x3 = 37, x4 = 64, y = 6, y2 = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> DIP Switch Flags");

  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("\x80 Debug Control (192-223)"); // https://wiki.henkaku.xyz/vita/KBL_Param#DIP_Switches
  y+=2;

  /// 192
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Dmac6:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(192) == TRUE ? "ON" : "OFF");
  
  /// 193
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable SDbgSdio:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(193) == TRUE ? "ON" : "OFF");
  
  /// 194
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable CP Modules:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(194) == TRUE ? "ON" : "OFF");
  
  /// 195
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Disable USB Debug:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(195) == TRUE ? "ON" : "OFF");
  
  /// 196
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Kernel UART0:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(196) == TRUE ? "ON" : "OFF");
   
  /// 197
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Kernel UART1:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(197) == TRUE ? "ON" : "OFF");
  
  /// 198
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable System TTY:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(198) == TRUE ? "ON" : "OFF");
  
  /// 199
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable TTY stdio:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(199) == TRUE ? "ON" : "OFF");
  
  /// 200
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Stop when assert fails:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(200) == TRUE ? "ON" : "OFF");
  
  /// 201
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Assert Level 1:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(201) == TRUE ? "ON" : "OFF");
  
  /// 202
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Assert Level 2:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(202) == TRUE ? "ON" : "OFF");
  
  /// 203
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Disable Dipsw Re-Config:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(203) == TRUE ? "ON" : "OFF");
  
  /// 204
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Default Debug Level 1:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(204) == TRUE ? "ON" : "OFF");
  
  /// 205
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Default Debug Level 2:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(205) == TRUE ? "ON" : "OFF");
  
  /// 206
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Allow syscall debug:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(206) == TRUE ? "ON" : "OFF");
  


  /// 210
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("ENABLE_TOOL_PHYMEMPART:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(210) == TRUE ? "ON" : "OFF");
  
  /// 211
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Usermode UART logging:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(211) == TRUE ? "ON" : "OFF");
  
  /// 212
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Usermode PA memory map:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(212) == TRUE ? "ON" : "OFF");
  
  /// 213
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Tiny PA memory Range:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(213) == TRUE ? "ON" : "OFF");
  
  /// 214
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Disable ASLR:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(214) == TRUE ? "ON" : "OFF");
  
  /// 215
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Disable SysDbg Trace:"); // "Disable System Debug Process Trace"
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(215) == TRUE ? "ON" : "OFF");
  
  /// 216
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Wipe kernel stack by 0xFF:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(216) == TRUE ? "ON" : "OFF");
  
  /// 217
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Enable Path Logging:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(217) == TRUE ? "ON" : "OFF");
  
  /// 218
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("Ignore app keystone error:");
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(218) == TRUE ? "ON" : "OFF");
  


  /// 222
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("KBL MemTest ScratchPad:"); // "Enable KBL Simple Memory Test over ScePowerScratchPad32KiB"
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(222) == TRUE ? "ON" : "OFF");
  
  /// 223
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenPrintf("KBL MemTest Secure DRAM:"); // "Enable KBL Simple Memory Test over Secure DRAM"
  psvDebugScreenSetXY(x2, y++);
  psvDebugScreenPrintf(GetDipsw(223) == TRUE ? "ON" : "OFF");
  



  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  

  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("\x80 Shell Flags (160-191)"); // https://wiki.henkaku.xyz/vita/KBL_Param#DIP_Switches
  y2+=2;

  /// 168
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Memory Size:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(168) == TRUE ? "ON" : "OFF");

  /// 184
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Extra TTY:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(184) == TRUE ? "ON" : "OFF");

  /// 185
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("System Boot Time Notif.:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(185) == TRUE ? "ON" : "OFF");

  /// 187
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Thread All Core CPU Aff:");  // Allow processes to run on all cores (CPU affinity)
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(187) == TRUE ? "ON" : "OFF");

  /// 190
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Disable ShellDummy PA:");  // Disable to ScePhyMemPartShellDummy with PA 0x78000000 0x8000000-bytes. 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(190) == TRUE ? "ON" : "OFF");

  /// 191
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Grow PhyMemPart with PA:");  // Grow PhyMemPart with PA 0x78000000 0x8000000-bytes. Shell += 48MiB, Shared += 80MiB. 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(191) == TRUE ? "ON" : "OFF");


  y2+=3;

  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  ////  

  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("\x80 System Control (224-255)"); // https://wiki.henkaku.xyz/vita/KBL_Param#DIP_Switches
  y2+=2;

  /// 224
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("System Development Mode:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(224) == TRUE ? "ON" : "OFF");
  
  /// 225
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("L2 Cache Disabled: ?"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(225) == TRUE ? "ON" : "OFF");
  
  /// 228
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Hardware break point:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(228) == TRUE ? "ON" : "OFF");
  
  /// 229
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("HDCP:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(229) == TRUE ? "ON" : "OFF");
  
  /// 230
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Init sd0 and ur0:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(230) == TRUE ? "ON" : "OFF");
  
  /// 231
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Init os0:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(231) == TRUE ? "ON" : "OFF");
  
  /// 236
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("GPU Overclock to 166MHz:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(236) == TRUE ? "ON" : "OFF");
  
  /// 238
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Underclock to 111MHz: ?"); // is it BUS?
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(238) == TRUE ? "ON" : "OFF");
   
  /// 240
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Disable QAFlags:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(240) == TRUE ? "ON" : "OFF");
  
  /// 241
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Disable QAFlags 0xD/0xE:"); // Disable QA flags 0xD mask 1 and 0xE mask 1
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(241) == TRUE ? "ON" : "OFF");
  
  /// 250
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Enable tty0:"); 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(250) == TRUE ? "ON" : "OFF");
  
  /// 251
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Allow SysModule Debug:"); // Enable "dummytty0:". Also allow sysmodule load from host0:
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(251) == TRUE ? "ON" : "OFF");
  
  /// 252
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("Allow Host Module:"); // Allow host0: access
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(252) == TRUE ? "ON" : "OFF");
  
  /// 253
  psvDebugScreenSetXY(x3, y2);
  psvDebugScreenPrintf("PowerCtrl Console Log:"); // Related to remote power control 
  psvDebugScreenSetXY(x4, y2++);
  psvDebugScreenPrintf(GetDipsw(253) == TRUE ? "ON" : "OFF");
  
  
}

void qaflags() {
  int x1, x2, y;

  x1 = 3, x2 = 18, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> QA Flags");

  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);


  /// Flags (via kblparam)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Flags:");
  psvDebugScreenSetXY(x2, y);
  ret = getQAFlags(retstr, rawstr);
  if( ret >= 0 ) { // success
    psvDebugScreenPrintf("%s", retstr);
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  
  
  /// Token Template Name
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Template:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsQaf(retstr, &rawval);
  if( ret >= 0 && rawval == 0x00 ) { // theres an active token
    ret = getQATokenName(retstr); 
    if( ret >= 0 ) { // success
      psvDebugScreenPrintf("%s", retstr);
    } else { // sceSblQafMgr failed so something is off
      ret2 = getQATokenNameViaNvs(retstr2); 
      if( ret2 >= 0 ) { // but there is one in NVS though!
        psvDebugScreenSetTextColor(COL_WARNG);
        psvDebugScreenPrintf("%s (inactive)", retstr2);
      } else psvDebugScreenPrintf("/");
    } 
  } else psvDebugScreenPrintf("NO_FLAGS");
  y += 2;

  /// NVS - QAF Token Flag (0x480)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Token Status:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsQaf(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%02X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;
  

  /// https://wiki.henkaku.xyz/vita/KBL_Param#QA_flags
  psvDebugScreenSetTextColor(COL_TEXTS);


  // Byte 0x0 - Mask 0x1
  if( GetQAFlag(0x0, 0x1) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Skip System Update version check on CEX");
    y++;
  }

  // Byte 0x0 - Mask 0x10
  if( GetQAFlag(0x0, 0x10) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Finalized Retail PKG on None CEX without StoreFlag");
    y++;
  }

  // Byte 0x6 - Mask 0x2
  if( GetQAFlag(0x6, 0x2) == SCE_TRUE ) { 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow full features for QA"); 
  /*
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Dtcp Ip Reset"); // ksceSblQafMgrIsAllowDtcpIpReset()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow NearTest"); // ksceSblQafMgrIsAllowNearTest()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow PSPEmuShowQAInfo"); // ksceSblQafMgrIsAllowPSPEmuShowQAInfo()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Fake AC Install");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Show Title Upgrade Info");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Liblocation Change Model on CEX"); 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow ScreenShot Always"); // sceSblQafMgrIsAllowScreenShotAlways()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Limited Debug Menu Display"); // sceSblQafMgrIsAllowLimitedDebugMenuDisplay()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Np Full Test"); // sceSblQafMgrIsAllowNpFullTest()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Control Idu Auto Update"); // ksceSblQafMgrIsAllowControlIduAutoUpdate()
    */ y++;
  }

  // Byte 0xB - Mask 0x4
  if( GetQAFlag(0xB, 0x4) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow App debug with alt Keyset");
  /* 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Use alternate keyset e.g. QA SPSFO key/NpDrm app key");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow ignore app keystone error");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow User App Debug");
    */ y++;
  }

  // Byte 0xB - Mask 0x10
  if( GetQAFlag(0xB, 0x10) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow MagicGate");
  /* 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Bypass platform requirement for loading fSELF with att 128");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow EMPR via vshSblQafMgrIsAllowLoadMagicGate");
    */ y++;
  }

  // Byte 0xC - Mask 0x2
  if( GetQAFlag(0xC, 0x2) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Remote Play Debug");
  /* 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow All Debug Menu Display"); // sceSblQafMgrIsAllowAllDebugMenuDisplay() 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow RemotePlayDebug"); // ksceSblQafMgrIsAllowRemotePlayDebug()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow sceSblACMgrIsSystem bypass");
    */ y++;
  }

  // Byte 0xC - Mask 0x4
  if( GetQAFlag(0xC, 0x4) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow ePSP on TOOL & skip Updater version checks");
  /* 
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Skip version checks in system updates");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow compat_sm operation on DevKit (can be use PspEmu)");
    */ y++;
  }

  // Byte 0xD - Mask 0x1 "Allow Kernel Debug"
  if( GetQAFlag(0xD, 0x1) == SCE_TRUE ) { // sceSblQafManagerIsAllowKernelDebugForUser()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Kernel Debug"); 
    y++;
  }

  // Byte 0xD - Mask 0x2 "Allow System Debug"
  if( GetQAFlag(0xD, 0x2) == SCE_TRUE ) { // sceSblQafMgrIsAllowRemoteSysmoduleLoad(), ksceSblQafMgrIsAllowHost0Access(), ksceSblQafMgrIsAllowMarlinTest(), ksceSblQafMgrIsAllowSystemAppDebug()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow System Debug");
    y++;
  }

  // Byte 0xE - Mask 0x1 "Allow Module Debug"
  if( GetQAFlag(0xE, 0x1) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Module Debug");
    y++;
  }

  // Byte 0xF - Mask 0x1
  if( GetQAFlag(0xF, 0x1) == SCE_TRUE ) {
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Mini features for QA");
  /*
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow SELF attribute");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Enable DMAC5 keyset for 0x10001 ");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow QAUpdate");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow Minimum Debug Menu Display"); // sceSblQafMgrIsAllowMinimumDebugMenuDisplay()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow Non QAPup on CEX"); // sceSblQafMgrIsAllowNonQAPup()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow Debug DRM Loose Bind");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow Np Test"); // sceSblQafMgrIsAllowNpTest()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow more registry keys");
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("Allow Keep CoreFile"); // ksceSblQafMgrIsAllowKeepCoreFile()
    */ y++;
  }

  // Byte 0xF - Mask 0x2 "Allow Force Update"
  if( GetQAFlag(0xF, 0x2) == SCE_TRUE ) { // sceSblQafMgrIsAllowForceUpdate()
    psvDebugScreenSetXY(x1, ++y);
    psvDebugScreenPrintf("> Allow Force Update"); 
    y++;
  }


  /*
  SceSblQafMgr
    int sceSblQafMgrIsAllowMinimumDebugMenuDisplay(void);
    int sceSblQafMgrIsAllowLimitedDebugMenuDisplay(void);
    int sceSblQafMgrIsAllowAllDebugMenuDisplay(void);
    int sceSblQafManagerIsAllowKernelDebugForUser(void);
    int sceSblQafMgrIsAllowForceUpdate(void);
    int sceSblQafMgrIsAllowNpTest(int a1, int a2, int a3);
    int sceSblQafMgrIsAllowNpFullTest(void);
    int sceSblQafMgrIsAllowNonQAPup(void);
    int sceSblQafMgrIsAllowScreenShotAlways(void);
    int sceSblQafMgrIsAllowRemoteSysmoduleLoad(void);

  SceQafMgrForDriver
    ksceSblQafMgrIsAllowControlIduAutoUpdate: 0xF8BFEE48
    ksceSblQafMgrIsAllowDecryptedBootConfigLoad: 0x883E9465
    ksceSblQafMgrIsAllowDtcpIpReset: 0xE8B8F31F
    ksceSblQafMgrIsAllowHost0Access: 0x082A4FC2
    ksceSblQafMgrIsAllowKeepCoreFile: 0xC1EA75C8
    ksceSblQafMgrIsAllowLoadMagicGate: 0x36E5312E
    ksceSblQafMgrIsAllowMarlinTest: 0x10283EB8
    ksceSblQafMgrIsAllowNearTest: 0x9644171D
    ksceSblQafMgrIsAllowPSPEmuShowQAInfo: 0xB7B195B2
    ksceSblQafMgrIsAllowRemotePlayDebug: 0xBFD5E463
    ksceSblQafMgrIsAllowSystemAppDebug: 0xCAD47130
  */

}

void modules() {
  int i, x1, /*x2,*/ y;

  x1 = 3, /*x2 = 37,*/ y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Loaded Kernel Modules (None-SCE)");
  
  i = 0;
  do {  
    ret = getModuleInfo(0, i, 3, retstr, &rawval); // modname
    if( ret >= 0 && (retstr[0] != 'S' && retstr[1] != 'c' && retstr[2] != 'e') ) { 
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("%02d:", i);

      /// modname
      psvDebugScreenSetXY(x1 + 4, y);
      ret = getModuleInfo(0, i, 3, retstr, &rawval);
      psvDebugScreenPrintf("%s", retstr);
      
      /// modver
      psvDebugScreenSetXY(x1 + 25, y);
      ret = getModuleInfo(0, i, 2, retstr, &rawval);
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
      else psvDebugScreenPrintf("v%s", retstr);
      
      /// modid
      psvDebugScreenSetXY(x1 + 32, y);
      ret = getModuleInfo(0, i, 1, retstr, &rawval);
      psvDebugScreenPrintf("0x%X", rawval);

      /// size
      psvDebugScreenSetXY(x1 + 43, y);
      ret = getModuleInfo(0, i, 0, retstr, &rawval);
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
      else psvDebugScreenPrintf("%s", retstr);

      /// state
      psvDebugScreenSetXY(x1 + 55, y);
      ret = getModuleInfo(0, i, 5, retstr, &rawval);
      if( RAW ) psvDebugScreenPrintf("0x%X", rawval);
      else psvDebugScreenPrintf("%s", retstr);


      y += 2; // <-- make it 1 for more lines

    } i++;
  
  } while( ret >= 0 && y <= 31 ); // 12 items max on screen with two rows

  if( y >= 31 ) {
    psvDebugScreenSetXY(33, 32);
    psvDebugScreenSetTextColor(GREY); 
    psvDebugScreenPrintf("...");
  }


  /*
  /// User modules
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("User: (None-SCE only)");
  y = 9; i = 0;
  do {  
    ret = getModuleInfo(1, i, retstr, &rawval);
    if( ret >= 0 && (retstr[0] != 'S' && retstr[1] != 'c' && retstr[2] != 'e') ) { 
      psvDebugScreenSetXY(x1, y);
      //psvDebugScreenSetTextColor(COL_TEXTS);
      //psvDebugScreenPrintf("%02d:", i);

      /// modname
      //psvDebugScreenSetXY(x1 + 4, y);
      psvDebugScreenPrintf(" %s [0x%X]", retstr, rawval);

      y += 2; // <-- make it 1 for more lines

    } i++;
  
  } while( ret >= 0 && y <= 31 ); // 10 items max on screen with two rows 
  

  y = 6; // reset for second column


  /// Kernel modules
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Kernel: (None-SCE only)");
  y = 9; i = 0;
  do {  
    ret = getModuleInfo(0, i, retstr, &rawval);
    if( ret >= 0 && (retstr[0] != 'S' && retstr[1] != 'c' && retstr[2] != 'e') ) { 
      psvDebugScreenSetXY(x2, y);
      //psvDebugScreenSetTextColor(COL_TEXTS);
      //psvDebugScreenPrintf("%02d:", i);

      /// modname
      //psvDebugScreenSetXY(x2 + 4, y);
      psvDebugScreenPrintf(" %s [0x%X]", retstr, rawval);

      y += 2; // <-- make it 1 for more lines

    } i++;
  
  } while( ret >= 0 && y <= 31 ); // 10 items max on screen with two rows
  */
}

void miscellaneous() {
  int x1, x2, y;

  x1 = 3, x2 = 24, y = 6;
  
  psvDebugScreenSetXY(2, 3);
  psvDebugScreenSetTextColor(COL_CATEG);
  psvDebugScreenPrintf("> Miscellaneous");
  
  /// Refurbished
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Refurbished:");
  psvDebugScreenSetXY(x2, y);
  ret = getRefurbished(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Diagnostics Software / Factory tests  https://wiki.henkaku.xyz/vita/IdStorage#0x104
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Diagnosed:");
  psvDebugScreenSetXY(x2, y);
  ret = getDiagnostics(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%d", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  
  /// CP Timestamp
  if( vshSblAimgrIsTool() == 1 ) { // DEVKIT
    psvDebugScreenSetXY(x1, y);
    psvDebugScreenSetTextColor(COL_TEXTS);
    psvDebugScreenPrintf("\x80 CP Timestamp:");
    psvDebugScreenSetXY(x2, y);
    ret = getCpTimestamp(retstr, &rawval); 
    if( ret >= 0 ) { // success
      if( RAW ) { // raw
        psvDebugScreenPrintf("0x%08X", rawval);
      } else { // print nice
        psvDebugScreenPrintf("%s", retstr);
      }
    } else psvDebugScreenPrintf(error(ret, "ERROR"));
    y += 2;
  }

  y += 1;  



  /// Enso installed
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Enso installed:");
  psvDebugScreenSetXY(x2, y);
  ret = getEnso(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("%X", rawval);
    } else { // print nice
      ret2 = getEnsoUninstalled(retstr2, &rawval2); // WAS it ever installed though?
      if( ret2 >= 0 && rawval == FALSE && rawval2 == TRUE) { // success
        psvDebugScreenPrintf("%s (but it was)", retstr); // FALSE
      } else psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;
  
  


  /// Codename
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Codename:");
  psvDebugScreenSetXY(x2, y);
  ret = getCodename(retstr, &rawval); 
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("\"%s\"", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;




  /// Boot Type Indicator 1
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("BootType Indicator:");
  psvDebugScreenSetXY(x2, y);
  ret = getBootTypeIndicator1(retstr, &rawval);
  if( ret >= 0 ) { // success
    //if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    //} else { // print nice
    //  psvDebugScreenPrintf("%s", retstr);
    //}
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  


  /// WakeupReq / Sleep Factor
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Sleep Factor:");
  psvDebugScreenSetXY(x2, y);
  ret = getWakeupReqFactor(retstr, &rawval);
  if( ret >= 0 ) { // success
    //if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    //} else { // print nice
    //  psvDebugScreenPrintf("%s", retstr);
    //}
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;  

  /// Wakeup Factor
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Wakeup Factor:");
  psvDebugScreenSetXY(x2, y);
  ret = getWakeupFactor(retstr, &rawval);
  if( ret >= 0 ) { // success
    //if( RAW ) { // raw
      psvDebugScreenPrintf("0x%X", rawval);
    //} else { // print nice
    //  psvDebugScreenPrintf("%s", retstr);
    //}
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 3;  






  /// NVS - Extra UART Flag (0x481)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("NVS Extra UART:");
  psvDebugScreenSetXY(x2, y);
  ret = getNvsExtraUart(retstr, &rawval);
  if( ret >= 0 ) { // success
    if( RAW ) { // raw
      psvDebugScreenPrintf("0x%02X", rawval);
    } else { // print nice
      psvDebugScreenPrintf("%s", retstr);
    }
  } else psvDebugScreenPrintf(error(ret, "ERROR"));
  y += 2;



}

void summary() {
  int i, x1, y;

  x1 = 3, y = 6;
  
  //psvDebugScreenSetXY(2, 3);
  //psvDebugScreenSetTextColor(COL_CATEG);
  //psvDebugScreenPrintf("> Summary & Anomalies");
  
  //int COL_UNIQUE = 0xFF000000 + uniquevar(); // some device unique color (color preset would be cooler? but what about black)
  int COL_UNIQUE = COL_CATEG;

  y = 4;

  /// MESSAGE TO OWNER
  psvDebugScreenSetTextColor(COL_TEXTS);
  if( kernmode ) { // only when helper plugin loaded

    psvDebugScreenSetXY(x1, y);
    ret = getSystemUsername(retstr, rawstr);
    if( ret >= 0 ) {
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("Hi ");
      psvDebugScreenSetTextColor(COL_UNIQUE);
      psvDebugScreenPrintf("%s", retstr);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf(",");
      y+=2;

      psvDebugScreenSetXY(x1, y);
      ret = getModelName(retstr, rawstr);
      if( ret >= 0 ) {
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("this is a ");
        psvDebugScreenSetTextColor(COL_UNIQUE);
        psvDebugScreenPrintf("%s ", retstr);
      }
      
      ret = getCurrentFirmware(1, retstr, &rawval); // os0
      ret2 = getCurrentFirmware(2, retstr2, &rawval2); // index.dat
      if( ret >= 0 ) {
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("running firmware ");
        psvDebugScreenSetTextColor(COL_UNIQUE);
        if( ret2 >= 0 && (vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1) ) // Developer firmware - print full
          psvDebugScreenPrintf("%s", retstr2); // X.YYY.ZZZ
        else  // normal CEX firmware (display via os0)
          psvDebugScreenPrintf("%s", retstr); // X.YY
      }

      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf(" produced");
      y+=2;


      psvDebugScreenSetXY(x1, y);

      memset(retstr, 0, 256);
      //strncpy(retstr, getSmiString1(9) + 7, 4); // it ALWAYS has been "AA.BBB_YYYYMMDD" since 0.945 but then they made "03.60_20160404_01_E" .. -.-
      strncpy(retstr, strchr(getSmiString1(9), '_') + 1, 4);
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("in ");
      psvDebugScreenSetTextColor(COL_UNIQUE);
      psvDebugScreenPrintf("%s ", retstr);
      

      ret = getTarget(1, retstr, &rawval); // not via SblAimgr since it could be spoofed!
      if( ret >= 0 ) {
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("for ");
        psvDebugScreenSetTextColor(COL_UNIQUE);
        switch(rawval) {
          case 0x100:
            sprintf(retstr, "internal Testing");
            break;
            
          case 0x101: // Devkit
            sprintf(retstr, "Development");
            break;

          case 0x102: // Testkit
            sprintf(retstr, "Testing");
            break;
          
          case 0x103:  case 0x104:  case 0x105:  case 0x106:
          case 0x107:  case 0x108: case 0x109:  case 0x10A:
          case 0x10B:  case 0x10C:  case 0x10D:  case 0x10E:
          case 0x10F:  case 0x110:  case 0x111: 
            sprintf(retstr, "Consumers");
            break;

          default: sprintf(retstr, "error");
        }
        psvDebugScreenPrintf("%s ", retstr);
      }

      ret = getRegion(1, retstr, &rawval); // not via SblAimgr since it could be spoofed
      if( ret >= 0 ) {
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("in the ");
        psvDebugScreenSetTextColor(COL_UNIQUE);
        switch(rawval) {
          case 0x101: case 0x102: // Devkit / Testkit
            getRegionForDev(retstr, &rawval);
            break;
          
          case 0x103: sprintf(retstr, "Japanese"); break; // 0
          case 0x104: sprintf(retstr, "North American"); break;  // 1 
          case 0x105: sprintf(retstr, "European"); break; // 4
          case 0x106: sprintf(retstr, "South Korean"); break; // 5
          case 0x107: sprintf(retstr, "United Kingdom"); break; // 3
          case 0x108: sprintf(retstr, "Mexican"); break;
          case 0x109: sprintf(retstr, "Australian/New Zealand"); break;
          case 0x10A: sprintf(retstr, "Asian"); break; // 6
          case 0x10B: sprintf(retstr, "Taiwan"); break; //7
          case 0x10C: sprintf(retstr, "Russian"); break; //8
          case 0x10D: sprintf(retstr, "Chinese"); break;
          case 0x10E: sprintf(retstr, "Hong Kong"); break;

          default: sprintf(retstr, "error");
        }  
        psvDebugScreenPrintf("%s ", retstr);
      }
      
      psvDebugScreenSetTextColor(COL_TEXTS);
      psvDebugScreenPrintf("region!");
      y+=2;



      psvDebugScreenSetXY(x1, y);
      ret = getCodename(retstr, &rawval);
      if( ret >= 0 ) {
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf("Call me ");
        psvDebugScreenSetTextColor(COL_UNIQUE);
        psvDebugScreenPrintf("%s", retstr);
        psvDebugScreenSetTextColor(COL_TEXTS);
        psvDebugScreenPrintf(", ..Gandalf!?"); // more like Jim Henson. "Hi-Ho and welcome to the Muppet Show, xxxx!"
        y+=2;
      }

      //psvDebugScreenSetXY(x1, y);
      //psvDebugScreenPrintf("Cheers!"); // See ya!
      //y+=2;

      y+=1;
    }

  }


  /// INFO (everything that makes this unit special)
  psvDebugScreenSetTextColor(MAGENTA);

    /// Prototype
    ret = getSerial(retstr, rawstr); 
    if( ret >= 0 ) {
      if( (rawstr[0] == 'T' && rawstr[1] == 'G') || (rawstr[1] == 'T' && rawstr[2] == 'G') ) { // either "00-TG.." or "03-TG.."
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This Unit is a Prototype!");
        y+=2;
      }
    }    

    /// Refurbished
    ret = getRefurbished(retstr, &rawval);
    if( ret >= 0 && rawval == TRUE ) { 
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenPrintf("This Unit was factory refurbished by Sony!");
      y += 2;  
    }

    /// Internal Firmware 
    ret = getFirmwareInternal(retstr, &rawval); 
    if( ret >= 0 ) {
      if( rawval == TRUE ) {
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This Unit is currently running an internal Firmware!");
        y+=2;
      }
    }
    
    /// QA Token
    ret = getNvsQaf(retstr, &rawval); // just because the flag is on doesn't mean its active. (internal token on external firmware is inactive for example)
    if( ret >= 0 ) {
      if( rawval == 0x00 ) { // enabled flag
        psvDebugScreenSetXY(x1, y);
        ret2 = getQATokenTarget(retstr2, &rawval2);
        if( ret2 >= 0 ) {
          psvDebugScreenPrintf("This Unit has an %s QA Token installed!", retstr2);
        } else { // failsafe
          psvDebugScreenPrintf("This Unit has a QA Token installed!");
        } y+=2;
      }
      if( rawval == 0x01 ) { // even if qaf name and nvs block is FFed when uninstalled the flag stays! (my JEC 511 has this! snvs qafv still there)
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This Unit had a QA Token uninstalled or deactivated!");
        y+=2;
      }  
    }

    if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // Development Hardware

      /// Development Activation
      ret = getActivationCountNvs(retstr, &rawval);
      if( ret == -2468 ) { // 
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This Unit was never activated for development via AFV token!");
        y += 2;  
      }

    }

    /// Launch Model (came with 1.00)
    if( vshSblAimgrIsCEX() == 1 && vshSblAimgrIsVITA() == 1 && vshSysconIsMCEmuCapable() == 0 ) { // Retail Fat (ignore devkits)
      ret = getMinFirmware(retstr, &rawval);
      if( ret >= 0 && rawval == 0x01000000 ) {
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This likely is a first production run unit! (FW 1.00)");
        y += 2;  
      }
    }

    /// New SoC on Fat ("Iris VA" / CPV-B)
    if( vshSblAimgrIsVITA() == 1 && vshSysconIsMCEmuCapable() == 0 ) { // Retail Fat
      ret = getSoCRevision(retstr, &rawval);
      if( ret >= 0 && rawval == 0x80000115 ) {
        psvDebugScreenSetXY(x1, y);
        psvDebugScreenPrintf("This Unit has the newer and more energy-efficient SoC!");
        y += 2;  
      }
    }

    /// Unusual factory code
    ret = getPscodeFactory(retstr, &rawval);
    if( ret >= 0 && (rawval == 35  || rawval == 36 || rawval == 61 || rawval == 62 || rawval == 63) ) {
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenPrintf("This Unit has been to a Diagnostics / Servicing Center! (0x%X)", rawval);
      y += 2;  
    }

    /// IDU
    ret = getModeIDU(retstr, &rawval); 
    if( ret >= 0 && rawval == TRUE ) {
      psvDebugScreenSetXY(x1, y);
      psvDebugScreenPrintf("This is an 'In-Store Demonstration Unit' running Demo Mode!");
      y+=2;
    }

  y += 2;  






  /// ANOMALIES (everything thats not normal)
  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("The following anomalies have been detected:");
  y += 2;  

  psvDebugScreenSetXY(x1 + 1, y);
  psvDebugScreenPrintf("> Nothing found. All good!"); // this get overwritten if something is found
  
  psvDebugScreenSetTextColor(COL_WARNG);


  /// spoofed firmware
  ret = getCurrentFirmware(0, retstr, &rawval); // sceKernelGetSystemSwVersion (which is where Henkaku is applying the spoof)
  ret2 = getCurrentFirmware(1, retstr2, &rawval2); // os0
  if( ret >= 0 && ret2 >= 0 && rawval != rawval2 ) { 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The firmware is currently spoofed to %s!", retstr);
    y += 2;  
  } 


  /// different consoleid (from CEX2DEX / psp2etoi / spoof)
  ret = getConsoleID(0, retstr, rawstr); // current
  ret2 = getConsoleID(1, retstr2, rawstr2); // idstorage
  if( ret >= 0 && ret2 >= 0 && strcmp(rawstr, rawstr2) != 0 ) { 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The ConsoleID has been replaced or spoofed!");
    y += 2;  
  } 

  /// different open_psid (from CEX2DEX / psp2etoi / spoof)
  ret = getOpenPsid(retstr, rawstr); 
  ret2 = getOpenPsidViaIDStorage(retstr2, rawstr2); 
  if( ret >= 0 && ret2 >= 0 && strcmp(rawstr, rawstr2) != 0 ) { 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The OpenPSID has been replaced or spoofed!");
    y += 2;  
  } 


  /// Default Enter Button for device region
  ret = getRegion(0, retstr, &rawval);
  ret2 = getEnterButton(retstr2, &rawval2);
  if( ret >= 0 && ret2 >= 0 && rawval == 0x103 && rawval2 == 1 ) { // J1 + Cross
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The default enter button has been swapped to cross!");
    y += 2;  
  } else if( ret >= 0 && ret2 >= 0 && rawval != 0x103 && rawval != 0x101 && rawval2 == 0 ) { // !J1 (!DEV) + Circle
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The default enter button has been swapped to circle!");
    y += 2;  
  }


  /// Touchpanels 
  if( vshSblAimgrIsVITA() == 1 ) { // ignore PSTVs

    if( getTouchpanelInfoViaSyscon(retstr, &rawval, 3) < 0 ) { // even front fails to get values if back is disconnected though
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- The rear touchpad might be broken or disconnected!");
      y += 2;  
    } else {
      int fr_vid, fr_fw, fr_cfg, re_vid, re_fw, re_cfg, fr_vid_, fr_fw_, fr_cfg_, re_vid_, re_fw_, re_cfg_;
      
      ret = getTouchpanelInfoViaSyscon(retstr, &fr_vid, 0); 
      ret2 = getTouchpanelInfoViaIDStorage(retstr2, &fr_vid_, 0);
      ret += getTouchpanelInfoViaSyscon(retstr, &fr_fw, 1); 
      ret2 += getTouchpanelInfoViaIDStorage(retstr2, &fr_fw_, 1);
      ret += getTouchpanelInfoViaSyscon(retstr, &fr_cfg, 2); 
      ret2 += getTouchpanelInfoViaIDStorage(retstr2, &fr_cfg_, 2);
      if( ret >= 0 && ret2 >= 0 && (fr_vid != fr_vid_ || fr_fw != fr_fw_ || fr_cfg != fr_cfg_)) {
        psvDebugScreenSetXY(x1 + 1, y);
        psvDebugScreenPrintf("- The front panel has been replaced!");
        y += 2;  
      }
      
      ret = getTouchpanelInfoViaSyscon(retstr, &re_vid, 3); 
      ret2 = getTouchpanelInfoViaIDStorage(retstr2, &re_vid_, 3);
      ret += getTouchpanelInfoViaSyscon(retstr, &re_fw, 4); 
      ret2 += getTouchpanelInfoViaIDStorage(retstr2, &re_fw_, 4);
      ret += getTouchpanelInfoViaSyscon(retstr, &re_cfg, 5); 
      ret2 += getTouchpanelInfoViaIDStorage(retstr2, &re_cfg_, 5);
      if( ret >= 0 && ret2 >= 0 && (re_vid != re_vid_ || re_fw != re_fw_ || re_cfg != re_cfg_)) {
        psvDebugScreenSetXY(x1 + 1, y);
        psvDebugScreenPrintf("- The rear touchpad has been replaced!");
        y += 2;  
      }
    }
  }

  /// Downgraded
  ret = getCurrentFirmware(1, retstr, &rawval); // os0
  ret2 = getPreviousFirmware(retstr2, &rawval2);
  if( ret >= 0 && ret2 >= 0 && rawval < rawval2 ) { 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The firmware has been downgraded!");
    y += 2;
  }


  /// Target Build check
  ret = getTarget(1, retstr, &rawval); // via IDStorage
  memset(retstr2, 0, 256);
  sprintf(retstr2, getVersionTxtStringValueForKey("build"));
  if( ret >= 0 && strcmp(retstr2, "ERROR") != 0 ) {
    switch(rawval) {
      case 0x100: break; // TEST

      case 0x101: // Devkit "TOOL"
        if( strcmp(retstr2, "CEX_FOR_TOOL") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A CEX_FOR_TOOL Firmware is installed!"); // which is "normal"
          y += 2;
        }
        if( strcmp(retstr2, "TOOL_HIDE_LATEST") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A TOOL_HIDE_LATEST Firmware is installed!"); // which was only found once at 1.80
          y += 2;
        }
        if( strcmp(retstr2, "CEX") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A CEX Firmware is installed on TOOL device target!");
          y += 2;
        }
        if( strcmp(retstr2, "DEX") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A DEX Firmware is installed on TOOL device target!");
          y += 2;
        }
        break; 

      case 0x102: // Testkit "DEX"
        if( strcmp(retstr2, "CEX") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A CEX Firmware is installed on DEX device target!");
          y += 2;
        }
        if( strcmp(retstr2, "TOOL") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A TOOL Firmware is installed on DEX device target!");
          y += 2;
        }
        break; 
  
      case 0x103:  case 0x104:  case 0x105:  case 0x106:
      case 0x107:  case 0x108: case 0x109:  case 0x10A:
      case 0x10B:  case 0x10C:  case 0x10D:  case 0x10E:
      case 0x10F:  case 0x110:  case 0x111: // Retail "CEX"
        if( strcmp(retstr2, "DEX") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A DEX Firmware is installed on CEX device target!");
          y += 2;
        }
        if( strcmp(retstr2, "TOOL") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A TOOL Firmware is installed on CEX device target!");
          y += 2;
        }
        if( strcmp(retstr2, "CEX_FOR_TOOL") == 0 ) {
          psvDebugScreenSetXY(x1 + 1, y);
          psvDebugScreenPrintf("- A CEX_FOR_TOOL Firmware is installed on CEX device target!");
          y += 2;
        }
        break;
    }
  }


  if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // Development Hardware

    /// Dev Activation
    ret = getActivationPeriodNvs(retstr, &rawval);
    ret2 = getActivationPeriodDat(retstr2, &rawval2);
    if( ret >= 0 && ret2 >= 0 && rawval != rawval2 ) { // comparing start date int
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- There is a mismatch with activation data between nvs and tm0!");
      y += 2;

    } else if( ret >= 0 && doesFileExist("tm0:activate/act.dat") < 0 ) {
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- No activation files in tm0 although valid nvs is present!");
      y += 2;
    }

    
    /// act.dat backup in pd0
    if( doesFileExist("pd0:/data/act.dat") == 0 ) { // activate.vkp was used
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- Activation backups found in pd0:data/ from using activate.vpk!");
      y += 2;  
    }


  }


  /// read-only partitions mount check (os0, vs0)
  if( doesDirExist("os0:SceIoTrash") == 0 ) { // 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The os0: partition has been mounted for writing before!");
    y += 2;  
  }
  /*if( doesDirExist("vs0:SceIoTrash") == 0 ) { // 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The vs0: partition has been mounted for writing!");
    y += 2;  
  }*/



  /// MBR checks / like check ur0 resize / internal ux0 on Fat
  int ur0blocks = 0, ux0blocks = 0;
  
  for(i = 0; i < 0x10; i++) {
    ret = getMbrPartitionEntry(retstr, &rawval, i, 2); // part_id
    if( ret >= 0 && rawval == 0x7) { // this is ur0
      ret = getMbrPartitionEntry(retstr, &rawval, i, 1); // n_sectors
      if( ret >= 0 ) ur0blocks = rawval; //
    }
    if( ret >= 0 && rawval == 0x8) { // this is ux0
      ret = getMbrPartitionEntry(retstr, &rawval, i, 1); // n_sectors
      if( ret >= 0 ) ux0blocks = rawval; //
    }
  }

  //int hwinfo = 0,
  //ret = getHardwareInfo(retstr, &rawval);
  //if( ret >= 0 ) {
  //  hwinfo = rawval;
  //}

  //if( hwinfo & 0x10  ) { // first byte 0x10 masked means MC EMU capable (this is a Slim or PSTV)
  if( vshSysconIsMCEmuCapable() == 1 ) { // use function instead (is that patched with IMCUnlock though?)

    if( ur0blocks > 0 && ur0blocks != 0x300000 ) { // known slim/pstv size https://wiki.henkaku.xyz/vita/EMMC
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- The ur0: partition has been resized!");
      y += 2;  
    }

    if( ux0blocks > 0 && ux0blocks != 0x208000 && ux0blocks != 0x21A000 && ux0blocks != 0x1EE000 && ux0blocks != 0x260000 ) { // known slim/pstv size https://wiki.henkaku.xyz/vita/EMMC (0x1EE000 & 0x260000 seen on CPV proto)
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- The internal memorycard partition has been resized!");
      y += 2;  
    }

  } else { // no MC EMU capable (this is a Fat model)
    
    if( ur0blocks > 0 && ur0blocks != 0x4EE000 && ur0blocks != 0x50A000 && ur0blocks != 0x51A000 ) { // known fat sizes https://wiki.henkaku.xyz/vita/EMMC
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- The ur0: partition has been resized!");
      y += 2;  
    }

    if( ux0blocks != 0 ) { //
      psvDebugScreenSetXY(x1 + 1, y);
      psvDebugScreenPrintf("- An internal memorycard partition was found on original model!");
      y += 2;  
    }
  }

  
  /// preinstalled wave https://github.com/Princess-of-Sleeping/psp2wpp/blob/master/src/main.c#L441  (uses "vshIdStorageLookup" though)
  ret = getPreinstWave(retstr, &rawval);
  if( ret >= 0 && (rawval & 2) && doesFileExist("pd0:wave/waveparam.bin") != 0 ) { // idstorage yes, file no
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- No pre-installed wave in pd0: although there should be!");
    y += 2;  
  } else if( ret >= 0 && !(rawval & 2) && doesFileExist("pd0:wave/waveparam.bin") == 0 ) { // idstorage no, file yes
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- Pre-installed wave in pd0: although there shouldn't be!");
    y += 2;  
  }

  /// preinstalled theme
  ret = getPreinstTheme(retstr, &rawval);
  if( ret >= 0 && (rawval & 4) && doesDirExist("pd0:theme") != 0 ) { // idstorage yes, actual file no
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- No pre-installed theme in pd0: although there should be!");
    y += 2;  
  } else if( ret >= 0 && !(rawval & 4) && doesDirExist("pd0:theme") == 0 ) { // idstorage no, actual file yes
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- Pre-installed theme in pd0: although there shouldn't be!");
    y += 2;  
  }


  /// near has been replaced with VitaDeploy
  if( doesDirExist("vs0:app/NPXS10000/near_backup") == 0 ) { // https://github.com/SKGleba/VitaDeploy/blob/main/main/main.c#L363
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The 'Near' application has been replaced with VitaDeploy!");
    y += 2;  
  } // also "vdep.vpk" in ur0:temp/

  /// iTLS-Enso
  /*if( doesDirExist("vs0:/data/external/itls/") == 0 ) { // folder stays after uninstall - https://github.com/SKGleba/iTLS-Enso/blob/master/main.c#L19C33-L19C57
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- 'iTLS-Enso' installation files found in vs0!");
    y += 2;  

  } else if( doesFileExist("vs0:/data/external/cert/CA_LIST.cer_old") == 0 ) { // else if in case someone ONLY installed these  https://github.com/SKGleba/iTLS-Enso/blob/master/main.c#L174C8-L174C9
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- 'iTLS-Enso' certificate installation found in vs0!");
    y += 2;  
  } // ur0:tai/config_preitls.txt stays after uninstall as well.. AND the plugin entry in config.txt! wtf gleba!
  */
  
  // more examples: opening imcunlock from vitadeploy will put ud0:vd_ktmp.skprx https://github.com/SKGleba/VitaDeploy/blob/64808f2a90733a1919d475542074da92f2f41981/main/main.c#L134 also ud0:ur0-patch.zip 
  // Enso-ex uninstall doesn't clean up either in ux0 or os0! ARGH   -> I should make a "Skgleba tools leftover files found" section here instead. 

  /*
    EnsoEX
      os0:e2x_ckldr.skprx
      os0:ex/
      ux0:eex/

    VitaDeploy
      ur0:temp/vdep.vpk
      ud0:vd_ktmp.skprx
      ud0:ur0-patch.zip
      ux0:downloads/
      ..

    ITLS-Enso
      ur0:tai/config_preitls.txt
      vs0:data/external/itls/
  */


  /// IDU specific files on memory card
  if( vshRemovableMemoryGetCardInsertState() == 1 && doesFileExist("ux0:shell/applayout.ald") == 0 ) { // "bgimage.bgd" came with later refresh cards
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- Demo Unit specific content files found on the memory card!");
    y += 2;  
  }


  /// ux0 was USB mounted..
  if( doesDirExist("ux0:System Volume Information") == 0 ) { // ..on Windows 
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The memory card has been mounted on Windows!");
    y += 2;  
  } 
  if( doesDirExist("ux0:.fseventsd") == 0 || doesDirExist("ux0:.Spotlight-V100") == 0 || doesFileExist("ux0:.DS_Store") == 0 ) { // ..on macOS
    psvDebugScreenSetXY(x1 + 1, y);
    psvDebugScreenPrintf("- The memory card has been mounted on MacOS!");
    y += 2;  
  }

  y += 2;  

}

void tests() {
  int x1 = 3, x2 = 46, y = 4;


  psvDebugScreenSetXY(x1, y);
  psvDebugScreenSetTextColor(COL_TEXTS);
  psvDebugScreenPrintf("Lorem Ipsum");
  y+=2;




  /////////////////////////////////////////////////////////////////////////////////////////

  /// Device color tests
  y = 2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF555555); // 0x00000000
  psvDebugScreenPrintf("(Crystal) Black");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFDDDDDD); // 0xFFFFFFFF
  psvDebugScreenPrintf("(Crystal) White");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFFF4400);
  psvDebugScreenPrintf("Sapphire Blue");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF0000FF);
  psvDebugScreenPrintf("Cosmic Red");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFC0C0C0);
  psvDebugScreenPrintf("Ice Silver");
  y+=2;

  
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF46EAC7);
  psvDebugScreenPrintf("Lime Green / White");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFE6D8AD);
  psvDebugScreenPrintf("Light Blue / White");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFC000C0);
  psvDebugScreenPrintf("Pink / Black");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF4C7585); // #B3A27A #85754C
  psvDebugScreenPrintf("Khaki / Black");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF0000FF);
  psvDebugScreenPrintf("Red / Black");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFE16941);
  psvDebugScreenPrintf("Blue / Black");
  y+=2;
  
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFC1B6FF);
  psvDebugScreenPrintf("Light Pink / White");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFFFFFFF);
  psvDebugScreenPrintf("Glacier White");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFFFC123);
  psvDebugScreenPrintf("Aqua Blue");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF1F5FFF);
  psvDebugScreenPrintf("Neon Orange");
  y+=2;
  
  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFFC0C0C0);
  psvDebugScreenPrintf("Silver");
  y+=2;

  psvDebugScreenSetXY(x2, y);
  psvDebugScreenSetTextColor(0xFF3442E3);
  psvDebugScreenPrintf("Metallic Red");
  y+=2;


}


////////////////////  ////////////////////////////  /////////////////////////  ///////////////////////  ////////////////////  ////////////////////////////

unsigned char *message(int code, unsigned char *msg) {
  static unsigned char string[16];
  
  if( RAW) {
    sprintf(string, "0x%08X", code);
    return string;
  }
  
  switch(code) { // only a small selection of error codes
    case 0x80010002: return "SCE_ERROR_ERRNO_ENOENT";
    //case 0x80010058: return "SCE_ERROR_ERRNO_ENOSYS";
    case 0x8002D013: return "SCE_KERNEL_ERROR_MODULEMGR_OLD_LIB";
    case 0x8002D003: return "SCE_KERNEL_ERROR_MODULEMGR_NO_LIB";
    case 0x80024501: return "SCE_KERNEL_ERROR_INVALID_UID";
    case 0x80024302: return "SCE_KERNEL_ERROR_NO_FREE_PHYSICAL_PAGE";
    case 0x80230005: return "NO LEAF"; // C4-3085-6
    //case 0x803D0007: return "?"; // when no real memorycard

    // custom error
    case -1111: return "NO HELPER";  //eg: helper plugin is not available)
    case -1234: return "NO DATA";  // eg: a leaf may exist but only FFs where data was expected
    case -2468: return "UNEXPECTED DATA"; // eg: a magic mismatch

    default: return msg;
  }
  
  return msg;
}

unsigned char *warning(int code, unsigned char *msg) {
  psvDebugScreenSetTextColor(COL_WARNG);
  return message(code, msg);
}

unsigned char *error(int code, unsigned char *msg) {
  psvDebugScreenSetTextColor(COL_ERROR);
  return message(code, msg);
}


int savereport(char *file) {
  int i = 0;
  sceIoRemove(file);
  
  ret = logPrintf(file, "PSVident %s", VERSION);
  if( ret < 0) 
    return ret;
  logPrintf(file, "--------------");


  /// Special //////////////////

    /// Prototype
    ret = getSerial(retstr, rawstr); 
    if( ret >= 0 ) {
      if( (rawstr[0] == 'T' && rawstr[1] == 'G') || (rawstr[1] == 'T' && rawstr[2] == 'G') ) // either "00-TG.." or "03-TG.."
        logPrintf(file, "(!) Prototype");
    }

    /// Refurbished
    ret = getRefurbished(retstr, &rawval);
    if( ret >= 0 && rawval == TRUE ) { 
      logPrintf(file, "(!) Refurbished");
    }

    /// Internal
    ret = getFirmwareInternal(retstr, &rawval); 
    if( ret >= 0 && rawval == TRUE) {
      logPrintf(file, "(!) Internal Firmware");
    }

    /// QA Token
    ret = getNvsQaf(retstr, &rawval); 
    if( ret >= 0 && rawval == 0x00) { // enabled flag
      logPrintf(file, "(!) QA-Flagged");
    }

    /// Launch Model (came with 1.00)
    if( vshSblAimgrIsCEX() == 1 && vshSblAimgrIsVITA() == 1 && vshSysconIsMCEmuCapable() == 0 ) { // Retail Fat (ignore devkits)
      ret = getMinFirmware(retstr, &rawval);
      if( ret >= 0 && rawval == 0x01000000 ) {
        logPrintf(file, "(!) Launch Model");
      }
    }

    /// New SoC on Fat ("Iris VA" / CPV-B)
    if( vshSblAimgrIsVITA() == 1 && vshSysconIsMCEmuCapable() == 0 ) { // Retail Fat
      ret = getSoCRevision(retstr, &rawval);
      if( ret >= 0 && rawval == 0x80000115 ) {
        logPrintf(file, "(!) New SoC");
      }
    }

    /// Unusual factory code
    ret = getPscodeFactory(retstr, &rawval);
    if( ret >= 0 && (rawval == 35  || rawval == 36 || rawval == 61 || rawval == 62 || rawval == 63) ) {
      logPrintf(file, "(!) Serviced (0x%X)", rawval);
    }

    /// IDU
    ret = getModeIDU(retstr, &rawval); 
    if( ret >= 0 && rawval == TRUE ) {
      logPrintf(file, "(!) Demonstration Unit");
    }
  
    logPrintf(file, "");


  /// Overview ////////////////////////

    logPrintf(file, "Model:                %s",   (ret = getModelName(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Color:                %s",   (ret = getModelColor(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Region:               %s",   (ret = getRegion(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Target:               %s",   (ret = getTarget(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Motherboard:          %s",   (ret = getMotherboard(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Hardware Info:        %s",   (ret = getHardwareInfo(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    if( vshSblAimgrIsTool() == 1 ) { // has CP-Board
    logPrintf(file, "★ CP-Board:           %s",   (ret = getCpboard(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "★ CP-Board Info:      %s",   (ret = getCpInfo(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    logPrintf(file, "");

  //logPrintf(file, "Cur. Firmware:        %s %s",        (ret = getCurrentFirmware(2, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"), getVersionTxtStringValueForKey("build"));
    logPrintf(file, "Current Firmware:     %s [%s]",      (ret = getCurrentFirmware(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"), (ret2 = getFirmwareInternal(retstr2, &rawval2)) >= 0 ? retstr2 : error(ret2, "ERROR"));
    logPrintf(file, "Previous Firmware:    %s",           (ret = getPreviousFirmware(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Factory Firmware:     %s",           (ret = getFactoryFirmware(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Minimum Firmware:     %s",           (ret = getMinFirmware(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "ePSP Firmware:        %s",           (ret = getPspemuFirmware(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");

    logPrintf(file, "Device Serial:        %s",           (ret = getSerial(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Mainboard Serial:     %s",           (ret = getKibanId(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    if( vshSblAimgrIsTool() == 1 ) { // is Devkit
    logPrintf(file, "★ CP-Board Serial:    %s",           (ret = getCpKibanId(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    if( vshSblAimgrIsVITA() == 1 ) { // has screen
    logPrintf(file, "OLED Serial #1:       %s",           (ret = getOledSerial1(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "OLED Serial #2:       %s",           (ret = getUnknown194(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "LCD Serial:           %s",           (ret = getLcdSerial(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    } 
    if( vshSysconHasWWAN() ) { // has SIM
    logPrintf(file, "3G Module Info:       %s",           (ret = getUnknown120(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    if( vshSblAimgrIsVITA() == 1 && vshSblAimgrIsTool() == 0 ) { // has battery
    logPrintf(file, "Battery Serial:       %s",           (ret = getBatterySerial(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    } 
    logPrintf(file, "");

    logPrintf(file, "Mac Address (Wifi):   %s",           (ret = getMacAddressWifi(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Mac Address (BT):     %s",           (ret = getMacAddressBluetoothViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    if( vshSblAimgrIsDolce() == 1 ) { // has Ethernet
    logPrintf(file, "Mac Address (LAN):    %s",           (ret = getMacAddressLanViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    } 
    if( vshSysconHasWWAN() ) { // has SIM
    logPrintf(file, "IMEI:                 %s",           (ret = getImeiViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    logPrintf(file, "");

    logPrintf(file, "SoC Revision:         %s [0x%08X]",  (ret = getSoCRevision(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"), (ret2 = getSoC(retstr2, &rawval2)) >= 0 ? rawval : -1);
    logPrintf(file, "SysCon Version:       %s",           (ret = getBaryonVersion(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "PowerMan Version:     %s",           (ret = getPowerManVersionViaIDStorage(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    if( vshSblAimgrIsVITA() == 1 && vshSblAimgrIsTool() == 0 ) { // has battery
    logPrintf(file, "USB ChargeMan Ver.:   %s",           (ret = getUsbChargeManVersionViaIDStorage(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "FuelGauge Revision:   %s (HWinfo)",  (ret = getBatteryVersionHwinfo(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "FuelGauge Version:    %s (FWinfo)",  (ret = getBatteryVersionFwinfo(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "FuelGauge Param:      %s (DFinfo)",  (ret = getBatteryVersionDfinfo(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    logPrintf(file, "Bootloader Rev:       %s",           (ret = getBootloaderVersion(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");

    logPrintf(file, "Content Manager ID:   %s",  getRegistryBinaryNow("/CONFIG/NP", "account_id", 8));
    logPrintf(file, "");

    logPrintf(file, "USB/MTP Serial:       %s",  (ret = getMtpSerial(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");
    
    logPrintf(file, "Open PSID:            %s",  (ret = getOpenPsidViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");

    logPrintf(file, "ConsoleID:            %s",  (ret = getConsoleID(1, retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Product Code:       %s",  (ret = getConsoleIdProductcode(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Product SubCode:    %s",  (ret = getConsoleIdSubcode(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Factory Code:       %s",  (ret = getConsoleIdFactoryCode(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Serial Number:      %s",  (ret = getConsoleIdSerial(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\n");


    logPrintf(file, "Username:             %s",  (ret = getSystemUsername(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Enter Button:         %s",  (ret = getEnterButton(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Language:             %s",  (ret = getNvsLanguage(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Latitude:             %s",  getRegistryStringNow("/CONFIG/LOCATION/", "last_latitude"));
    logPrintf(file, "Longitude:            %s",  getRegistryStringNow("/CONFIG/LOCATION/", "last_longitude"));
    logPrintf(file, "Installed Apps:       %s",  (ret = getUserApps(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Installed Themes:     %s",  (ret = getUserThemes(retstr, &rawval)) >= 0 ? retstr : (uint8_t*)"0");
    logPrintf(file, "Last Firmware Update: %s",  (ret = getDateLastFirmwareUpdated(retstr)) >= 0 ? retstr : error(ret, "UNKNOWN"));
    logPrintf(file, "");

    logPrintf(file, "MemoryCard inserted:  %s",  (vshRemovableMemoryGetCardInsertState() == 1 ? "TRUE" : "FALSE"));
    logPrintf(file, "Emulates MemoryCard:  %s",  (vshSysconIsMCEmuCapable() == 1 ? "TRUE" : "FALSE"));
    logPrintf(file, "NP Activated:         %s",  getRegistryBooleanNow("/CONFIG/NP", "enable_np"));
    if( vshSysconHasWWAN() ) { // has SIM
    logPrintf(file, "SIM Locked:           %s",  (ret = getSimLocked(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    logPrintf(file, "Demo Mode:            %s",  (ret = getModeIDU(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    if( vshSblAimgrIsDEX() == 1 ) { // testkit
    logPrintf(file, "★ Show Mode:          %s",  (ret = getModeShow(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    if( vshSblAimgrIsTool() == 1 ) { // devkit
    logPrintf(file, "★ Development Mode:   %s",  (ret = getModeDevelopment(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "★ PSTV Mode:          %s",  (ret = getModePstvForDevkit(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    }
    logPrintf(file, "Manufacturing Mode:   %s",  (ret = getModeManufacturing(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Enso installed:       %s",  (ret = getEnso(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Enso was installed:   %s",  (ret = getEnsoUninstalled(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Auto AVLS:            %s",  (ret = getAutoAvls(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Diagnosed:            %s",  (ret = getDiagnostics(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Refurbished:          %s",  (ret = getRefurbished(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Preinstalled Wave:    %s",  (ret = getPreinstWave(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "Preinstalled Theme:   %s",  (ret = getPreinstTheme(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\n");

    logPrintf(file, "%s",    getVersionTxtString());
    logPrintf(file, "\n");

    logPrintf(file, "%s",    getPspemuVersionTxt());
    logPrintf(file, "\n");

    logPrintf(file, "%s",    getSmiString1(-1));
    logPrintf(file, "%s",    getSmiString2(-1));
    logPrintf(file, "\n");

    if( vshSblAimgrIsTool() == 1 ) { // devkit
    logPrintf(file, "★ Kit Nickname:       %s",    getRegistryStringNow("/DEVENV/HOST", "devkitname"));
    logPrintf(file, "★ CP Timestamp:       %s",    (ret = getCpTimestamp(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");
    } 
    if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // 
    logPrintf(file, "★ Activation Status:  %s",    (ret = getActivationStatus(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "★ Activation Period:  %s",    (ret = getActivationPeriodNvs(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "★ Activation Count:   %s",    (ret = getActivationCountNvs(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "★ Activation Files:   %s",    (doesFileExist("tm0:activate/act.dat") == 0 && doesFileExist("tm0:activate/actsig.dat") == 0) ? "Present" : "Not Present");
    logPrintf(file, "\n");
    }

    if( vshSblAimgrIsDolce() == 0 ) { // has touch & Motion
    logPrintf(file, "Front Touch Panel");
    logPrintf(file, "> Vendor ID:     %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 0)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Firmware Rev:  %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 1)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Config Rev:    %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 2)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\nRear Touch Pad");
    logPrintf(file, "> Vendor ID:     %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 3)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Firmware Rev:  %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 4)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Config Rev:    %s",  (ret = getTouchpanelInfoViaSyscon(retstr, &rawval, 5)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\nMotion Sensor");
    logPrintf(file, "> Firmware Rev:  %s",  (ret = getMotionSensorInfoViaIDStorage(retstr, &rawval, 0)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Hardware Info: %s",  (ret = getMotionSensorInfoViaIDStorage(retstr, &rawval, 1)) >= 0 ? retstr : error(ret, "ERROR"));
    } logPrintf(file, "\n");
  
    logPrintf(file, "DipSwitches:");
    logPrintf(file, "> 000-031: %s (cp_timestamp_1)",        (ret = getDipSwitches(retstr, &rawval, 0)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 032-047: %s     (cp_version)",        (ret = getDipSwitches(retstr, &rawval, 1)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 048-063: %s     (cp_build_id)",       (ret = getDipSwitches(retstr, &rawval, 2)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 064-095: %s (cp_timestamp_2)",        (ret = getDipSwitches(retstr, &rawval, 3)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 096-127: %s (aslr_seed)",             (ret = getDipSwitches(retstr, &rawval, 4)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 128-159: %s (sce_sdk_flags)",         (ret = getDipSwitches(retstr, &rawval, 5)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 160-191: %s (shell_flags)",           (ret = getDipSwitches(retstr, &rawval, 6)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 192-223: %s (debug_control_flags)",   (ret = getDipSwitches(retstr, &rawval, 7)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> 224-255: %s (system_control_flags)",  (ret = getDipSwitches(retstr, &rawval, 8)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\n");


    ret = getNvsQaf(retstr, &rawval);
    if( ret >= 0 && rawval == 0x00 ) {
      logPrintf(file, "QA Flags:     %s", (ret = getQAFlags(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
      logPrintf(file, "Template:     %s", (ret = getQATokenName(retstr)) >= 0 ? retstr : error(ret, "ERROR"));
      logPrintf(file, "");
    }
    logPrintf(file, "Boot Flags:     %s", (ret = getBootFlags(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "");
    logPrintf(file, "Hardware Flags: %s", (ret = getHardwareFlags(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "\n");


    logPrintf(file, "PSN Details");
    logPrintf(file, "> Username:      %s",  (ret = getSystemUsername(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
    logPrintf(file, "> Account ID:    %s",  getRegistryBinaryNow("/CONFIG/NP", "account_id", 8));
    logPrintf(file, "> Login ID:      %s",  getRegistryStringNow("/CONFIG/NP", "login_id"));
    logPrintf(file, "> Password:      %s",  getRegistryStringNow("/CONFIG/NP", "password"));
    logPrintf(file, "> Date of Birth: %04d-%02d-%02d", getRegistryIntegerNow_("/CONFIG/NP", "yob"), getRegistryIntegerNow_("/CONFIG/NP", "mob"), getRegistryIntegerNow_("/CONFIG/NP", "dob"));
    logPrintf(file, "> Country:       %s",  getRegistryStringNow("/CONFIG/NP", "country"));
    logPrintf(file, "> Language:      %s",  getRegistryStringNow("/CONFIG/NP", "lang"));
    logPrintf(file, "\n");


    for( i = 1; i < 0x1F; i++) {
      ret = getRegistryWifiSSID(retstr, i);
      if( ret >= 0 && strlen(retstr) > 0 ) { // skip empty
        logPrintf(file, "Wifi Profile %d", i);
        logPrintf(file, "> SSID: %s",  (ret = getRegistryWifiSSID(retstr, i)) >= 0 ? retstr : error(ret, "ERROR"));
        logPrintf(file, "> Pass: %s",  (ret = getRegistryWifiPassword(retstr, i)) >= 0 ? retstr : error(ret, "ERROR"));
        logPrintf(file, "");
      }
    } logPrintf(file, "\n");






  /// Anomalies //////////////////////////////

    logPrintf(file, "Detected Anomlies & Problems:");


    /// spoofed firmware
    ret = getCurrentFirmware(0, retstr, &rawval); // sceKernelGetSystemSwVersion (which is where Henkaku is applying the spoof)
    ret2 = getCurrentFirmware(1, retstr2, &rawval2); // os0
    if( ret >= 0 && ret2 >= 0 && rawval != rawval2 ) { 
      logPrintf(file, "- The firmware is currently spoofed to %s!", retstr);
    } 

    /// different consoleid (from CEX2DEX / psp2etoi / spoof)
    ret = getConsoleID(0, retstr, rawstr); // current
    ret2 = getConsoleID(1, retstr2, rawstr2); // idstorage
    if( ret >= 0 && ret2 >= 0 && strcmp(rawstr, rawstr2) != 0 ) { 
      logPrintf(file, "- The ConsoleID has been replaced or spoofed!");
    } 

    /// different open_psid (from CEX2DEX / psp2etoi / spoof)
    ret = getOpenPsid(retstr, rawstr); 
    ret2 = getOpenPsidViaIDStorage(retstr2, rawstr2); 
    if( ret >= 0 && ret2 >= 0 && strcmp(rawstr, rawstr2) != 0 ) { 
      logPrintf(file, "- The OpenPSID has been replaced or spoofed!");
    } 

    /// Default Enter Button for device region
    ret = getRegion(0, retstr, &rawval);
    ret2 = getEnterButton(retstr2, &rawval2);
    if( ret >= 0 && ret2 >= 0 && rawval == 0x103 && rawval2 == 1 ) { // J1 + Cross
      logPrintf(file, "- The default enter button has been swapped to cross!");
    } else if( ret >= 0 && ret2 >= 0 && rawval != 0x103 && rawval != 0x101 && rawval2 == 0 ) { // !J1 (!DEV) + Circle
      logPrintf(file, "- The default enter button has been swapped to circle!");
    }

    /// Touchpanels 
    if( vshSblAimgrIsVITA() == 1 ) { // ignore PSTVs
      if( getTouchpanelInfoViaSyscon(retstr, &rawval, 3) < 0 ) { // even front fails to get values if back is disconnected though
        logPrintf(file, "- The rear touchpad might be broken or disconnected!");
      } else {
        int fr_vid, fr_fw, fr_cfg, re_vid, re_fw, re_cfg, fr_vid_, fr_fw_, fr_cfg_, re_vid_, re_fw_, re_cfg_;
        ret = getTouchpanelInfoViaSyscon(retstr, &fr_vid, 0); 
        ret2 = getTouchpanelInfoViaIDStorage(retstr2, &fr_vid_, 0);
        ret += getTouchpanelInfoViaSyscon(retstr, &fr_fw, 1); 
        ret2 += getTouchpanelInfoViaIDStorage(retstr2, &fr_fw_, 1);
        ret += getTouchpanelInfoViaSyscon(retstr, &fr_cfg, 2); 
        ret2 += getTouchpanelInfoViaIDStorage(retstr2, &fr_cfg_, 2);
        if( ret >= 0 && ret2 >= 0 && (fr_vid != fr_vid_ || fr_fw != fr_fw_ || fr_cfg != fr_cfg_)) {
          logPrintf(file, "- The front panel has been replaced!");
        }
        ret = getTouchpanelInfoViaSyscon(retstr, &re_vid, 3); 
        ret2 = getTouchpanelInfoViaIDStorage(retstr2, &re_vid_, 3);
        ret += getTouchpanelInfoViaSyscon(retstr, &re_fw, 4); 
        ret2 += getTouchpanelInfoViaIDStorage(retstr2, &re_fw_, 4);
        ret += getTouchpanelInfoViaSyscon(retstr, &re_cfg, 5); 
        ret2 += getTouchpanelInfoViaIDStorage(retstr2, &re_cfg_, 5);
        if( ret >= 0 && ret2 >= 0 && (re_vid != re_vid_ || re_fw != re_fw_ || re_cfg != re_cfg_)) {
          logPrintf(file, "- The rear touchpad has been replaced!");
        }
      }
    }

    /// Downgraded
    ret = getCurrentFirmware(1, retstr, &rawval); // os0
    ret2 = getPreviousFirmware(retstr2, &rawval2);
    if( ret >= 0 && ret2 >= 0 && rawval < rawval2 ) { 
      logPrintf(file, "- The firmware has been downgraded!");
    }

    /// Target Build check
    ret = getTarget(1, retstr, &rawval); // via IDStorage
    memset(retstr2, 0, 256);
    sprintf(retstr2, getVersionTxtStringValueForKey("build"));
    if( ret >= 0 && strcmp(retstr2, "ERROR") != 0 ) {
      switch(rawval) {
        case 0x100: break; // TEST

        case 0x101: // Devkit "TOOL"
          if( strcmp(retstr2, "CEX_FOR_TOOL") == 0 ) {
            logPrintf(file, "- A CEX_FOR_TOOL Firmware is installed!"); // which is "normal"
          }
          if( strcmp(retstr2, "TOOL_HIDE_LATEST") == 0 ) {
            logPrintf(file, "- A TOOL_HIDE_LATEST Firmware is installed!"); // which was only found once at 1.80
          }
          if( strcmp(retstr2, "CEX") == 0 ) {
            logPrintf(file, "- A CEX Firmware is installed on TOOL device target!");
          }
          if( strcmp(retstr2, "DEX") == 0 ) {
            logPrintf(file, "- A DEX Firmware is installed on TOOL device target!");
          }
          break; 

        case 0x102: // Testkit "DEX"
          if( strcmp(retstr2, "CEX") == 0 ) {
            logPrintf(file, "- A CEX Firmware is installed on DEX device target!");
          }
          if( strcmp(retstr2, "TOOL") == 0 ) {
            logPrintf(file, "- A TOOL Firmware is installed on DEX device target!");
          }
          break; 
    
        case 0x103:  case 0x104:  case 0x105:  case 0x106:
        case 0x107:  case 0x108: case 0x109:  case 0x10A:
        case 0x10B:  case 0x10C:  case 0x10D:  case 0x10E:
        case 0x10F:  case 0x110:  case 0x111: // Retail "CEX"
          if( strcmp(retstr2, "DEX") == 0 ) {
            logPrintf(file, "- A DEX Firmware is installed on CEX device target!");
          }
          if( strcmp(retstr2, "TOOL") == 0 ) {
            logPrintf(file, "- A TOOL Firmware is installed on CEX device target!");
          }
          if( strcmp(retstr2, "CEX_FOR_TOOL") == 0 ) {
            logPrintf(file, "- A CEX_FOR_TOOL Firmware is installed on CEX device target!");
          }
          break;
      }
    }
    

    if( vshSblAimgrIsDEX() == 1 || vshSblAimgrIsTool() == 1 ) { // Development Hardware

      /// Dev Activation
      ret = getActivationPeriodNvs(retstr, &rawval);
      ret2 = getActivationPeriodDat(retstr2, &rawval2);
      if( ret >= 0 && ret2 >= 0 && rawval != rawval2 ) { // comparing start date int
        logPrintf(file, "- There is a mismatch with activation data between nvs and tm0!");

      } else if( ret >= 0 && doesFileExist("tm0:activate/act.dat") < 0 ) {
        logPrintf(file, "- No activation files in tm0 although valid nvs is present!");
      }

      /// act.dat backup in pd0
      if( doesFileExist("pd0:/data/act.dat") == 0 ) { // activate.vkp was used
        logPrintf(file, "- Activation backups found in pd0:data/ from using activate.vpk!");
      }

    }


    /// read-only partitions mount check (os0, vs0)
    if( doesDirExist("os0:SceIoTrash") == 0 ) { // 
      logPrintf(file, "- The os0: partition has been mounted for writing before!");
    }


    /// MBR checks / like check ur0 resize / internal ux0 on Fat
    int ur0blocks = 0, ux0blocks = 0;
    for(i = 0; i < 0x10; i++) {
      ret = getMbrPartitionEntry(retstr, &rawval, i, 2); // part_id
      if( ret >= 0 && rawval == 0x7) { // this is ur0
        ret = getMbrPartitionEntry(retstr, &rawval, i, 1); // n_sectors
        if( ret >= 0 ) ur0blocks = rawval; //
      }
      if( ret >= 0 && rawval == 0x8) { // this is ux0
        ret = getMbrPartitionEntry(retstr, &rawval, i, 1); // n_sectors
        if( ret >= 0 ) ux0blocks = rawval; //
      }
    }
    if( vshSysconIsMCEmuCapable() == 1 ) { // use function instead (is that patched with IMCUnlock though?)

      if( ur0blocks > 0 && ur0blocks != 0x300000 ) { // known slim/pstv size https://wiki.henkaku.xyz/vita/EMMC
        logPrintf(file, "- The ur0: partition has been resized!");
      }

      if( ux0blocks > 0 && ux0blocks != 0x208000  && ux0blocks != 0x21A000 && ux0blocks != 0x1EE000 && ux0blocks != 0x260000 ) { // known slim/pstv size https://wiki.henkaku.xyz/vita/EMMC (0x1EE000 & 0x260000 seen on CPV proto)
        logPrintf(file, "- The internal memorycard partition has been resized!");
      }

    } else { // no MC EMU capable (this is a Fat model)
      
      if( ur0blocks > 0 && ur0blocks != 0x4EE000 && ur0blocks != 0x50A000 && ur0blocks != 0x51A000 ) { // known fat sizes https://wiki.henkaku.xyz/vita/EMMC
        logPrintf(file, "- The ur0: partition has been resized!");
      }

      if( ux0blocks != 0 ) { //
        logPrintf(file, "- An internal memorycard partition was found on original model!");
      }
    }


    /// preinstalled wave https://github.com/Princess-of-Sleeping/psp2wpp/blob/master/src/main.c#L441  (uses "vshIdStorageLookup" though)
    ret = getPreinstWave(retstr, &rawval);
    if( ret >= 0 && (rawval & 2) && doesFileExist("pd0:wave/waveparam.bin") != 0 ) { // idstorage yes, file no
      logPrintf(file, "- No pre-installed wave in pd0: although there should be!");
    } else if( ret >= 0 && !(rawval & 2) && doesFileExist("pd0:wave/waveparam.bin") == 0 ) { // idstorage no, file yes
      logPrintf(file, "- Pre-installed wave in pd0: although there shouldn't be!");
    }


    /// preinstalled theme
    ret = getPreinstTheme(retstr, &rawval);
    if( ret >= 0 && (rawval & 4) && doesDirExist("pd0:theme") != 0 ) { // idstorage yes, actual file no
      logPrintf(file, "- No pre-installed theme in pd0: although there should be!");
    } else if( ret >= 0 && !(rawval & 4) && doesDirExist("pd0:theme") == 0 ) { // idstorage no, actual file yes
      logPrintf(file, "- Pre-installed theme in pd0: although there shouldn't be!");
    }


    /// near has been replaced with VitaDeploy
    if( doesDirExist("vs0:app/NPXS10000/near_backup") == 0 ) { // https://github.com/SKGleba/VitaDeploy/blob/main/main/main.c#L363
      logPrintf(file, "- The 'Near' application has been replaced with VitaDeploy!");
    }


    /// IDU specific files on memory card
    if( vshRemovableMemoryGetCardInsertState() == 1 && doesFileExist("ux0:shell/applayout.ald") == 0 ) { // "bgimage.bgd" came with later refresh cards
      logPrintf(file, "- Demo Unit specific content files found on the memory card!");
    }


    /// ux0 was USB mounted..
    if( doesDirExist("ux0:System Volume Information") == 0 ) { // ..on Windows 
      logPrintf(file, "- The memory card has been mounted on Windows!"); 
    } 
    if( doesDirExist("ux0:.fseventsd") == 0 || doesDirExist("ux0:.Spotlight-V100") == 0 || doesFileExist("ux0:.DS_Store") == 0 ) { // ..on macOS
      logPrintf(file, "- The memory card has been mounted on MacOS!");
    }


    logPrintf(file, "\n--------------------------------------------------------------------\n");



    printCapabilities(file);
    logPrintf(file, "");

    printQaf(file);
    logPrintf(file, "");

    return 0;

  /// Overview (test / alt) //////////////////////////////
/*  
  logPrintf(file, "> Overview");

  logPrintf(file, "Model:                                   %s",  (ret = getModelName(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Model Color:                             %s",  (ret = getModelColor(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Region (via SblAimgr):                   %s",  (ret = getRegion(0, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Region (via IDStorage):                  %s",  (ret = getRegion(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Target (via SblAimgr):                   %s",  (ret = getTarget(0, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Target (via IDStorage):                  %s",  (ret = getTarget(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Target (via vshSblAimgrIs):              %s",  (ret = getTarget(2, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Motherboard:                             %s",  (ret = getMotherboard(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Firmware (via os0):                      %s",  (ret = getCurrentFirmware(1, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Firmware (via index.dat):                %s",  (ret = getCurrentFirmware(2, retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Firmware Build (via index.dat):          %s",  getVersionTxtStringValueForKey("build"));
  logPrintf(file, "Firmware Keyset:                         %s",  (ret = getFirmwareInternal(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  
  logPrintf(file, "Min. Firmware:                           %s",  (ret = getMinFirmware(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Device Serial:                           %s",  (ret = getSerial(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "ConsoleID (via SblAimgr):                %s",  (ret = getConsoleID(0, retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "ConsoleID (via IDStorage):               %s",  (ret = getConsoleID(1, retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Mac Address Wifi:                        %s",  (ret = getMacAddressWifi(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Mac Address Wifi (via IDStorage):        %s",  (ret = getMacAddressWifiViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Mac Address Bluetooth (via IDStorage):   %s",  (ret = getMacAddressBluetoothViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Mac Address LAN (via IDStorage):         %s",  (ret = getMacAddressLanViaIDStorage(retstr, rawstr)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "SoC Revision:                            %s",  (ret = getSoCRevision(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));

  logPrintf(file, "Hardware Info (via Syscon):              %s",  (ret = getHardwareInfo(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
  logPrintf(file, "Hardware Info (via IDStorage):           %s",  (ret = getHardwareInfoViaIDStorage(retstr, &rawval)) >= 0 ? retstr : error(ret, "ERROR"));
*/

}

void printMenu(int menu) {

  psvDebugScreenClear();
  psvDebugScreenSetXY(1, 1);
  psvDebugScreenSetTextColor(COL_TITLE);
  psvDebugScreenPrintf("PSVident %s", VERSION);  
  
  psvDebugScreenSetTextColor(GREY);
  if( CENSORED ) psvDebugScreenPrintf(" -censor");
  if( RAW ) psvDebugScreenPrintf(" -raw");
  
  ////////////////////////////////////////////////
  
  /// menu indicator
  //if( 1 ) { // todo blink?!
    if( menu > 0 ) { // display "go left"
      psvDebugScreenSetXY(0, 16); // 32
      psvDebugScreenSetTextColor(GREY);
      psvDebugScreenPrintf("\xD0"); // <
    }
    if( menu < 23 /* hardcoded! */ ) { // display "go right"
      psvDebugScreenSetXY(67, 16); // 32
      psvDebugScreenSetTextColor(GREY);
      psvDebugScreenPrintf("\xCF"); // >
    }
  //} 
    
  ////////////////////////////////////////////////
  
  switch( menu ) {
    case 0: overview(); break;
    case 1: hardware(); break;
    case 2: firmware(); break;
    case 3: software(); break;
    case 4: consoleid(); break;
    case 5: battery(); break;
    case 6: power(); break;
    case 7: touchpanel(); break;
    case 8: memorycard(); break;
    case 9: gamecard(); break;
    case 10: partitions(); break;
    case 11: activation(); break;
    case 12: identifiers(); break;
    case 13: threegee(); break;
    case 14: wifiprofiles(); break;
    case 15: livearea(); break;
    case 16: registry(); break;
    case 17: playlog(); break;
    case 18: clocks(); break;
    case 19: dipswitches(); break;
    case 20: qaflags(); break;
    case 21: modules(); break;
    case 22: miscellaneous(); break;
    case 23: summary(); break;
    case 24: tests(); break;
    
    default: overview();
  }

}


int main(int argc, char *argv[]) {
  SceCtrlData pad, oldpad;
  oldpad.buttons = 0;
  int menu = 0; // 0 = "Overview", 8 = "Gamecard" page etc
  SceRtcTick tick, curtick;
  static SceUID kernel_id = -1, user_id = -1;
  u_int8_t modbuf[8];

  initNet();
  initSceAppUtil();
  sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL); 
  
  psvDebugScreenInit();
   
  
  psvDebugScreenPrintf("loading kernel plugin.. ");
  SceUID modid = _vshKernelSearchModuleByName("psvident_kernel", modbuf);
  if( modid >= 0 ) { // already exists
    psvDebugScreenPrintf("Already loaded!\n\n");
    kernmode = 1;

  } else { // load it
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
  }

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
  
  // sceKernelDelayThread(1 * 1000 * 1000); // wait 1 seconds

  if( !kernmode ) 
    sendNotification("Failed to load helper plugin!");
  

  /// set default category color to match device
  int ret = getModelColor(retstr, &rawval);
  if( ret >= 0 /*&& rawval != 0xFF000000 && rawval != 0xFFFFFFFF*/ ) { // when not black or white -> had to adjust black and white in the func
    COL_CATEG = rawval;
  }

  printMenu(menu);
  int refreshed = 0;

  while(1) {
    memset(&pad, 0, sizeof(pad));
    sceCtrlPeekBufferPositive(0, &pad, 1);
    
    if( pad.buttons != oldpad.buttons ) {
      
      /// update values      
      if( pad.buttons & SCE_CTRL_CROSS ) {
        printMenu(menu);
      }
      
      /// CENSORED mode
      if( pad.buttons & SCE_CTRL_SQUARE ) {
        CENSORED = 1 - CENSORED;
        if( CENSORED == 1 ) RAW = 0;
        printMenu(menu); // print again to take effect
      }
      
      /// advanced mode
      if( pad.buttons & SCE_CTRL_TRIANGLE ) {
        RAW = 1 - RAW;
        if( RAW == 1 ) CENSORED = 0;
        printMenu(menu); // print again to take effect
      }
      
      /// reset
      if( pad.buttons & SCE_CTRL_CIRCLE ) {      
        menu = 0;
        RAW = 0;
        CENSORED = 0;
        printMenu(menu);
      }
      
      /// menu next
      if( (pad.buttons & SCE_CTRL_RIGHT) || (pad.buttons & SCE_CTRL_RTRIGGER)) {
        if( menu < 23 ) /* hardcoded! */
          printMenu(++menu);

        //if( menu+1 == 4 && ksceSblAimgrIsDolce() == 0 ) menu++; // skip touchpanel() for PSTVs
        //if( menu+1 == 11 && ) // threegee()

      }
      
      /// menu previous
      if( (pad.buttons & SCE_CTRL_LEFT) || (pad.buttons & SCE_CTRL_LTRIGGER)) {
        if( menu > 0 ) 
          printMenu(--menu);
      }

      //if( pad.buttons & SCE_CTRL_UP) {
      //  ggg++;
      //}
      //if( pad.buttons & SCE_CTRL_DOWN) {
      //  ggg--;
      //}
      
      /// upload report
      //if( pad.buttons & SCE_CTRL_UP ) {
        //uploadreport(); ?!
      //}
      
      /// save report - stage 1
      if( pad.buttons & SCE_CTRL_DOWN ) {
        sceRtcGetCurrentTick(&tick);
        sceRtcTickAddSeconds(&tick, &tick, 2); // 2 seconds
        refreshed = 0;
      }
    }
    
    
    /// save report - stage 2
    if( pad.buttons & SCE_CTRL_DOWN ) {
      sceRtcGetCurrentTick(&curtick);
      if( refreshed == 0 && kernmode == 1 && sceRtcCompareTick(&curtick, &tick) >= 0 ) { // 0 on equal, <0 when tick1 < tick2, >0 when tick1 > tick2 
        psvDebugScreenSetXY(50, 3);
        psvDebugScreenSetTextColor(GREY);
        psvDebugScreenPrintf("Saving Report..");

        static char string[512];
        ret = getSerial(retstr, rawstr);
        if( ret >= 0 ) { // success
          sprintf(string, "ux0:data/psvident_%s.txt", retstr);
        } else sprintf(string, "ux0:data/psvident.txt");

        ret = savereport(string);
        if( ret >= 0 ) {
          static char string2[1024];
          sprintf(string2, "Report saved to %s", string);
          sendNotification(string2);
        }

        printMenu(menu); // refresh
        refreshed = 1;
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
