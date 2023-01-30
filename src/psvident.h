#ifndef __PSVIDENT_H__
#define __PSVIDENT_H__

int _vshSblAimgrGetConsoleId(char CID[16]);
int _vshSysconGetHardwareInfo(unsigned char *info);
int _vshSblAimgrGetSMI(unsigned int *info);
int vshKernelCheckModelCapability(void);
int vshSblAimgrIsVITA(void);
int vshSblAimgrIsDolce(void);
int sceSblPmMgrGetCurrentMode(int *result);
int sceSblQafManagerGetQafNameForUser(char *buffer, unsigned int max_len); // NID 0x0F7EA8C2 


char *getCPVersion();
char *getCPBoardId();
char *getCPTimestamp();
char *getSoCRevision();
char *getModelName();
char *getModelCode();
char *getSerial();
char *getConsoleID();
char *getHardwareInfo();
char *getMacAddress();
char *getFactoryFirmware();
char *getMinFirmware();
char *getTarget();
char *getRegionForDev();
char *getRegion();
char *getEmmcSize();
char *getMotherboard();
char *getModelNameCustom();
char *getModelMore();
char *getBaryonVersion();
char *getBaryonTimestamp();
char *getBootloaderVersion();
char *getDipSwitches(int x);
char *getQAFlags();
char *getBootFlags();
char *getHardwareFlags();
char *getQATokenName();
char *getActivationPeriod();
char *getActivationCount();
char *getCurrentFirmware();
char *getPreviousFirmware();
char *getSmiString1(int part);
char *getSmiString2(int part);
char *getVersionTxtString();
char *getBatteryStatus();
char *getBatteryCycleCount();
char *getBatteryTempInCelsius();
char *getBatteryStateOfHealth();
char *getBatteryPercentage();
char *getBatteryVoltage();
char *getBatteryCapacity();
char *getBatteryRemCapacity();
char *getBatteryLifeTime();
char *getBatteryVersionHwinfo();
char *getBatteryVersionFwinfo();
char *getBatteryVersionDfinfo();
char *getModeIDU();
char *getModeShow();
char *getModeDownloader();
char *getModeDevelopment();
char *getModeManufacturing();
char *getEnterButton();
char *getSystemUsername();
char *getRegistryNP();
char *getRegistryLoginId();
char *getRegistryPassword();
char *getRegistryCountry();
char *getRegistryLanguage();
char *getRegistryDateOfBirth();
char *getRegistryAccountId();
char *getTouchpanelInfo(int option);
char *getMemCardType();
char *getMemCardSize();
char *getMemCardDate();
char *getMemCardFsoffset();
char *getMemCardSectorSize();
char *getMemCardReadonly();
char *getWifiSSID(int profile);
char *getWifiPassword(int profile);
char *getAutoAvls();
char *getMotionInfo();

#endif