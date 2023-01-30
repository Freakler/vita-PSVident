#include <psp2kern/ctrl.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/idstorage.h> 
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/sysroot.h> 
#include <psp2kern/syscon.h> 
#include <psp2kern/lowio/pervasive.h> 

#include <taihen.h>

#include <stdio.h>
#include <string.h>

#include "common.h"


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

int k_psvident_sysroot_GetDipSwitches(uint8_t *string) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	uint8_t ds[0x20];
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		memcpy(ds, (sysroot+0x40), sizeof(ds));

	ksceKernelMemcpyKernelToUser(string, &ds, sizeof(ds));
	
	EXIT_SYSCALL(state);
	return 0;
}

int k_psvident_mbr_GetEmmcSize(unsigned int *var) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	SceUID fd;
	int size = 0;
	static master_block_t master;
	
	fd = ksceIoOpen("sdstor0:int-lp-act-entire", SCE_O_RDONLY, 0);
	if( fd >= 0 ) {
		ksceIoRead(fd, &master, sizeof(master));
		ksceIoClose(fd);
	} //else return fd;
	
	if( strncmp(master.magic, "Sony Computer Entertainment Inc.", 0x20) == 0 ) {
		size = master.device_size * 512; // blocks * block size
	} //else return -1;
	
	ksceKernelMemcpyKernelToUser(var, &size, sizeof(int));
	
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

int k_psvident_sysroot_GetBootloaderRevision(unsigned int *var) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	unsigned int bootldr = -1;
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		bootldr = *(unsigned int *)(sysroot + 0xF8);

	ksceKernelMemcpyKernelToUser(var, &bootldr, sizeof(int));
	
	EXIT_SYSCALL(state);
	return 0;
}

int k_psvident_sysroot_GetQAFlags(uint8_t *string) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	uint8_t qaflags[0x10];
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		memcpy(qaflags, (sysroot+0x20), sizeof(qaflags));

	ksceKernelMemcpyKernelToUser(string, &qaflags, sizeof(qaflags));
	
	EXIT_SYSCALL(state);
	return 0;
}

int k_psvident_sysroot_GetBootFlags(uint8_t *string) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	uint8_t bootflags[0x10];
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		memcpy(bootflags, (sysroot+0x30), sizeof(bootflags));

	ksceKernelMemcpyKernelToUser(string, &bootflags, sizeof(bootflags));
	
	EXIT_SYSCALL(state);
	return 0;
}
int k_psvident_sysroot_GetHardwareFlags(uint8_t *string) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	uint8_t hwflags[0x10];
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		memcpy(hwflags, (sysroot+0xE8), sizeof(hwflags));

	ksceKernelMemcpyKernelToUser(string, &hwflags, sizeof(hwflags));
	
	EXIT_SYSCALL(state);
	return 0;
}

int k_psvident_sysroot_GetBootTypeIndicator(unsigned int *var) { // // https://wiki.henkaku.xyz/vita/KBL_Param#Boot_type_indicator_1
	uint32_t state;
	ENTER_SYSCALL(state);
	
	unsigned int bootype = -1;
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		bootype = *(unsigned int *)(sysroot + 0x6C);

	ksceKernelMemcpyKernelToUser(var, &bootype, sizeof(int));
	
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

int k_psvident_sysroot_DIPSwitches(uint8_t *string) {
	uint32_t state;
	ENTER_SYSCALL(state);
	
	uint8_t dipswitches[0x20];
	void *sysroot = ksceKernelGetSysrootBuffer();
	if( sysroot )
		memcpy(dipswitches, (sysroot+0x40), sizeof(dipswitches));

	ksceKernelMemcpyKernelToUser(string, &dipswitches, sizeof(dipswitches));
	
	EXIT_SYSCALL(state);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////


void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize args, void *argp) {
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  return SCE_KERNEL_STOP_SUCCESS;
}
