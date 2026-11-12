//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#define MAX_MODEL_NAME (21)

#include "ModelInfoType.h" // rouz edit (ChatGPT)

#ifdef __cplusplus
struct CColModel;
struct C2dEffect; // rouz edit (ChatGPT)
#define BASEMODELINFO_COLMODEL_TYPE CColModel
#else
#define BASEMODELINFO_COLMODEL_TYPE void
#endif

// Plain data of CBaseModelInfo, the C++ class adds the virtual interface on top
typedef struct CBaseModelInfoData {
	char         m_name[MAX_MODEL_NAME];
	uint8_t      m_type;
	uint8_t      m_num2dEffects;
	bool         m_bOwnsColModel;
	BASEMODELINFO_COLMODEL_TYPE *m_colModel;
	int16_t      m_2dEffectsID;
	int16_t      m_objectId;
	uint16_t     m_refCount;
	int16_t      m_txdSlot;
} CBaseModelInfoData;

#ifdef __cplusplus
extern "C" {
#endif
void BaseModelInfo_Init(CBaseModelInfoData *self, int type);
void BaseModelInfo_ResetAfterShutdown(CBaseModelInfoData *self);
void BaseModelInfo_AddRef(CBaseModelInfoData *self);
void BaseModelInfo_RemoveRef(CBaseModelInfoData *self);
void BaseModelInfo_SetTexDictionary(CBaseModelInfoData *self, const char *name);
void BaseModelInfo_AddTexDictionaryRef(CBaseModelInfoData *self);
void BaseModelInfo_RemoveTexDictionaryRef(CBaseModelInfoData *self);
#ifdef __cplusplus
}

class CBaseModelInfo : public CBaseModelInfoData
{
    //+ rouz edit (ChatGPT)
    friend int16_t *ModelRegistry_EffectId(void *model);
    friend uint8_t *ModelRegistry_EffectCount(void *model);
    //- rouz edit (ChatGPT)
public:
	CBaseModelInfo(ModelInfoType type);
	virtual ~CBaseModelInfo() {}
	virtual void Shutdown(void);
	virtual void DeleteRwObject(void) = 0;
	virtual RwObject *CreateInstance(void) = 0;
	virtual RwObject *CreateInstance(RwMatrix *) = 0;
	virtual RwObject *GetRwObject(void) = 0;
	virtual void SetAnimFile(const char *file) {}
	virtual void ConvertAnimFileIndex(void) {}
	virtual int GetAnimFileIndex(void) { return -1; }

	// one day it becomes virtual
	uint8 GetModelType() const { return m_type; }
	bool IsBuilding(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME; }
	bool IsSimple(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME || m_type == MITYPE_WEAPON; }
	bool IsClump(void) { return m_type == MITYPE_CLUMP || m_type == MITYPE_PED || m_type == MITYPE_VEHICLE;	}
	char *GetModelName(void) { return m_name; }
	void SetModelName(const char *name) { strncpy(m_name, name, MAX_MODEL_NAME); }
	void SetColModel(CColModel *col, bool owns = false){
		m_colModel = col; m_bOwnsColModel = owns; }
	CColModel *GetColModel(void) { return m_colModel; }
	bool DoesOwnColModel(void) { return m_bOwnsColModel; }
	void DeleteCollisionModel(void);
	void ClearTexDictionary(void) { m_txdSlot = -1; }
	int16 GetObjectID(void) { return m_objectId; }
	void SetObjectID(int16 id) { m_objectId = id; }
	int16 GetTxdSlot(void) { return m_txdSlot; }
	void AddRef(void);
	void RemoveRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void RemoveTexDictionaryRef(void);
	uint16 GetNumRefs() const { return m_refCount; }
};
#endif
//- rouz edit (Claude)
