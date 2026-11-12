//+ rouz edit (Claude)
#include <stddef.h>
#include "BaseModelInfo.h"
#include "TxdStore.h"

void BaseModelInfo_Init(CBaseModelInfoData *self, int type)
{
	// Start with no collision model, effects, object, references or texture slot
	self->m_colModel = NULL;
	self->m_2dEffectsID = -1;
	self->m_objectId = -1;
	self->m_refCount = 0;
	self->m_txdSlot = -1;
	self->m_type = (uint8_t)type;
	self->m_num2dEffects = 0;
	self->m_bOwnsColModel = false;
}

void BaseModelInfo_ResetAfterShutdown(CBaseModelInfoData *self)
{
	// Forget the texture slot and object after a shutdown
	self->m_txdSlot = -1;
	self->m_objectId = -1;
}

void BaseModelInfo_AddRef(CBaseModelInfoData *self)
{
	// Count the reference and keep the texture dictionary loaded
	self->m_refCount++;
	BaseModelInfo_AddTexDictionaryRef(self);
}

void BaseModelInfo_RemoveRef(CBaseModelInfoData *self)
{
	// Drop the reference and release the texture dictionary
	self->m_refCount--;
	BaseModelInfo_RemoveTexDictionaryRef(self);
}

void BaseModelInfo_SetTexDictionary(CBaseModelInfoData *self, const char *name)
{
	// Find or create the texture slot for this name
	int slot = TxdStore_FindTxdSlot(name);
	if (slot == -1)
		slot = TxdStore_AddTxdSlot(name);
	self->m_txdSlot = (int16_t)slot;
}

void BaseModelInfo_AddTexDictionaryRef(CBaseModelInfoData *self)
{
	// Add a reference to the model's texture slot
	TxdStore_AddRef(self->m_txdSlot);
}

void BaseModelInfo_RemoveTexDictionaryRef(CBaseModelInfoData *self)
{
	// Remove a reference from the model's texture slot
	TxdStore_RemoveRef(self->m_txdSlot);
}
//- rouz edit (Claude)
