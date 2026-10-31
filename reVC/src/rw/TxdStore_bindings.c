//+ rouz edit (ChatGPT)
#include "TxdStore.h"
#include "TxdStore_bindings.h"
void BulletTraces_BackendPushTxd(void)
{
    // Preserve the original texture-dictionary push
    TxdStore_PushCurrentTxd();
}
int32_t BulletTraces_BackendFindTxd(const char *name)
{
    // Find the original named texture dictionary
    return TxdStore_FindTxdSlot(name);
}
void BulletTraces_BackendSetTxd(int32_t slot)
{
    // Select the original texture dictionary
    TxdStore_SetCurrentTxd(slot);
}
void BulletTraces_BackendPopTxd(void)
{
    // Restore the original texture dictionary
    TxdStore_PopCurrentTxd();
}
void Clouds_BackendPushTxd(void)
{
    // Retain the original typed owner operation at its original C call point
    TxdStore_PushCurrentTxd();
}
int32_t Clouds_BackendFindTxd(const char *name)
{
    // Retain the original typed owner operation at its original C call point
    return TxdStore_FindTxdSlot(name);
}
void Clouds_BackendSetTxd(int32_t slot)
{
    // Retain the original typed owner operation at its original C call point
    TxdStore_SetCurrentTxd(slot);
}
void Clouds_BackendPopTxd(void)
{
    // Retain the original typed owner operation at its original C call point
    TxdStore_PopCurrentTxd();
}
int32_t Markers3D_BackendFindTxd(const char *name)
{
    // Find the original particle texture dictionary before pushing it
    return TxdStore_FindTxdSlot(name);
}
void Markers3D_BackendPushTxd(void)
{
    // Push the original current dictionary
    TxdStore_PushCurrentTxd();
}
void Markers3D_BackendSetTxd(int32_t slot)
{
    // Set the original selected dictionary
    TxdStore_SetCurrentTxd(slot);
}
void Markers3D_BackendPopTxd(void)
{
    // Restore the original dictionary after both resource loads
    TxdStore_PopCurrentTxd();
}
int32_t PlayerSkin_BackendAddSlot(const char *name)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return TxdStore_AddTxdSlot(name);
}
void PlayerSkin_BackendCreateDictionary(int32_t slot)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_Create(slot);
}
void PlayerSkin_BackendAddRef(int32_t slot)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_AddRef(slot);
}
void PlayerSkin_BackendRemoveSlot(int32_t slot)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_RemoveTxdSlot(slot);
}
void PlayerSkin_BackendPushDictionary(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_PushCurrentTxd();
}
void PlayerSkin_BackendSetDictionary(int32_t slot)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_SetCurrentTxd(slot);
}
void PlayerSkin_BackendPopDictionary(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    TxdStore_PopCurrentTxd();
}
void Rubbish_BackendTxdPush(void)
{
	// Save the original current texture dictionary
	TxdStore_PushCurrentTxd();
}
int Rubbish_BackendTxdFind(const char *name)
{
	// Find the original particle dictionary
	return TxdStore_FindTxdSlot(name);
}
void Rubbish_BackendTxdSet(int slot)
{
	// Select the original texture dictionary slot
	TxdStore_SetCurrentTxd(slot);
}
void Rubbish_BackendTxdPop(void)
{
	// Restore the original current texture dictionary
	TxdStore_PopCurrentTxd();
}
void Skidmarks_BackendTxdPush(void)
{
	// Save the original current texture dictionary
	TxdStore_PushCurrentTxd();
}
int Skidmarks_BackendTxdFind(const char *name)
{
	// Find the original named texture dictionary
	return TxdStore_FindTxdSlot(name);
}
void Skidmarks_BackendTxdSet(int slot)
{
	// Select the same original dictionary slot
	TxdStore_SetCurrentTxd(slot);
}
void Skidmarks_BackendTxdPop(void)
{
	// Restore the original current texture dictionary
	TxdStore_PopCurrentTxd();
}
//- rouz edit (ChatGPT)
