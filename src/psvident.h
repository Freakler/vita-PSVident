#ifndef __PSVIDENT_H__
#define __PSVIDENT_H__

int _vshSblAimgrGetConsoleId(char CID[32]);
int _vshSysconGetHardwareInfo(unsigned char *info);
int _vshSblAimgrGetSMI(unsigned int *info);
int vshKernelCheckModelCapability(void);
int vshSblAimgrIsVITA(void);
int vshSblAimgrIsDolce(void);
int sceSblPmMgrGetCurrentMode(int *result);

int sceSblQafManagerGetQafNameForUser(char *buffer, unsigned int max_len); // NID 0x0F7EA8C2 
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


/////////////

SceBool GetDipsw(SceUInt32 no);
SceBool GetQAFlag(SceUInt32 no, SceUInt32 mask);

void GetSfoValueByKey(char *sfo, char *key, int *value);
void GetSfoStringByKey(char *sfo, char *key, char *value, int size);

char *getRegistryStringNow(char *path, char *key);
char *getRegistryIntegerNow(char *path, char *key);
int getRegistryIntegerNow_(char *path, char *key);
char *getRegistryBooleanNow(char *path, char *key);
char *getRegistryBinaryNow(char *path, char *key, int len); // length matters! - todo!?

char *getSmiString1(int part);
char *getSmiString2(int part);
char *getVersionTxtString();
char *getVersionTxtStringValueForKey(const char *key);
char *getIddatString();
char *getPspemuVersionTxt();

int getCurrentFirmware(int mode, uint8_t *retstr, uint32_t *rawval);
int getSerial(uint8_t *retstr, uint8_t *rawstr);
int getRefurbished(uint8_t *retstr, uint32_t *rawval);
int getPreviousFirmware(uint8_t *retstr, uint32_t *rawval);
int getModelCode(uint8_t *retstr, uint8_t *rawstr);
int getModelName(uint8_t *retstr, uint8_t *rawstr);
int getModelColor(uint8_t *retstr, uint32_t *rawval);
int getRegionForDev(uint8_t *retstr, uint32_t *rawval);
int getRegion(int mode, uint8_t *retstr, uint32_t *rawval);
int getTarget(int mode, uint8_t *retstr, uint32_t *rawval);
int getMotherboard(uint8_t *retstr, uint32_t *rawval);
int getCpboard(uint8_t *retstr, uint8_t *rawstr);
int getMinFirmware(uint8_t *retstr, uint32_t *rawval);
int getFactoryFirmware(uint8_t *retstr, uint8_t *rawstr);

int getConsoleID(int mode, uint8_t *retstr, uint8_t *rawstr);
int getConsoleIdCompanycode(int mode, uint8_t *retstr, uint32_t *rawval);
int getConsoleIdProductcode(int mode, uint8_t *retstr, uint32_t *rawval);
int getConsoleIdSubcode(int mode, uint8_t *retstr, uint32_t *rawval);
int getConsoleIdChassis(int mode, uint8_t *retstr, uint32_t *rawval);
int getConsoleIdFactoryCode(int mode, uint8_t *retstr, uint32_t *rawval);
int getConsoleIdSerial(int mode, uint8_t *retstr, uint32_t *rawval);

int getMacAddressWifi(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressWifiViaIDStorage(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressWifiOui(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressLan(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressLanViaIDStorage(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressBluetooth(uint8_t *retstr, uint8_t *rawstr);
int getMacAddressBluetoothViaIDStorage(uint8_t *retstr, uint8_t *rawstr);

int getSoC(uint8_t *retstr, uint32_t *rawval);
int getSoCDram(uint8_t *retstr, uint32_t *rawval);
int getSoCRevision(uint8_t *retstr, uint32_t *rawval);
int getHardwareInfo(uint8_t *retstr, uint32_t *rawval);
int getHardwareInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval);
int getKibanId(uint8_t *retstr, uint8_t *rawstr);
int getCpKibanId(uint8_t *retstr, uint8_t *rawstr);
int getCpInfo(uint8_t *retstr, uint8_t *rawstr);
int getCpTimestamp(uint8_t *retstr, uint32_t *rawval);
int getOpenPsid(uint8_t *retstr, uint8_t *rawstr);
int getOpenPsidViaIDStorage(uint8_t *retstr, uint8_t *rawstr);
int getPscode(uint8_t *retstr, uint8_t *rawstr);
int getPscodeFactory(uint8_t *retstr, uint32_t *rawval);
int getEmmcSize(uint8_t *retstr, uint32_t *rawval);
int getMbrPartitionEntry(uint8_t *retstr, uint32_t *rawval, int entry, int member);
int getEnso(uint8_t *retstr, uint32_t *rawval);
int getEnsoUninstalled(uint8_t *retstr, uint32_t *rawval);
int getMtpSerial(uint8_t *retstr, uint8_t *rawstr);
int getBaryonVersion(uint8_t *retstr, uint32_t *rawval);
int getBaryonTimestamp(uint8_t *retstr, uint8_t *rawstr);
int getBootloaderVersion(uint8_t *retstr, uint32_t *rawval);
int getDipSwitches(uint8_t *retstr, uint32_t *rawval, int x);
int getBootFlags(uint8_t *retstr, uint8_t *rawstr);
int getHardwareFlags(uint8_t *retstr, uint8_t *rawstr);
int getQAFlags(uint8_t *retstr, uint8_t *rawstr);
int getQATokenName(uint8_t *retstr);
int getQATokenNameViaNvs(uint8_t *retstr);
int getQATokenTarget(uint8_t *retstr, uint32_t *rawval);
int getBootTypeIndicator1(uint8_t *retstr, uint32_t *rawval);
int getWakeupFactor(uint8_t *retstr, uint32_t *rawval);
int getWakeupReqFactor(uint8_t *retstr, uint32_t *rawval);
int getEnterButton(uint8_t *retstr, uint32_t *rawval);
int getSystemUsername(uint8_t *retstr, uint8_t *rawstr);
int getTouchpanelInfoViaSyscon(uint8_t *retstr, uint32_t *rawval, int option);
int getTouchpanelInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval, int option);
int getMotionSensorInfoViaIDStorage(uint8_t *retstr, uint32_t *rawval, int option);

int getActivationStatus(uint8_t *retstr, uint32_t *rawval);
int getActivationPeriodNvs(uint8_t *retstr, uint32_t *rawval);
int getActivationPeriodDat(uint8_t *retstr, uint32_t *rawval);
int getActivationCountNvs(uint8_t *retstr, uint32_t *rawval);
int getActivationCountDat(uint8_t *retstr, uint32_t *rawval);

int getMemCardType(uint8_t *retstr, uint32_t *rawval);
int getMemCardSize(uint8_t *retstr, uint8_t *rawstr);
int getMemCardDate(uint8_t *retstr, uint32_t *rawval);
int getMemCardFsoffset(uint8_t *retstr, uint32_t *rawval);
int getMemCardSectorSize(uint8_t *retstr, uint32_t *rawval);
int getMemCardReadonly(uint8_t *retstr, uint32_t *rawval);

int getBatteryVersionHwinfo(uint8_t *retstr, uint32_t *rawval);
int getBatteryVersionFwinfo(uint8_t *retstr, uint32_t *rawval);
int getBatteryVersionDfinfo(uint8_t *retstr, uint32_t *rawval);
int getBatterySerial(uint8_t *retstr, uint8_t *rawstr);
int getBatterySerialDate(uint8_t *retstr, uint8_t *rawstr);
int getBatterySerialModel(uint8_t *retstr, uint8_t *rawstr);
int getBatterySerialNumber(uint8_t *retstr, uint8_t *rawstr);
int getBatteryCharging(uint8_t *retstr, uint32_t *rawval);
int getBatteryPowerOnline(uint8_t *retstr, uint32_t *rawval);
int getBatteryCalibrationVoltage(uint8_t *retstr, uint32_t *rawval);
int getBatteryCalibrationCurrent(uint8_t *retstr, uint32_t *rawval);
int getBatteryCycleCount(uint8_t *retstr, uint32_t *rawval);
int getBatteryTempInCelsius(uint8_t *retstr, uint32_t *rawval);
int getBatteryStateOfHealth(uint8_t *retstr, uint32_t *rawval);
int getBatteryPercentage(uint8_t *retstr, uint32_t *rawval);
int getBatteryVoltage(uint8_t *retstr, uint32_t *rawval);
int getBatteryCapacity(uint8_t *retstr, uint32_t *rawval);
int getBatteryRemCapacity(uint8_t *retstr, uint32_t *rawval);
int getBatteryLifeTime(uint8_t *retstr, uint32_t *rawval);

int getModeIDU(uint8_t *retstr, uint32_t *rawval);
int getModeShow(uint8_t *retstr, uint32_t *rawval);
int getModeDownloader(uint8_t *retstr, uint32_t *rawval);
int getModeDevelopment(uint8_t *retstr, uint32_t *rawval);
int getModeDevelopmentViaDip(uint8_t *retstr, uint32_t *rawval);
int getModeManufacturing(uint8_t *retstr, uint32_t *rawval);

int getModePstvForDevkit(uint8_t *retstr, uint32_t *rawval);
int getModeReleaseCheckConsoleForDevkit(uint8_t *retstr, uint32_t *rawval);
int getModeMemSizeForDevkit(uint8_t *retstr, uint32_t *rawval);

int getFirmwareInternal(uint8_t *retstr, uint32_t *rawval);

int getRegistryWifiSSID(uint8_t *retstr, int profile);
int getRegistryWifiPassword(uint8_t *retstr, int profile);
int getWifiRegion(uint8_t *retstr, uint32_t *rawval);
int getLocalIp(uint8_t *retstr, uint8_t *rawstr);
int getSsid(uint8_t *retstr, uint8_t *rawstr);

int getSimLocked(uint8_t *retstr, uint32_t *rawval);
int getSimCarrier(uint8_t *retstr, uint32_t *rawval);
int getImeiViaIDStorage(uint8_t *retstr, uint8_t *rawstr);
int getIccidViaIDStorage(uint8_t *retstr, uint8_t *rawstr);

int getAutoAvls(uint8_t *retstr, uint32_t *rawval);
int getWaveColor(uint8_t *retstr, uint32_t *rawval);
int getPreinstWave(uint8_t *retstr, uint32_t *rawval);
int getPreinstTheme(uint8_t *retstr, uint32_t *rawval);
int getDiagnostics(uint8_t *retstr, uint32_t *rawval);

int getOledSerial1(uint8_t *retstr, uint8_t *rawstr);
int getUnknown120(uint8_t *retstr, uint8_t *rawstr);
int getUnknown194(uint8_t *retstr, uint8_t *rawstr);
int getLcdSerial(uint8_t *retstr, uint8_t *rawstr);

int getCodename(uint8_t *retstr, uint32_t *rawval);
int getUserApps(uint8_t *retstr, uint32_t *rawval);
int getUserThemes(uint8_t *retstr, uint32_t *rawval);

int getPlaylogEntry(uint8_t *retstr, uint32_t *rawval, int entry, int var);
int getDeviceInfo(uint8_t *retstr, uint32_t *rawval, char* device, int option);

int getPupVersion(uint8_t *retstr, uint32_t *rawval, char* pup);
int getParamValue(uint8_t *retstr, uint32_t *rawval, char* paramsfo, char *key);

int getGamecardMounted(uint8_t *retstr);
int getGamecardInfo(uint8_t *retstr, uint32_t *rawval, int val);

int getPspemuFirmware(uint8_t *retstr,  uint32_t *rawval);
int getUdcdState(uint8_t *retstr,  uint32_t *rawval);
int getUdcdDevInfo(uint8_t *retstr,  uint32_t *rawval);

int getBluetoothRevisionViaIDStorage(uint8_t *retstr,  uint32_t *rawval); // Robin
int getSysconRevisionViaIDStorage(uint8_t *retstr,  uint32_t *rawval); // Ernie
int getPowerManVersionViaIDStorage(uint8_t *retstr,  uint32_t *rawval); // Elmo
int getUsbChargeManVersionViaIDStorage(uint8_t *retstr,  uint32_t *rawval); // Cookie

int getClockArm(uint8_t *retstr, uint32_t *rawval);
int getClockGpu(uint8_t *retstr, uint32_t *rawval);
int getClockGpuXbar(uint8_t *retstr, uint32_t *rawval);
int getClockBus(uint8_t *retstr, uint32_t *rawval);
int getClockTimebase(uint8_t *retstr, uint32_t *rawval);

int getDateLastFirmwareUpdated(uint8_t *retstr);

int getNvsQaf(uint8_t *retstr, uint32_t *rawval);
int getNvsExtraUart(uint8_t *retstr, uint32_t *rawval);
int getNvsSavemode(uint8_t *retstr, uint32_t *rawval);
int getNvsLanguage(uint8_t *retstr, uint32_t *rawval);
int getNvsUpdateMode(uint8_t *retstr, uint32_t *rawval);

int getModuleInfo(int mode, int no, int val, uint8_t *retstr, uint32_t *rawval);

/////////////

void printCapabilities(char *file);
void printQaf(char *file);

#endif