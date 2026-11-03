//+ rouz edit (ChatGPT)
#include "RwHelper.h"
#include "PlayerSkin_backend.h"
#include "SharedText.h"
#include "FileMgr.h"
#include "Directory.h"
#include "Timer.h"
#include "Draw.h"
#include <stdio.h>
#include <string.h>
PlayerSkinClump *PlayerSkin_Clump;
float PlayerSkin_OldFov;
int32_t PlayerSkin_txdSlot;

void
PlayerSkin_FindPlayerDff(uint32_t *offset, uint32_t *size)
{
    // Preserve the complete original state writes, native call order and branch decisions
	int file;
	DirectoryInfo info;

	file = FileMgr_OpenFile("models\\gta3.dir", "rb");

	do {
		// Preserve the original conditional operation and traversal order
		if (!FileMgr_Read(file, (char*)&info, sizeof(DirectoryInfo)))
			return;
	} while (PlayerSkin_BackendCompare("player.dff", info.name) != 0);

	*offset = info.offset;
	*size = info.size;
}

static void
PlayerSkin_ReadMemory(void *memory)
{
    // Run the complete original stream sequence while the native memory owner remains alive
	void *stream = PlayerSkin_BackendOpenStream(memory);

	if (PlayerSkin_BackendFindChunk(stream))
		PlayerSkin_Clump = PlayerSkin_BackendReadClump(stream);

	PlayerSkin_BackendCloseStream(stream,memory);
}

void
PlayerSkin_LoadPlayerDff(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	uint32_t offset, size;
	uint8_t *buffer;
	bool streamWasAdded = false;

	if (PlayerSkin_BackendImageCount() == 0) {
		// Preserve the original conditional operation and traversal order
		PlayerSkin_BackendAddImage("models\\gta3.img");
		streamWasAdded = true;
	}

	PlayerSkin_FindPlayerDff(&offset, &size);
	buffer = PlayerSkin_BackendAllocate(size << 11, 2048);
	PlayerSkin_BackendReadSectors(buffer, offset, size);
	PlayerSkin_BackendSync();

	PlayerSkin_BackendWithMemory(buffer,size << 11,PlayerSkin_ReadMemory);

	PlayerSkin_BackendFree(buffer);

	if (streamWasAdded)
		PlayerSkin_BackendRemoveImages();
}

void
PlayerSkin_Initialise(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	// empty on PS2
	PlayerSkin_txdSlot = PlayerSkin_BackendAddSlot("skin");
	PlayerSkin_BackendCreateDictionary(PlayerSkin_txdSlot);
	PlayerSkin_BackendAddRef(PlayerSkin_txdSlot);
}

void
PlayerSkin_Shutdown(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	// empty on PS2
	PlayerSkin_BackendRemoveSlot(PlayerSkin_txdSlot);
}

PlayerSkinTexture *
PlayerSkin_GetSkinTexture(const char *texName)
{
    // Preserve the complete original state writes, native call order and branch decisions
	PlayerSkinTexture *tex;
	void *raster;
	int32_t width, height, depth, format;

	PlayerSkin_BackendPushDictionary();
	PlayerSkin_BackendSetDictionary(PlayerSkin_txdSlot);
	tex = RwHelper_ReadTexture(texName, NULL);
	PlayerSkin_BackendPopDictionary();
	if (tex != NULL) return tex;

	if (strcmp(DEFAULT_SKIN_NAME, texName) == 0 || texName[0] == '\0')
		sprintf(gString, "models\\generic\\player.bmp");
	else
		sprintf(gString, "skins\\%s.bmp", texName);

	void *image = PlayerSkin_BackendReadImage(gString);
	if (image) {
		// Preserve the original conditional operation and traversal order
		PlayerSkin_BackendRasterFormat(image, &width, &height, &depth, &format);
		raster = PlayerSkin_BackendCreateRaster(width, height, depth, format);
		PlayerSkin_BackendRasterImage(raster, image);

		tex = PlayerSkin_BackendCreateTexture(raster);
		PlayerSkin_BackendTextureName(tex, texName);
		PlayerSkin_BackendTextureFilter(tex);
		PlayerSkin_BackendAddTexture(PlayerSkin_txdSlot,tex);

		PlayerSkin_BackendDestroyImage(image);
	}
	return tex;
}

void
PlayerSkin_BeginFrontendSkinEdit(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	PlayerSkin_LoadPlayerDff();
	PlayerSkin_BackendSetRenderers(PlayerSkin_Clump);
	PlayerSkin_BackendLoadPlayer();
	PlayerSkin_OldFov = Draw_GetFOV();
	Draw_SetFOV(30.0f);
}

void
PlayerSkin_EndFrontendSkinEdit(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	PlayerSkin_BackendDestroyClump(PlayerSkin_Clump);
	PlayerSkin_Clump = NULL;
	Draw_SetFOV(PlayerSkin_OldFov);
}

void
PlayerSkin_RenderFrontendSkinEdit(void)
{
    // Preserve the complete original state writes, native call order and branch decisions
	static float rotation = 0.0f;
	PlayerSkinColor AmbientColor = { 0.65f, 0.65f, 0.65f, 1.0f };
	const PlayerSkinVector pos = { 1.35f, 0.35f, 7.725f };
	const PlayerSkinVector axis = { 0.0f, 1.0f, 0.0f };
	static uint32_t LastFlash = 0;

	void *frame = PlayerSkin_BackendFrame(PlayerSkin_Clump);

	if (Timer_GetTimeInMillisecondsPauseMode() - LastFlash > 7) {
		// Preserve the original conditional operation and traversal order
		rotation += 2.0f;
		if (rotation > 360.0f)
			rotation -= 360.0f;
		LastFlash = Timer_GetTimeInMillisecondsPauseMode();
	}
	PlayerSkin_BackendTransform(frame);
	PlayerSkin_BackendTranslate(frame,pos);
	PlayerSkin_BackendRotate(frame,axis,rotation);
	PlayerSkin_BackendUpdateObjects(frame);
	PlayerSkin_BackendAmbient(AmbientColor);
	PlayerSkin_BackendRenderClump(PlayerSkin_Clump);
}
//- rouz edit (ChatGPT)
