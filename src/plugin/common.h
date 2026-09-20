#ifndef __COMMON_H__
#define __COMMON_H__

typedef struct { // https://wiki.henkaku.xyz/vita/Partitions#Partition_Entries
  uint32_t start_lba;
  uint32_t n_sectors;
  uint8_t part_id;
  uint8_t part_type;
  uint8_t part_flag;
  uint16_t acl;
  uint32_t unk;
} __attribute__((packed)) SceMbrPartEntry;

typedef struct {
  uint8_t magic[0x20];
  uint32_t version;
  uint32_t n_sectors;
  uint8_t unk1[0x8];
  uint32_t loader_start;
  uint32_t loader_count;
  uint32_t current_bl_lba;
  uint32_t bl_bank0_lba;
  uint32_t bl_bank1_lba;
  uint32_t current_os_lba;
  uint8_t unk2[0x8];
  SceMbrPartEntry partitions[0x10];
  uint8_t unk3[0x6E];
  uint8_t unk4[0x30];
  uint16_t sig;
} __attribute__((packed)) SceMbr;

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
  uint8_t  FrontUnk2;
  uint8_t  FrontUnk3;
  uint16_t unused1;
  uint16_t BackVendorID;
  uint16_t BackFwVersion;
  uint16_t BackConfRev;
  uint8_t  BackUnk2;
  uint8_t  BackUnk3;
  uint16_t unused2;
} SceKernelTouchpanelDeviceInfo2;

typedef struct ScePsCode {
  uint16_t company_code;
  uint16_t product_code;  
  uint16_t product_sub_code;   
  uint16_t factory_code;
} ScePsCode;

int _vshSblAimgrGetPscode(ScePsCode *pData);

int ksceSysconGetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);

int ksceSblLicMgrGetLicenseStatus(void);

int (* sceSblNvsReadDataForKernel)(uint32_t offset, char *buffer, uint32_t size);
int (* SceKernelSuspendForDriver_4DF40893)(int a1);
int (* SceKernelSuspendForDriver_2BB92967)(int a1);

//int (*module_get_export_func)(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);
int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

//int ksceBtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo * info, SceSize info_size);

typedef struct SceDipsw { 
  uint32_t cp_timestamp_1;       // 0..
  uint16_t cp_version;           //
  uint16_t cp_build_id;          //
  uint32_t cp_timestamp_2;       //    ..95
  uint32_t aslr_seed;            //  96-127
  uint32_t sce_sdk_flags;        // 128-159
  uint32_t shell_flags;          // 160-191
  uint32_t debug_control_flags;  // 192-223
  uint32_t system_control_flags; // 224-255
} SceDipsw; 

typedef struct SceKernelPARange { 
  uint32_t addr;
  SceSize  size;
} SceKernelPARange; 

typedef struct SceOpenPsId { 
  uint8_t open_psid[0x10];
} SceOpenPsId; 

typedef struct SceKblParam { // https://docs.vitasdk.org/group__SceKblKernel.html#structSceKblParam
  uint16_t   version;  
  uint16_t   size;
  uint32_t   current_fw_version;
  uint32_t   factory_fw_version;
  uint32_t   unk_C;
  uint32_t   unk_10;
  uint8_t   unk_14[0xC];
  uint8_t   qa_flags[0x10];
  uint8_t   boot_flags[0x10];
  SceDipsw   dipsw;
  SceKernelPARange   dram;
  uint32_t   unk_68;
  uint32_t   boot_type_indicator_1;
  SceOpenPsId   openpsid;
  SceKernelPARange   secure_kernel;
  SceKernelPARange   context_auth_sm;
  SceKernelPARange   kprx_auth_sm;
  SceKernelPARange   prog_rvk;
  ScePsCode   pscode;
  uint32_t   __stack_chk_guard;
  uint32_t   unk_AC;
  uint8_t   session_id[0x10];
  uint32_t   unk_C0;
  uint32_t   wakeup_factor;
  uint32_t   unk_C8;
  uint32_t   hold_ctrl;
  uint32_t   resume_context_addr;
  uint32_t   hardware_info;
  uint32_t   boot_type_indicator_2;
  uint32_t   unk_DC;
  uint32_t   unk_E0;
  uint32_t   unk_E4;
  uint8_t   hardware_flags[0x10];
  uint32_t   bootldr_revision;
  uint32_t   magic;
  uint8_t   coredump_session_key[0x20];
  uint8_t   unused[0xE0];
} __attribute__((packed)) SceKblParam;

/* typedef struct SceMsId { // size is 8 bytes
    SceUInt8 unk_0x0; // maybe manufacture code. ex: 0x20.
    SceUInt16 manuf_year;
    SceUInt8 manuf_month;
    SceUInt8 manuf_day;
    SceUInt8 manuf_hour;
    SceUInt8 manuf_minute;
    SceUInt8 manuf_second;
} SceMsId;

typedef struct SceMsInfo_ { // size is 0x24 on FW 0.990-3.01, 0x40 on FW 3.10-3.60
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
} SceMsInfo; */

typedef struct  __attribute__((packed)) {
  SceUInt32 signature;
  SceUInt32 version;
  SceUInt32 fields_table_offs;
  SceUInt32 values_table_offs;
  SceUInt32 nitems;
} SFOHeader;

typedef struct __attribute__((packed)) {
  SceUInt16 field_offs;
  SceUInt8  unk;
  SceUInt8  type; // 0x2 -> string, 0x4 -> number
  SceUInt32 unk2;
  SceUInt32 unk3;
  SceUInt16 val_offs;
  SceUInt16 unk4;
} SFODir;


typedef struct EmmcCardId { // https://github.com/mfdl/mmcCIDParser/blob/main/structures.h#L17 / https://problemkaputt.de/gbatek-dsi-sd-mmc-protocol-cid-register-128bit-card-identification.htm
  uint8_t unused : 1; // "stop bit"
  uint8_t crc7 : 7;
  uint8_t mdt : 8; // Manufacturer date  (myh) (m=1..12, y=0..15; +1997)
  uint32_t psn : 32; // Serial Number
  uint8_t prv : 8; // Product revision (BCD, 00h-99h) (eg 62h = rev 6.2)
  uint8_t pnm[6]; // Product name 
  uint8_t oid : 8; // OEM/Application ID
  uint8_t cbx : 2; // Device  -> Card/BGA Index (0=Card, 1=BGA, 2=POP "Package-on-Package")
  uint8_t reserved : 6;
  uint8_t mid : 8; // Manufacturer ID
} __attribute__((packed)) EmmcCardId;
static_assert(sizeof(EmmcCardId) == 0x10); // must be 0x10

typedef struct sd_context_part_base { // https://wiki.henkaku.xyz/vita/SceSdif
   uint32_t *gctx_ptr; // struct sd_context_global* gctx_ptr;
   uint32_t unk_4;
   uint32_t def_sector_size_mmc; // looks like default sector size - used in mmc read/write commands for resp_block_size_24
   uint32_t def_sector_size_sd; // looks like default sector size - used in sd read/write commands for resp_block_size_24
   //uint8_t unk_10; // can be padding
   uint8_t CID[16]; // this is CID data but in reverse
   //uint8_t unk_20; // can be padding
   uint8_t CSD[16]; // this is CSD data but in reverse
} sd_context_part_base;

typedef struct sd_context_part_mmc { // size is 0x398
   sd_context_part_base ctxb;
   uint8_t EXT_CSD[0x200]; // 0x30
   uint8_t data_230[0x160];
   void* unk_390;
   uint32_t unk_394;
} sd_context_part_mmc;


////////////////////////////////////////

int psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo);
int k_psvident_syscon_GetTouchpanelDeviceInfo(SceKernelTouchpanelDeviceInfo *pInfo);

int psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);
int k_psvident_syscon_GetTouchpanelDeviceInfo2(SceKernelTouchpanelDeviceInfo2 *pInfo);

int psvident_syscon_GetBaryonVersion(int *var);
int k_psvident_syscon_GetBaryonVersion(int *var);

int psvident_syscon_GetBaryonTimestamp(unsigned long long *var);
int k_psvident_syscon_GetBaryonTimestamp(unsigned long long *var);


int psvident_kblparam_GetBuffer(SceKblParam *kblparam);
int k_psvident_kblparam_GetBuffer(SceKblParam *kblparam);

int psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo);
int k_psvident_sysroot_GetBatteryVersion(unsigned long long int *hwinfo, unsigned int *fwinfo, unsigned int *dfinfo);

int psvident_pervasive_GetSoCRevision(unsigned int *soc);
int k_psvident_pervasive_GetSoCRevision(unsigned int *soc);

int psvident_mbr_Get(SceMbr *master);
int k_psvident_mbr_Get(SceMbr *master);

int psvident_mbr_CheckEnso(unsigned int *var);
int k_psvident_mbr_CheckEnso(unsigned int *var);

int psvident_mbr_CheckPreviousEnso(unsigned int *var);
int k_psvident_mbr_CheckPreviousEnso(unsigned int *var);

int psvident_GetActivationStatus(void);
int k_psvident_GetActivationStatus(void);

int psvident_nvs_Get(int offset, uint8_t *buffer, int size);
int k_psvident_nvs_Get(int offset, uint8_t *buffer, int size);

int psvident_BtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo *info, SceSize info_size);
int k_psvident_BtGetRegisteredInfo(int device, int unk, SceBtRegisteredInfo *info, SceSize info_size);

int psvident_sdifgetcontextmmc(EmmcCardId *cardid, uint8_t *cid, uint8_t *csd, uint8_t *ext_csd);
int k_psvident_sdifgetcontextmmc(EmmcCardId *cardid, uint8_t *cid, uint8_t *csd, uint8_t *ext_csd);

int psvident_detect_plugins(int mode, int no, SceKernelModuleInfo *modinfo);
int k_psvident_detect_plugins(int mode, int no, SceKernelModuleInfo *modinfo);

int psvident_test(int *test);
int k_psvident_test(int *test);


////////////////////////////////////////

#endif
