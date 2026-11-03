//+ rouz edit (ChatGPT)
#include "config.h"
#ifndef _WIN32
#include "PlatformFind.h"
#include "CasePath.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

HANDLE FindFirstFile(const char* pathname, WIN32_FIND_DATA* firstfile) {
    // Preserve the original directory search behavior
	char pathCopy[MAX_PATH];
	strcpy(pathCopy, pathname);

	char *folder = strtok(pathCopy, "*");
	char *extension = strtok(NULL, "*");

    // because I remember like strtok might not return NULL for last delimiter
    if (extension && extension - folder == strlen(pathname))
        extension = NULL;
	
	// Case-sensitivity and backslashes...
    // Will be freed at the bottom
    char *realFolder = casepath(folder, true); // rouz edit (ChatGPT)
	if (realFolder) {
        folder = realFolder;
	}

	strncpy(firstfile->folder, folder, sizeof(firstfile->folder));

	if (extension)
		strncpy(firstfile->extension, extension, sizeof(firstfile->extension));
	else
		firstfile->extension[0] = '\0';

    if (realFolder)
        free(realFolder);

	HANDLE d;
	if ((d = (HANDLE)opendir(firstfile->folder)) == NULL || !FindNextFile(d, firstfile))
		return NULL;

	return d;
}

bool FindNextFile(HANDLE d, WIN32_FIND_DATA* finddata) {
    // Preserve the original directory iteration and metadata reads
	struct dirent *file;
	static struct stat fileStats;
	static char path[PATH_MAX], relativepath[NAME_MAX + sizeof(finddata->folder) + 1];
	int extensionLen = strlen(finddata->extension);
	while ((file = readdir((DIR*)d)) != NULL) {

		// We only want "DT_REG"ular Files, but reportedly some FS and OSes gives DT_UNKNOWN as type.
		if ((file->d_type == DT_UNKNOWN || file->d_type == DT_REG || file->d_type == DT_LNK) &&
			(extensionLen == 0 || strncasecmp(&file->d_name[strlen(file->d_name) - extensionLen], finddata->extension, extensionLen) == 0)) {

			sprintf(relativepath, "%s/%s", finddata->folder, file->d_name);
			realpath(relativepath, path);
			stat(path, &fileStats);
			strncpy(finddata->cFileName, file->d_name, sizeof(finddata->cFileName));
			finddata->ftLastWriteTime = fileStats.st_mtime;
			return true;
		}
	}
	return false;
}

#endif
//- rouz edit (ChatGPT)
