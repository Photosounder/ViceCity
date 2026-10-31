//+ rouz edit (ChatGPT)
#include "Directory.h"
#include "General.h"
#include "FileMgr.h"
#include <stdlib.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#ifndef MASTER
#define DIRECTORY_ASSERT(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define DIRECTORY_ASSERT(expr) (expr)
#endif

void
Directory_Initialize(CDirectory *directory, int32_t capacity)
{
	// Initialize capacity before count and allocate the original entry array
	directory->maxEntries = capacity;
	directory->numEntries = 0;
	directory->entries = (DirectoryInfo*)malloc(sizeof(DirectoryInfo)*directory->maxEntries); // rouz edit (ChatGPT)
}

void
Directory_Shutdown(CDirectory *directory)
{
	// Release only the original entry allocation
	free(directory->entries); // rouz edit (ChatGPT)
}

void
Directory_ReadDirFile(CDirectory *directory, const char *filename)
{
	// Read the original fixed-size records and add every positive-length read
	int fd;
	DirectoryInfo dirinfo;

	fd = FileMgr_OpenFile(filename, "rb");
	while(FileMgr_Read(fd, (char*)&dirinfo, sizeof(dirinfo)))
		Directory_AddItem(directory, &dirinfo);
	FileMgr_CloseFile(fd);
}

bool
Directory_WriteDirFile(CDirectory *directory, const char *filename)
{
	// Write the original contiguous records and compare the returned byte count
	int fd;
	size_t n;
	fd = FileMgr_OpenFileForWriting(filename);
	n = FileMgr_Write(fd, (char*)directory->entries, directory->numEntries*sizeof(DirectoryInfo));
	FileMgr_CloseFile(fd);
	return n == directory->numEntries*sizeof(DirectoryInfo);
}

void
Directory_AddItem(CDirectory *directory, const DirectoryInfo *dirinfo)
{
	// Retain the capacity assertion, duplicate check and record-copy order
	DIRECTORY_ASSERT(directory->numEntries < directory->maxEntries);
#ifdef FIX_BUGS
	// don't add if already exists
	uint32_t offset, size;
	if(Directory_FindItem(directory, dirinfo->name, &offset, &size))
		return;
#endif
	directory->entries[directory->numEntries++] = *dirinfo;
}

void
Directory_AddItemWithImage(CDirectory *directory, const DirectoryInfo *dirinfo, int32_t imgId)
{
	// Copy the input record before applying the original image bits
	DirectoryInfo di = *dirinfo;
	di.offset |= imgId<<24;
	Directory_AddItem(directory, &di);
}

bool
Directory_FindItem(CDirectory *directory, const char *name, uint32_t *offset, uint32_t *size)
{
	// Find the first case-insensitive match and write offset before size
	int i;

	for(i = 0; i < directory->numEntries; i++)
		if(!General_faststricmp(directory->entries[i].name, name)){
			*offset = directory->entries[i].offset;
			*size = directory->entries[i].size;
			return true;
		}
	return false;
}
//- rouz edit (ChatGPT)
