//+ rouz edit (ChatGPT)
#include "TexList.h"
#include "FileMgr.h"
#include <stddef.h>
bool TexList_ms_nTexUsed[TEXLIST_MAX_TEXUSED];

void
TexList_Initialise(void)
{
    // Preserve the original empty initialization without resetting usage flags
}

void
TexList_Shutdown(void)
{
    // Preserve the original empty shutdown without changing usage flags
}

TexListTexture *
TexList_SetTexture(int32_t slot, char *name)
{
    // Preserve the original null texture result without reading the name or changing slot flags
    (void)slot;(void)name;
	return NULL;
}

int32_t
TexList_GetFirstFreeTexture(void)
{
    // Find the first unused flag in the original ascending slot order
	for (int32_t i = 0; i < TEXLIST_MAX_TEXUSED; i++)
		if (!TexList_ms_nTexUsed[i])
			return i;
	return -1;
}

TexListTexture *
TexList_LoadFileNameTexture(char *name)
{
    // Keep the original free-slot lookup before the unchanged texture setter
	return TexList_SetTexture(TexList_GetFirstFreeTexture(), name);
}

void
TexList_LoadGlobalTextureList(void)
{
    // Enter the same file-manager operation with the original directory string
	FileMgr_SetDir("TEXTURES");
}
//- rouz edit (ChatGPT)
