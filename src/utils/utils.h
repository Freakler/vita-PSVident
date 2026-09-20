#ifndef __UTILS_H__
#define __UTILS_H__

#define NET_INIT_SIZE 1 * 1024 * 1024
#define NET_CTL_ERROR_NOT_TERMINATED 0x80412102

//#define GETBIT(var, bit)  (((var) >> (bit)) & 1)

enum {
  FALSE,
  TRUE
};

void initSceAppUtil();
void closeSceAppUtil();

int initNet();
int doesFileExist(const char *file);
int doesDirExist(const char *file);
char* getFileDateModify(const char* file);
void aes256cbc(uint8_t *key, uint8_t *iv_in, uint8_t *in, uint16_t len, uint8_t *out);

void convert2hex(unsigned char *buf, char *buf2, int len);
void firmware_string(char string[8], unsigned int version);
void baryon_string(char string[32], unsigned int version);
void model_string(char out[32], char in[32]);

void getSizeString(char string[16], uint64_t size);
int logPrintf(char *file, const uint8_t *text, ...);

void ascii2utf(uint16_t* dst, char* src);
void sendNotification(char *text, ...);

const char *part_id(int code);
const char *part_type(int type);
const char *part_acl(short acl);

void removeChar(char* s, char c);

#endif