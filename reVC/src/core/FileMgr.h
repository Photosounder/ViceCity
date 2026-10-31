//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T FileMgrSize;
#elif defined(__MWERKS__)
typedef ptrdiff_t FileMgrSize;
#else
#include <sys/types.h>
typedef ssize_t FileMgrSize;
#endif
#ifdef __cplusplus
extern "C" {
#endif
void FileMgr_Initialise(void);
void FileMgr_ChangeDir(const char *dir);
void FileMgr_SetDir(const char *dir);
void FileMgr_SetDirMyDocuments(void);
FileMgrSize FileMgr_LoadFile(const char *file, uint8_t *buf, int maxlen, const char *mode);
int FileMgr_OpenFile(const char *file, const char *mode);
int FileMgr_OpenFileDefault(const char *file);
int FileMgr_OpenFileForWriting(const char *file);
size_t FileMgr_Read(int fd, const char *buf, FileMgrSize len);
size_t FileMgr_Write(int fd, const char *buf, FileMgrSize len);
bool FileMgr_Seek(int fd, int offset, int whence);
bool FileMgr_ReadLine(int fd, char *buf, int len);
int FileMgr_CloseFile(int fd);
int FileMgr_GetErrorReadWrite(int fd);
char *FileMgr_GetRootDirName(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
