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
	// Delegate the field initialisation to the C implementation
	BaseModelInfo_Init(this, type);
}

void
CBaseModelInfo::Shutdown(void)
{
	DeleteCollisionModel();
	DeleteRwObject();
	// Reset the model effect range through the C policy
	ModelRegistry_ResetEffects(this);
	BaseModelInfo_ResetAfterShutdown(this);
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
			CPool_Delete(Pools_GetColModelPool(), m_colModel);
			//- rouz edit (ChatGPT)
		}
//- rouz edit (ChatGPT)
		m_colModel = nil;
	}
}

void
CBaseModelInfo::AddRef(void)
{
	// Delegate to the C implementation
	BaseModelInfo_AddRef(this);
}

void
CBaseModelInfo::RemoveRef(void)
{
	// Delegate to the C implementation
	BaseModelInfo_RemoveRef(this);
}

void
CBaseModelInfo::SetTexDictionary(const char *name)
{
	// Delegate to the C implementation
	BaseModelInfo_SetTexDictionary(this, name);
}

void
CBaseModelInfo::AddTexDictionaryRef(void)
{
	// Delegate to the C implementation
	BaseModelInfo_AddTexDictionaryRef(this);
}

void
CBaseModelInfo::RemoveTexDictionaryRef(void)
{
	// Delegate to the C implementation
	BaseModelInfo_RemoveTexDictionaryRef(this);
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
