//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "templates.h"
#include "TxdStore.h"
#include "2dEffect.h"
#include "BaseModelInfo.h"
#include "ModelInfo.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)
#include "ColModel.h"
#include "Pools.h" // rouz edit (ChatGPT)

CBaseModelInfo::CBaseModelInfo(ModelInfoType type)
{
	m_colModel = nil;
	m_2dEffectsID = -1;
	m_objectId = -1;
	m_refCount = 0;
	m_txdSlot = -1;
	m_type = type;
	m_num2dEffects = 0;
	m_bOwnsColModel = false;
}

void
CBaseModelInfo::Shutdown(void)
{
	DeleteCollisionModel();
	DeleteRwObject();
	// Reset the model effect range through the C policy
	ModelRegistry_ResetEffects(this);
	m_txdSlot = -1;
	m_objectId = -1;
}

void
CBaseModelInfo::DeleteCollisionModel(void)
{
	if(m_colModel && m_bOwnsColModel){
		if(m_colModel)
//+ rouz edit (ChatGPT)
			// Destroy and release the collision model without invoking C++ delete.
		{
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			ColModel_Destroy(m_colModel);
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPool_Delete(CPools::GetColModelPool(), m_colModel);
			//- rouz edit (ChatGPT)
		}
//- rouz edit (ChatGPT)
		m_colModel = nil;
	}
}

void
CBaseModelInfo::AddRef(void)
{
	m_refCount++;
	AddTexDictionaryRef();
}

void
CBaseModelInfo::RemoveRef(void)
{
	m_refCount--;
	RemoveTexDictionaryRef();
}

void
CBaseModelInfo::SetTexDictionary(const char *name)
{
	// Use the complete C texture dictionary owner with the original live arguments
	int slot = TxdStore_FindTxdSlot(name);
	if(slot == -1)
		// Use the complete C texture dictionary owner with the original live arguments
		slot = TxdStore_AddTxdSlot(name);
	m_txdSlot = slot;
}

void
CBaseModelInfo::AddTexDictionaryRef(void)
{
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(m_txdSlot);
}

void
CBaseModelInfo::RemoveTexDictionaryRef(void)
{
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveRef(m_txdSlot);
}

int16_t *ModelRegistry_EffectId(void *model)
{
    // Borrow the original model effect identifier
    return &((CBaseModelInfo*)model)->m_2dEffectsID;
}

uint8_t *ModelRegistry_EffectCount(void *model)
{
    // Borrow the original model effect count
    return &((CBaseModelInfo*)model)->m_num2dEffects;
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
