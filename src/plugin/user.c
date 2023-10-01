#include <psp2/appmgr.h>
#include <psp2/shellutil.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/ctrl.h>
#include <taihen.h>

#include <stdio.h>
#include <string.h>

#include "common.h"


int psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo) {
	return k_psvident_syscon_GetTouchpanelDeviceInfo(pInfo);
}
int psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo) {
	return k_psvident_syscon_GetTouchpanelDeviceInfo2(pInfo);
}


int psvident_syscon_GetBaryonVersion(int *var) {
	return k_psvident_syscon_GetBaryonVersion(var);
}

int psvident_syscon_GetBaryonTimestamp(unsigned long long *var) {
	return k_psvident_syscon_GetBaryonTimestamp(var);
}

int psvident_sysroot_GetBootloaderRevision(unsigned int* var) {
	return k_psvident_sysroot_GetBootloaderRevision(var);
}

int psvident_sysroot_GetQAFlags(uint8_t *string) {
	return k_psvident_sysroot_GetQAFlags(string);
}

int psvident_sysroot_GetBootFlags(uint8_t *string) {
	return k_psvident_sysroot_GetBootFlags(string);
}

int psvident_sysroot_GetHardwareFlags(uint8_t *string) {
	return k_psvident_sysroot_GetHardwareFlags(string);
}

int psvident_sysroot_GetBootTypeIndicator(unsigned int *var) {
	return k_psvident_sysroot_GetBootTypeIndicator(var);
}

int psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo) {
	return k_psvident_sysroot_GetBatteryVersion(hwinfo, fwinfo, dfinfo);
}

int psvident_pervasive_GetSoCRevision(unsigned int *soc) {
	return k_psvident_pervasive_GetSoCRevision(soc);
}

int psvident_sysroot_DIPSwitches(uint8_t *string) {
	return k_psvident_sysroot_DIPSwitches(string);
}

int psvident_mbr_GetEmmcSize(unsigned int *var) {
	return k_psvident_mbr_GetEmmcSize(var);
}

int psvident_sysroot_GetDipSwitches(uint8_t *string) {
	return k_psvident_sysroot_GetDipSwitches(string);
}

int psvident_nvs_GetKibanId(uint8_t *string) {
	return k_psvident_nvs_GetKibanId(string);
}

///////////////////////////////////////////////

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  return SCE_KERNEL_STOP_SUCCESS;
}