//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "FileMgr.h"
#include "FileMgr_backend.h"
#include "PlatformFiles.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#define FileMgr_CaseOpen fopen
#else
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
#include "CasePath.h"
#define FileMgr_CaseOpen _fcaseopen
#endif
#ifndef MASTER
#define FileMgr_Assert(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define FileMgr_Assert(expr) (expr)
#endif

/*
 * Windows FILE is BROKEN for GTA.
 *
 * We need to support mapping between LF and CRLF for text files
 * but we do NOT want to end the file at the first sight of a SUB character.
 * So here is a simple implementation of a FILE interface that works like GTA expects.
 */

typedef struct myFILE
{
	bool isText;
	FILE *file;
} myFILE;

#define NUMFILES 20
static myFILE myfiles[NUMFILES];


#if !defined(_WIN32)
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#define _getcwd getcwd

// Case-insensitivity on linux (from https://github.com/OneSadCookie/fcaseopen)
static void mychdir(char const *path)
{
	// Resolve the original non-Windows path before changing the current directory
	char* r = casepath(path, false);
    if (r) {
        chdir(r);
		free(r);
    } else {
        errno = ENOENT;
    }
}
#else
#define mychdir chdir
#endif

/* Force file to open as binary but remember if it was text mode */
static int
myfopen(const char *filename, const char *mode)
{
	// Reserve the first free slot and force binary CRT mode while remembering text mode
	int fd;
	char realmode[10], *p;

	for(fd = 1; fd < NUMFILES; fd++)
		if(myfiles[fd].file == NULL)
			goto found;
	return 0;	// no free fd
found:
	myfiles[fd].isText = strchr(mode, 'b') == NULL;
	p = realmode;
	while(*mode)
		if(*mode != 't' && *mode != 'b')
			*p++ = *mode++;
		else
			mode++;
	*p++ = 'b';
	*p = '\0';
	
	myfiles[fd].file = FileMgr_CaseOpen(filename, realmode);
	if(myfiles[fd].file == NULL)
		return 0;
	return fd;
}

static int
myfclose(int fd)
{
	// Close an occupied slot and release it for reuse
	int ret;
	FileMgr_Assert(fd < NUMFILES);
	if(myfiles[fd].file){
		ret = fclose(myfiles[fd].file);
		myfiles[fd].file = NULL;
		return ret;
	}
	return EOF;
}

static int
myfgetc(int fd)
{
	// Translate CRLF to LF while retaining lone CR and embedded SUB bytes
	int c;
	c = fgetc(myfiles[fd].file);
	if(myfiles[fd].isText && c == 015){
		/* translate CRLF to LF */
		c = fgetc(myfiles[fd].file);
		if(c == 012)
			return c;
		ungetc(c, myfiles[fd].file);
		return 015;
	}
	return c;
}

static int
myfputc(int c, int fd)
{
	// Insert CR before LF only for text-mode output
	/* translate LF to CRLF */
	if(myfiles[fd].isText && c == 012)
		fputc(015, myfiles[fd].file);
	return fputc(c, myfiles[fd].file);
}

static char*
myfgets(char *buf, int len, int fd)
{
	// Read through the text adapter until newline, capacity or EOF
	int c;
	char *p;

	p = buf;
	len--;	// NUL byte
	while(len--){
		c = myfgetc(fd);
		if(c == EOF){
			if(p == buf)
				return NULL;
			break;
		}
		*p++ = c;
		if(c == '\n')
			break;
	}
	*p = '\0';
	return buf;
}

static size_t
myfread(void *buf, size_t elt, size_t n, int fd)
{
	// Read translated text bytes individually or delegate binary reads to the CRT
	if(myfiles[fd].isText){
		unsigned char *p;
		size_t i;
		int c;

		n *= elt;
		p = (unsigned char*)buf;
		for(i = 0; i < n; i++){
			c = myfgetc(fd);
			if(c == EOF)
				break;
			*p++ = (unsigned char)c;
		}
		return i / elt;
	}
	return fread(buf, elt, n, myfiles[fd].file);
}

static size_t
myfwrite(void *buf, size_t elt, size_t n, int fd)
{
	// Write translated text bytes individually or delegate binary writes to the CRT
	if(myfiles[fd].isText){
		unsigned char *p;
		size_t i;
		int c;

		n *= elt;
		p = (unsigned char*)buf;
		for(i = 0; i < n; i++){
			c = *p++;
			myfputc(c, fd);
			if(feof(myfiles[fd].file))	// is this right?
				break;
		}
		return i / elt;
	}
	return fwrite(buf, elt, n, myfiles[fd].file);
}

static int
myfseek(int fd, long offset, int whence)
{
	// Return the original raw CRT seek status
	return fseek(myfiles[fd].file, offset, whence);
}

static int
myfeof(int fd)
{
	// Report EOF using the original feof operation
	return feof(myfiles[fd].file);
//	return ferror(myfiles[fd].file);
}


static char FileMgr_rootDirName[128] = {'\0'};
static char FileMgr_dirName[128];

void
FileMgr_Initialise(void)
{
	// Capture the current root directory and append the original backslash
	_getcwd(FileMgr_rootDirName, 128);
	strcat(FileMgr_rootDirName, "\\");
}

void
FileMgr_ChangeDir(const char *dir)
{
	// Extend or reset the persistent directory buffer before changing directories
	if(*dir == '\\'){
		strcpy(FileMgr_dirName, FileMgr_rootDirName);
		dir++;
	}
	if(*dir != '\0'){
		strcat(FileMgr_dirName, dir);
		// BUG in the game it seems, it's off by one
		if(dir[strlen(dir)-1] != '\\')
			strcat(FileMgr_dirName, "\\");
	}
	mychdir(FileMgr_dirName);
}

void
FileMgr_SetDir(const char *dir)
{
	// Build the requested directory from the live root buffer
	strcpy(FileMgr_dirName, FileMgr_rootDirName);
	if(*dir != '\0'){
		strcat(FileMgr_dirName, dir);
		// BUG in the game it seems, it's off by one
		if(dir[strlen(dir)-1] != '\\')
			strcat(FileMgr_dirName, "\\");
	}
	mychdir(FileMgr_dirName);
}

void
FileMgr_SetDirMyDocuments(void)
{
	// Restore the root before querying and entering the platform user directory
	FileMgr_SetDir("");	// better start at the root if user directory is relative
	mychdir(_psGetUserFilesFolder());
}

FileMgrSize
FileMgr_LoadFile(const char *file, uint8_t *buf, int maxlen, const char *mode)
{
	// Read the original fixed-size chunks and append the terminating NUL
	int fd;
	FileMgrSize n, len;

	fd = myfopen(file, mode);
	if(fd == 0)
		return -1;
	len = 0;
	do{
		n = myfread(buf + len, 1, 0x4000, fd);
#ifndef FIX_BUGS
		if (n < 0)
			return -1;
#endif
		len += n;
		FileMgr_Assert(len < maxlen);
	}while(n == 0x4000);
	buf[len] = 0;
	myfclose(fd);
	return len;
}

int
FileMgr_OpenFile(const char *file, const char *mode)
{
	// Open the requested mode through the original slot allocator
	return myfopen(file, mode);
}

int
FileMgr_OpenFileForWriting(const char *file)
{
	// Retain the original binary-write convenience operation
	return FileMgr_OpenFile(file, "wb");
}

size_t
FileMgr_Read(int fd, const char *buf, FileMgrSize len)
{
	// Read into the original caller buffer despite its const-qualified API
	return myfread((void*)buf, 1, len, fd);
}

size_t
FileMgr_Write(int fd, const char *buf, FileMgrSize len)
{
	// Write the original caller bytes through the text or binary adapter
	return myfwrite((void*)buf, 1, len, fd);
}

bool
FileMgr_Seek(int fd, int offset, int whence)
{
	// Retain the original true-on-failure seek convention
	return !!myfseek(fd, offset, whence);
}

bool
FileMgr_ReadLine(int fd, char *buf, int len)
{
	// Report whether the original bounded line reader returned a buffer
	return myfgets(buf, len, fd) != NULL;
}

int
FileMgr_CloseFile(int fd)
{
	// Release the original file slot and return its close status
	return myfclose(fd);
}

int
FileMgr_GetErrorReadWrite(int fd)
{
	// Retain the original EOF query despite the method name
	return myfeof(fd);
}

int
FileMgr_OpenFileDefault(const char *file)
{
	// Preserve the original one-argument overload's binary-read mode
	return FileMgr_OpenFile(file, "rb");
}

char *
FileMgr_GetRootDirName(void)
{
	// Return the original live root buffer without copying it
	return FileMgr_rootDirName;
}
//- rouz edit (ChatGPT)
