#ifndef __COMMON_H__
#define __COMMON_H__

typedef struct {
	uint32_t off;
	uint32_t sz;
	uint8_t code;
	uint8_t type;
	uint8_t active;
	uint32_t flags;
	uint16_t unk;
} __attribute__((packed)) partition_t;

typedef struct {
	char magic[0x20];
	uint32_t version;
	uint32_t device_size;
	char unk1[0x28];
	partition_t partitions[0x10];
	char unk2[0x5e];
	char unk3[0x10 * 4];
	uint16_t sig;
} __attribute__((packed)) master_block_t;

int ksceSysconGetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo);

typedef struct SceKernelTouchpanelDeviceInfo {
  uint16_t FrontVendorID;
  uint16_t FrontFwVersion;
  uint16_t BackVendorID;
  uint16_t BackFwVersion;
} SceKernelTouchpanelDeviceInfo;

int ksceSysconGetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo);

typedef struct SceKernelTouchpanelDeviceInfo2 {
	uint16_t FrontVendorID;
	uint16_t FrontFwVersion;
	uint16_t FrontConfRev;
	uint8_t FrontUnk2;
	uint8_t FrontUnk3;
	uint16_t unused1;
	uint16_t BackVendorID;
	uint16_t BackFwVersion;
	uint16_t BackConfRev;
	uint8_t BackUnk2;
	uint8_t BackUnk3;
	uint16_t unused2;
} SceKernelTouchpanelDeviceInfo2;

int ksceSysconGetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);

int ksceSblLicMgrGetLicenseStatus(void);

int (* sceSblNvsReadDataForKernel)(uint32_t offset, char *buffer, uint32_t size);
int (* SceKernelSuspendForDriver_4DF40893)(int a1);
int (* SceKernelSuspendForDriver_2BB92967)(int a1);

//int (*module_get_export_func)(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

////////////////////////////////////////

int psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo);
int k_psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo);

int psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);
int k_psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);

int psvident_syscon_GetBaryonVersion(int *var);
int k_psvident_syscon_GetBaryonVersion(int *var);

int psvident_syscon_GetBaryonTimestamp(unsigned long long *var);
int k_psvident_syscon_GetBaryonTimestamp(unsigned long long *var);

int psvident_sysroot_GetBootloaderRevision(unsigned int *var);
int k_psvident_sysroot_GetBootloaderRevision(unsigned int *var);

int psvident_sysroot_GetQAFlags(uint8_t *string);
int k_psvident_sysroot_GetQAFlags(uint8_t *string);

int psvident_sysroot_GetBootFlags(uint8_t *string);
int k_psvident_sysroot_GetBootFlags(uint8_t *string);

int psvident_sysroot_GetHardwareFlags(uint8_t *string);
int k_psvident_sysroot_GetHardwareFlags(uint8_t *string);

int psvident_sysroot_GetBootTypeIndicator(unsigned int *var);
int k_psvident_sysroot_GetBootTypeIndicator(unsigned int *var);

int psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo);
int k_psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo);

int psvident_pervasive_GetSoCRevision(unsigned int *soc);
int k_psvident_pervasive_GetSoCRevision(unsigned int *soc);

int psvident_mbr_GetEmmcSize(unsigned int *var);
int k_psvident_mbr_GetEmmcSize(unsigned int *var);

int psvident_sysroot_GetDipSwitches(uint8_t *string);
int k_psvident_sysroot_GetDipSwitches(uint8_t *string);

int psvident_nvs_GetKibanId(uint8_t *string);
int k_psvident_nvs_GetKibanId(uint8_t *string);

int psvident_GetActivationStatus(void);
int k_psvident_GetActivationStatus(void);

int psvident_nvs_GetActData(uint8_t *string);
int k_psvident_nvs_GetActData(uint8_t *string);

////////////////////////////////////////

#endif
