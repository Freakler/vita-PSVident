#include <psp2kern/ctrl.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/idstorage.h> 
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/sysroot.h> 
#include <psp2kern/kernel/syscon.h> 
#include <psp2kern/lowio/pervasive.h> 
#include <psp2kern/bt.h> 

#include <psp2kern/kernel/kbl/kbl.h> 
#include <psp2kern/kernel/dipsw.h> 
#include <psp2kern/kernel/ssmgr.h> 
#include <psp2kern/types.h> 
#include <psp2kern/kernel/sdif.h>

#include <taihen.h>

#include <stdio.h>
#include <string.h>

#include "common.h"

int k_psvident_BtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo *info, SceSize info_size) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  SceBtRegisteredInfo kinfo;
  
  int ret = ksceBtGetRegisteredInfo(device, unk, &kinfo, info_size);
  if( ret != 0 )
    return ret;
  
  ksceKernelMemcpyKernelToUser(info, &kinfo, sizeof(info));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  SceKernelTouchpanelDeviceInfo info;
  
  int ret = ksceSysconGetTouchpanelDeviceInfo(&info);
  if( ret != 0 )
    return ret;
  
  ksceKernelMemcpyKernelToUser(pInfo, &info, sizeof(info));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  SceKernelTouchpanelDeviceInfo2 info;
  
  int ret = ksceSysconGetTouchpanelDeviceInfo2(&info);
  if( ret != 0 )
    return ret;
  
  ksceKernelMemcpyKernelToUser(pInfo, &info, sizeof(info));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_mbr_Get(SceMbr *master) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  SceUID fd;
  static SceMbr lokalmaster;
  
  fd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDONLY, 0);
  if( fd >= 0 ) {
    ksceIoRead(fd, &lokalmaster, sizeof(lokalmaster)); // 0x200
    ksceIoClose(fd);
  }
  
  ksceKernelMemcpyKernelToUser(master, &lokalmaster, sizeof(lokalmaster));


  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_mbr_CheckEnso(unsigned int *var) { // https://blog.xyz.is/2018/enso.html
  uint32_t state;
  ENTER_SYSCALL(state);
  
  int ret;
  SceUID fd;
  static SceMbr master;
  
  fd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDONLY, 0);
  if( fd < 0 ) { // failed to open device
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -1;
  }

  ret = ksceIoLseek(fd, 0x200, SCE_SEEK_SET);
  if( ret != 0x200 ) { // failed to seek the device to real mbr copy
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -2;
  }

  ret = ksceIoRead(fd, &master, sizeof(master));
  if( ret != sizeof(master)) { // failed to read the mbr block
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -3;
  }

  ksceIoClose(fd);

  if( memcmp(master.magic, "Sony Computer Entertainment Inc.", 0x20) == 0 ) { // magic found
    ret = 1; // TRUE
  } else {
    ret = 0; // FALSE
  }

  ksceKernelMemcpyKernelToUser(var, &ret, sizeof(int));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_mbr_CheckPreviousEnso(unsigned int *var) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  int ret;
  SceUID fd;
  static SceMbr master;
  
  fd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDONLY, 0);
  if( fd < 0 ) { // failed to open device
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -1;
  }

  ret = ksceIoLseek(fd, 0x6000, SCE_SEEK_SET); // from here its not AAed
  if( ret != 0x6000 ) { // failed to seek
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -2;
  }

  ret = ksceIoRead(fd, &master, sizeof(master));
  if( ret != sizeof(master)) { // failed to read
    ksceIoClose(fd);
    EXIT_SYSCALL(state);
    return -3;
  }

  ksceIoClose(fd);

  if( *(uint32_t*)&master.magic[0] != 0xAAAAAAAA ) 
    ret = 1; // TRUE
  else
    ret = 0; // FALSE

  ksceKernelMemcpyKernelToUser(var, &ret, sizeof(int));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_syscon_GetBaryonVersion(int *var) {
  uint32_t state;
  ENTER_SYSCALL(state);

  int ret = ksceSysconGetBaryonVersion();
  
  ksceKernelMemcpyKernelToUser(var, &ret, sizeof(int));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_syscon_GetBaryonTimestamp(unsigned long long *var) { // todo
  uint32_t state;
  ENTER_SYSCALL(state);

  unsigned long long ret = ksceSysconGetBaryonTimestamp();

  ksceKernelMemcpyKernelToUser(var, &ret, sizeof(unsigned long long));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo) {
  uint32_t state;
  unsigned int k_fwinfo, k_dfinfo;
  unsigned long long int k_hwinfo;
  
  ENTER_SYSCALL(state);
  
  ksceSysconGetBatteryVersion(&k_hwinfo, &k_fwinfo, &k_dfinfo);
  
  ksceKernelMemcpyKernelToUser(hwinfo, &k_hwinfo, sizeof(unsigned long long int));
  ksceKernelMemcpyKernelToUser(fwinfo, &k_fwinfo, sizeof(unsigned int));
  ksceKernelMemcpyKernelToUser(dfinfo, &k_dfinfo, sizeof(unsigned int));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_pervasive_GetSoCRevision(unsigned int *soc) {
  uint32_t state;
  unsigned int k_soc;
  
  ENTER_SYSCALL(state);
  
  k_soc = kscePervasiveGetSoCRevision();
  
  ksceKernelMemcpyKernelToUser(soc, &k_soc, sizeof(unsigned int));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_GetActivationStatus(void) {
  return ksceSblLicMgrGetLicenseStatus();
}

int k_psvident_nvs_Get(int offset, uint8_t *buffer, int size) { // reading in full nvs takes time 
  uint32_t state;
  ENTER_SYSCALL(state);
  
  ////////////////////
  
  static uint8_t knvs[0xB60]; // max
  unsigned int ret = -1;
  
  memset(knvs, 0, 0xB60);
  
  
  tai_module_info_t sysmem_info;
  taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &sysmem_info);
  ret = module_get_export_func(KERNEL_PID, "SceSysmem", 0x7290B21C, 0x4DF40893, (uintptr_t *)&SceKernelSuspendForDriver_4DF40893); // SignalNvsAcquire
  if( ret < 0 ) {
    EXIT_SYSCALL(state);
    return SCE_KERNEL_START_NO_RESIDENT;
  }
  ret = module_get_export_func(KERNEL_PID, "SceSysmem", 0x7290B21C, 0x2BB92967, (uintptr_t *)&SceKernelSuspendForDriver_2BB92967); // SignalNvsFree
  if( ret < 0 ) {
    EXIT_SYSCALL(state);
    return SCE_KERNEL_START_NO_RESIDENT;
  }
  tai_module_info_t ssmgr_info;
  taiGetModuleInfoForKernel(KERNEL_PID, "SceSblSsMgr", &ssmgr_info);
  ret = module_get_export_func(KERNEL_PID, "SceSblSsMgr", 0x74580D9F, 0xC2EC8F5A, (uintptr_t *)&sceSblNvsReadDataForKernel);
  if( ret < 0 ) {
    EXIT_SYSCALL(state);
    return SCE_KERNEL_START_NO_RESIDENT;
  }
  
  SceKernelSuspendForDriver_4DF40893(0); // signal acquire
  ret = sceSblNvsReadDataForKernel(offset, &knvs[0], size); // offset, buffer, size
  if( ret < 0 ) {
    SceKernelSuspendForDriver_2BB92967(0); // signal free
    EXIT_SYSCALL(state);
    return ret;
  }
  SceKernelSuspendForDriver_2BB92967(0); // signal free

    /* https://github.com/SKGleba/psp2etoi/blob/5c8b1e85a3c1db83c3cf38710e3bd1a9cc9500c3/kernel/main.c#L377 */
    
    /// STEP 0
        //ksceKernelSignalNvsAcquire(0);
    
        /// STEP 1
        //ksceSysconNvsSetRunMode(0);
    
    /// STEP 2/3
        //ret = ksceSysconNvsReadData(0x520, nvs, 0x20);
    
    /// STEP 4
        //ksceKernelSignalNvsFree(0);
    
    
  ////////////////////
  
  ksceKernelMemcpyKernelToUser(buffer, &knvs, size);
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_kblparam_GetBuffer(SceKblParam *kblparam) {
  uint32_t state;
  SceKblParam *kblptr;

  ENTER_SYSCALL(state);
  
  kblptr = ksceKernelSysrootGetKblParam();

  ksceKernelMemcpyKernelToUser(kblparam, kblptr, sizeof(SceKblParam));
  
  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_sdifgetcontextmmc(EmmcCardId *cardid, uint8_t *cid, uint8_t *csd, uint8_t *ext_csd) {
  uint32_t state;
  ENTER_SYSCALL(state);

  sd_context_part_mmc *k_context = (sd_context_part_mmc*)ksceSdifGetSdContextPartValidateMmc(SCE_SDIF_DEVICE_GC);  // https://docs.vitasdk.org/sdif_8h_source.html
  if( k_context == NULL ) 
    return -1;
  
  ksceKernelMemcpyKernelToUser(cardid, k_context->ctxb.CID, sizeof(k_context->ctxb.CID));
  ksceKernelMemcpyKernelToUser(cid, k_context->ctxb.CID, sizeof(k_context->ctxb.CID));  
  ksceKernelMemcpyKernelToUser(csd, k_context->ctxb.CSD, sizeof(k_context->ctxb.CSD));  
  ksceKernelMemcpyKernelToUser(ext_csd, k_context->EXT_CSD, sizeof(k_context->EXT_CSD));  

  EXIT_SYSCALL(state);
  return 0;
}

int k_psvident_detect_plugins(int mode, int no, SceKernelModuleInfo *modinfo) { // https://github.com/TheOfficialFloW/modoru/blob/master/kernel.c#L304
  #define MOD_LIST_SIZE 128
  SceKernelModuleInfo info;
  SceUID k_modlist[MOD_LIST_SIZE];
  size_t count = MOD_LIST_SIZE;
  int res;

  uint32_t state;
  ENTER_SYSCALL(state);

  int (* _ksceKernelGetModuleList)(SceUID pid, int flags1, int flags2, SceUID *modids, size_t *num);
  int (* _ksceKernelGetModuleInfo)(SceUID pid, SceUID modid, SceKernelModuleInfo *info);

  res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", TAI_ANY_LIBRARY, 0x97CF7B4E, (uintptr_t *)&_ksceKernelGetModuleList);
  if (res < 0)
    res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", TAI_ANY_LIBRARY, 0xB72C75A4, (uintptr_t *)&_ksceKernelGetModuleList);
  if (res < 0)
    goto err;

  res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", TAI_ANY_LIBRARY, 0xD269F915, (uintptr_t *)&_ksceKernelGetModuleInfo);
  if (res < 0)
    res = module_get_export_func(KERNEL_PID, "SceKernelModulemgr", TAI_ANY_LIBRARY, 0xDAA90093, (uintptr_t *)&_ksceKernelGetModuleInfo);
  if (res < 0)
    goto err;


  if( mode == 0 ) { // kernel 
    res = _ksceKernelGetModuleList(KERNEL_PID, 0x7fffffff, 1, k_modlist, &count);
    if (res < 0)
      goto err;

    info.size = sizeof(SceKernelModuleInfo);
    res = _ksceKernelGetModuleInfo(KERNEL_PID, k_modlist[no], &info);
    if (res < 0)
      goto err;

  }

  if( mode == 1 ) { // user 
    res = _ksceKernelGetModuleList(ksceKernelGetProcessId(), 0x7fffffff, 1, k_modlist, &count);
    if (res < 0)
      goto err;

    info.size = sizeof(SceKernelModuleInfo);
    res = _ksceKernelGetModuleInfo(ksceKernelGetProcessId(), k_modlist[no], &info);
    if (res < 0)
      goto err;

  }

  ksceKernelMemcpyKernelToUser(modinfo, &info, info.size);
  res = 0;

err:
  EXIT_SYSCALL(state);
  return res;
}

int k_psvident_test(int *test) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  int k_test = ksceSblLicMgrGetLicenseStatus();

  ksceKernelMemcpyKernelToUser(test, &k_test, sizeof(int));
  
  EXIT_SYSCALL(state);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////


int _start(SceSize args, void *argp) __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize args, void *argp) {
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  return SCE_KERNEL_STOP_SUCCESS;
}
