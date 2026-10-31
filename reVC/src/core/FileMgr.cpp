//+ rouz edit (ChatGPT)
#include "common.h"
#include "crossplatform.h"
#include "FileMgr_backend.h"

const char *_psGetUserFilesFolder();

const char *
FileMgr_BackendUserFilesFolder(void)
{
	// Query the original platform folder provider at the original call point
	return _psGetUserFilesFolder();
}

#ifndef _WIN32
char *
FileMgr_BackendCasePath(const char *path, bool checkPathFirst)
{
	// Retain the existing case-correcting path helper and its allocated result
	return casepath(path, checkPathFirst);
}

FILE *
FileMgr_BackendCaseOpen(const char *filename, const char *mode)
{
	// Retain the original case-insensitive platform file-open operation
	return fcaseopen(filename, mode);
}
#endif
//- rouz edit (ChatGPT)
