#include <psp2/appmgr.h>
#include <psp2/shellutil.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/ctrl.h>
#include <psp2kern/bt.h> 
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

int psvident_kblparam_GetBuffer(SceKblParam *kblparam) {
  return k_psvident_kblparam_GetBuffer(kblparam);
}

int psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo) {
  return k_psvident_sysroot_GetBatteryVersion(hwinfo, fwinfo, dfinfo);
}

int psvident_pervasive_GetSoCRevision(unsigned int *soc) {
  return k_psvident_pervasive_GetSoCRevision(soc);
}

int psvident_mbr_Get(SceMbr *master) {
  return k_psvident_mbr_Get(master);
}

int psvident_mbr_CheckEnso(unsigned int *var) {
  return k_psvident_mbr_CheckEnso(var);
}

int psvident_mbr_CheckPreviousEnso(unsigned int *var) {
  return k_psvident_mbr_CheckPreviousEnso(var);
}

int psvident_nvs_Get(int offset, uint8_t *buffer, int size) {
  return k_psvident_nvs_Get(offset, buffer, size);
}

int psvident_GetActivationStatus(void) {
    return k_psvident_GetActivationStatus();
}

int psvident_BtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo *info, SceSize info_size) {
  return k_psvident_BtGetRegisteredInfo(device, unk, info, info_size);
}

int psvident_sdifgetcontextmmc(EmmcCardId *cardid, uint8_t *cid, uint8_t *csd, uint8_t *ext_csd) {
  return k_psvident_sdifgetcontextmmc(cardid, cid, csd, ext_csd);
}

int psvident_detect_plugins(int mode, int no, SceKernelModuleInfo *modinfo) {
  return k_psvident_detect_plugins(mode, no, modinfo);
}

int psvident_test(int *test) {
  return k_psvident_test(test);
}


///////////////////////////////////////////////

int _start(SceSize args, void *argp) __attribute__ ((weak, alias("module_start")));

int module_start(SceSize args, void *argp) {
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  return SCE_KERNEL_STOP_SUCCESS;
}